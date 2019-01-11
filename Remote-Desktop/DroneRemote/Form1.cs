using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Net.Sockets;
using System.Net;
using System.IO;

namespace DroneRemote
{
    public partial class Form1 : Form
    {
        #region Extern

        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        private static extern bool BitBlt(IntPtr hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, IntPtr hdcSrc, int nXSrc, int nYSrc, System.Int32 dwRop);
        [System.Runtime.InteropServices.DllImportAttribute("user32.dll")]
        public static extern IntPtr GetDC(IntPtr hwnd);
        [System.Runtime.InteropServices.DllImportAttribute("user32.dll")]
        public static extern IntPtr ReleaseDC(IntPtr hwnd, IntPtr hdc);

        #endregion

        #region Fields

        Math3D.Cube mainCube;
        Point drawOrigin;

        UdpClient client;
        String remoteIp = "192.168.1.15";
        private byte[] buffer = new byte[128];

        Keys KeyUp = Keys.W;
        Keys KeyDownS = Keys.S;
        Keys KeyLeft = Keys.A;
        Keys KeyRight = Keys.D;
        Keys KeyLess = Keys.Control;
        Keys KeyMore = Keys.Shift;
        Keys KeyInit = Keys.I;
        Keys KeyStop = Keys.E;

        static byte GYRO = 32;
        static byte ROL = 0;
        static byte PIT = 1;
        static byte THR = 2;
        static byte RUD = 3;
        static byte AU1 = 4;
        static byte AU2 = 5;
        static byte ACC_READ = 6;
        static byte GET_PID = 7;
        static byte SET_PID = 8;

        float[] rotate = { 0, 0, 0 };
        int [] vel = { 0, 0, 0, 0 };

        bool refreshgGyro = true;

        #endregion

        #region Init

        public Form1()
        {
            InitializeComponent();
        }

        private async void Form1_Load(object sender, EventArgs e)
        {
            IPAddress addr = IPAddress.Parse(GetLocalIPAddress());
            int port = 4242;
            client = new UdpClient();
            client.Connect(remoteIp, 4242);
            client.BeginReceive(Recv, null);
            mainCube = new Math3D.Cube(100, 200, 75);
            drawOrigin = new Point(pictureBox1.Width / 2, pictureBox1.Height / 2);
            LoopGyro();
        }

        #endregion

        #region Network

        public static string GetLocalIPAddress()
        {
            var host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (var ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    return ip.ToString();
                }
            }
            throw new Exception("No network adapters with an IPv4 address in the system!");
        }

        private void SendToClient(byte[] buff)
        {
            client.Send(buff, buff.Length);
        }

        private void SendChanToClient(byte chan, int value)
        {
            MemoryStream str = new MemoryStream();
            BinaryWriter wr = new BinaryWriter(str);
            wr.Write(chan);
            wr.Write(value);
            client.Send(str.GetBuffer(), (int)str.Length);
        }


        private void Log(string text, params object[] param)
        {
            this.BeginInvoke(new Action(() =>
            {
                richTextBox1.AppendText(string.Format(text, param));
                richTextBox1.AppendText("\n");
                richTextBox1.ScrollToCaret();
            }));
        }

        private void Recv(IAsyncResult res)
        {
            byte[] rdbuff = null;
            IPEndPoint ipp = (IPEndPoint)client.Client.RemoteEndPoint;
            try
            {
                rdbuff = client.EndReceive(res, ref ipp);
            }
            catch
            {
                Log("Connection lost !");
                return;
            }
            try
            {
                BinaryReader rd = new BinaryReader(new MemoryStream(rdbuff));
                int r = rd.ReadInt32();

                if (r == GYRO && buffer.Length > sizeof(int) * 4)
                {
                    rotate[0] = rd.ReadInt32();
                    rotate[1] = rd.ReadInt32();
                    vel[0] = rd.ReadInt16();
                    vel[1] = rd.ReadInt16();
                    vel[2] = rd.ReadInt16();
                    vel[3] = rd.ReadInt16();
                    for (int i = 0; i < 4; i++)
                    {
                        vel[i] = vel[i]/ 20;
                        vel[i] = Math.Min(vel[i], 100);
                    }
                    Log("Receive gyro state {0} {1} {2}", rotate[0], rotate[1], 0);
                    this.BeginInvoke(new Action(() =>
                    {
                   // if (vel[0] > 70 && vel[1] > 70 && vel[2] > 70 && vel[3] > 70 )
                        {
                            pbA.Value = vel[0];
                            pbB.Value = vel[1];
                            pbC.Value = vel[2];
                            pbD.Value = vel[3];

                            if (frm != null)
                            {
                                frm.Refresh(rotate[0], rotate[1]);
                            }

                        }
                  /*      else
                        {
                            pbA.Value = Math.Min((vel[0] * 2) - 100, 100);
                            pbB.Value = Math.Min((vel[1] * 2) - 100, 100);
                            pbC.Value = Math.Min((vel[2] * 2) - 100, 100);
                            pbD.Value = Math.Min((vel[3] * 2) - 100, 100);
                        }*/
                        this.Refresh();
                    }));
                }
                else if (r == GET_PID)
                {
                    Log("PID received !");
                    UpdatePid(new int[] {
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                        rd.ReadByte(),
                    });
                }
                else if (r == SET_PID)
                {
                    Log("PID Set !");
                }
                else
                {
                    Log("> {0}", UTF8Encoding.Default.GetString(rdbuff));
                }
            }
            catch
            {

            }
            buffer = new byte[128];
            client.BeginReceive(Recv, null);
        }

