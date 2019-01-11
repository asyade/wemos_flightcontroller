using OxyPlot;
using OxyPlot.Series;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DroneRemote
{
    public partial class Form2 : Form
    {
        LineSeries serie;

        public Form2()
        {
            InitializeComponent();
            var pm = new PlotModel
            {
                PlotType = PlotType.Cartesian,
                Background = OxyColors.White
            };
            serie = new LineSeries();

            pm.Series.Add(serie);
            plot1.Model = pm;
        }

        internal void Refresh(float v1, float v2)
        {
            var pm = new PlotModel
            {
                PlotType = PlotType.Cartesian,
                Background = OxyColors.White
            };
            serie.Points.Add(new DataPoint(v1, v2));
            pm.Series.Add(serie);
            plot1.Model = pm;
        }
    }
}
