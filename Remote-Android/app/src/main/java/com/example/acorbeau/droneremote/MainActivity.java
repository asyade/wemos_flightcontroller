package com.example.acorbeau.droneremote;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.DragAndDropPermissions;
import android.view.DragEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.reflect.Array;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Arrays;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

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

    private Rect rectangle1;
    private Rect rectangle2;
    ImageView mImageView;
    public int[] Angle;

    DatagramSocket udpSocket;
    InetAddress serverAddr = InetAddress.getByName("192.168.1.15");
    short serverPort = 4242;
    byte[] buffer = new byte[128];

    public MainActivity() throws UnknownHostException {
    }

    private void InitServe()
    {
        try {
            udpSocket = new DatagramSocket(serverPort);
            Log.d("UDP","Udp loaded !");
        }
        catch (Exception e)
        {
            Log.e("UDP","Can't reash server !");
            Log.e("UDP", e.toString());
        }
    }

    private  void SendBuffer(int  n)
    {
        DatagramPacket packet = new DatagramPacket(buffer, n,serverAddr, serverPort);
        try {
            udpSocket.send(packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void SendChanToClient(final byte chan, final int value)
    {
        new Thread(new Runnable() {
            @Override
            public void run() {
                buffer[0] = chan;

                buffer[4] = (byte)((value & 0xFF000000) >> 24);
                buffer[3] = (byte)((value & 0x00FF0000) >> 16);
                buffer[2] = (byte)((value & 0x0000FF00) >> 8);
                buffer[1] = (byte)(value & 0x000000FF);
                SendBuffer(54);
                Log.d("UDP", String.format("Sending vel of %d", value));
            }
        }).start();
    }



    private  void RefreshRect()
    {
        Display display;
        display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int width = size.x;
        int height = size.y;
        Bitmap bitmap = Bitmap.createBitmap(
                width, // Width
                height, // Height
                Bitmap.Config.ARGB_8888 // Config
        );

        Canvas canvas = new Canvas(bitmap);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);

        int fh = (height / 2) + (Angle[0] / 5);

        rectangle1 = new Rect(0, 0, width, fh);
        rectangle2 = new Rect(0, fh, width, height -fh);

        canvas.drawColor(Color.BLUE);
        canvas.drawRect(rectangle1, paint);
        paint.setColor(Color.RED);
        canvas.drawRect(rectangle2, paint);
        mImageView.setImageBitmap(bitmap);
        mImageView.setRotation(-(Angle[1] / 6));
    }

    JoystickView js;

    SeekBar mode;
    SeekBar vel;

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        vel = findViewById(R.id.velbar);
        mode = findViewById(R.id.modeBar);
        js = findViewById(R.id.joystickView);


        vel.setMin(0);
        vel.setMax(2500);
        vel.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SendChanToClient(THR, vel.getProgress());
            }
        });

        mode.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        js.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {

            }}, 17); // around 60/sec

        mImageView = (ImageView) findViewById(R.id.iv);
        Angle = new int[]{0, 0};
        InitServe();
        final Activity self = this;
        Thread thread = new Thread(new Runnable() {
            public void run() {
                while (true)
                {
                    Log.d("Gyro", "Req update");
                        buffer[0] = 12;
                        SendBuffer(1);
                        byte[] buffer = new byte[128];
                        DatagramPacket p = new DatagramPacket(buffer, 128);
                        try {
                            udpSocket.setSoTimeout(3000);
                            udpSocket.receive(p);
                            DataInputStream bf = new DataInputStream(new ByteArrayInputStream(buffer));
                            int id = bf.readInt();

                            Angle[0] = bf.readInt();
                            Angle[0] = Angle[0] >> 24 | (Angle[0] & 0x00ff0000) >> 8 | (Angle[0] & 0x0000ff00 ) << 8 | (Angle[0] & 0xff000000) << 16;
                            Angle[1] = bf.readInt();
                            Angle[1] = Angle[1] >> 24 | (Angle[1] & 0x00ff0000) >> 8 | (Angle[1] & 0x0000ff00 ) << 8 | (Angle[1] & 0xff000000) << 16;
                            self.runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    RefreshRect();
                                }
                            });
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    try {
                        Thread.sleep(50);
                    } catch (Exception e){}
                }
            }

        });

       // thread.start();
        RefreshRect();
    }

}