        #endregion

        #region Routine

        private async void LoopGyro()
        {
            while (true)
            {
                await Task.Delay((int)numericUpDown1.Value);

                if (!refreshgGyro)
                    continue;
                byte[] br = new byte[1];
                br[0] = 12;
                client.Send(br, 1);
            }
        }

        private void UpdatePid(int[] ids)
        {
            this.BeginInvoke(new Action(() =>
            {
                gp.Value = ids[0];
                sp.Value = ids[1];
                rp.Value = ids[2];
                gi.Value = ids[3];
                si.Value = ids[4];
                ri.Value = ids[5];
                gd.Value = ids[6];
                sd.Value = ids[7];
                rd.Value = ids[8];
            }));
        }

        #endregion

        #region Compass

        private void Render()
        {

            Bitmap img = new Bitmap(128, 128);
            Graphics g = Graphics.FromImage(img);
            g.DrawEllipse(new Pen(Brushes.Black, 4), 4, 4, 120, 120);

            g.DrawRectangle(new Pen(Brushes.Black, 4), 4, 60 + rotate[0] / 5  , 120, 8);
            //rotate
            pictureBox1.Image = RotateImage(img, -((float)rotate[1] / 6));

        }

        public static Bitmap RotateImage(Bitmap b, float angle)
        {
            //create a new empty bitmap to hold rotated image
            Bitmap returnBitmap = new Bitmap(b.Width, b.Height);
            //make a graphics object from the empty bitmap
            using (Graphics g = Graphics.FromImage(returnBitmap))
            {
                //move rotation point to center of image
                g.TranslateTransform((float)b.Width / 2, (float)b.Height / 2);
                //rotate
                g.RotateTransform(angle);
                //move image back
                g.TranslateTransform(-(float)b.Width / 2, -(float)b.Height / 2);
                //draw passed in image onto graphics object
                g.DrawImage(b, new Point(0, 0));
            }
            return returnBitmap;
        }

        #endregion

        #region UI Handlers

        private void trackBar1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == KeyInit)
            {
                Log("Sending initialization sequence ...");
                SendChanToClient(AU2, trackBar1.Value);
            }

        }

        private void trackBar1_MouseUp(object sender, MouseEventArgs e)
        {
            if (client != null)
            {
                Log("Sending velocity of {0}", trackBar1.Value);
                SendChanToClient(AU1, trackBar1.Value);
            }
        }

        private void trackBar2_MouseUp(object sender, MouseEventArgs e)
        {
            if (client != null)
            {
                Log("Sending throt of {0}", trackBar2.Value);
                SendChanToClient(THR, trackBar2.Value);
            }
        }

 
        private void Form1_Paint(object sender, PaintEventArgs e)
        {

            Render();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            client = new UdpClient();
            client.Connect(remoteIp, 4242);
            client.BeginReceive(Recv, null);
        }



        private void button2_Click_2(object sender, EventArgs e)
        {
            MemoryStream ms = new MemoryStream();
            BinaryWriter wr = new BinaryWriter(ms);
            wr.Write(GET_PID);
            SendToClient(ms.GetBuffer().Take(sizeof(int)).ToArray());
        }

        private void button4_Click_1(object sender, EventArgs e)
        {
            MemoryStream ms = new MemoryStream();
            BinaryWriter wr = new BinaryWriter(ms);
            wr.Write(SET_PID);
            wr.Write((byte)gp.Value);
            wr.Write((byte)sp.Value);
            wr.Write((byte)rp.Value);
            wr.Write((byte)gi.Value);
            wr.Write((byte)si.Value);
            wr.Write((byte)ri.Value);
            wr.Write((byte)gd.Value);
            wr.Write((byte)sd.Value);
            wr.Write((byte)rd.Value);
            SendToClient(ms.GetBuffer().Take((int)ms.Length).ToArray());
        }

        private void button1_Click_2(object sender, EventArgs e)
        {
            SendChanToClient(ACC_READ, 1);
        }

        private void button3_Click_1(object sender, EventArgs e)
        {
            refreshgGyro = !refreshgGyro;
        }

        #endregion

        #region Movement buttons

        private void button6_MouseDown(object sender, MouseEventArgs e)
        {
            SendChanToClient(PIT, 3000);
        }

        private void button6_MouseUp(object sender, MouseEventArgs e)
        {
            SendChanToClient(PIT, 0);
        }

        private void button8_MouseDown(object sender, MouseEventArgs e)
        {
            SendChanToClient(PIT, -3000);
        }

        private void button8_MouseUp(object sender, MouseEventArgs e)
        {
            SendChanToClient(PIT, 0);
        }

        private void button9_MouseDown(object sender, MouseEventArgs e)
        {
            SendChanToClient(ROL, 3000);
        }

        private void button9_MouseUp(object sender, MouseEventArgs e)
        {
            SendChanToClient(ROL, 0);
        }

        private void button7_MouseDown(object sender, MouseEventArgs e)
        {
            SendChanToClient(ROL, -3000);
        }

        private void button7_MouseUp(object sender, MouseEventArgs e)
        {
            SendChanToClient(ROL, 0);
        }

        #endregion

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            {
                if (e.KeyCode == KeyLess)
                {
                    
                }
                else if (e.KeyCode == KeyMore)
                {

                }
            }
        }

        Form2 frm = null;

        private void button5_Click(object sender, EventArgs e)
        {
            frm = new Form2();
            frm.Show();
        }
    }
}
