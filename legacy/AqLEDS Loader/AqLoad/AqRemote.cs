using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace AqLoad
{
    public partial class AqRemote : Form
    {
        private SerialPort  sp;
        public AqRemote()
        {
            InitializeComponent();
        }

        public void SetSerialPort(SerialPort sp)
        {
            this.sp = sp;
        }
        private void btPower_Click(object sender, EventArgs e)
        {
            sp.Write("p");
        }

        private void btChannel_Click(object sender, EventArgs e)
        {
            sp.Write("c");
        }

        private void btSave_Click(object sender, EventArgs e)
        {
            sp.Write("s");
        }

        private void btMenu_Click(object sender, EventArgs e)
        {
            sp.Write("m");
        }

        private void btReturn_Click(object sender, EventArgs e)
        {
            sp.Write("x");
        }

        private void btDnight_Click(object sender, EventArgs e)
        {
            sp.Write("d");
        }

        private void tbLevel_Scroll(object sender, EventArgs e)
        {
            byte[] buffer = new byte[2];
            buffer[0] = (byte)(tbLevel.Value + 0x80);
            sp.Write(buffer, 0, 1);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            byte[] buffer = new byte[2];
            buffer[0] = (byte)(9);
            sp.Write(buffer, 0, 1);
        }

    }
}
