using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Ini;

namespace AqLoad
{
    public partial class FrmSetup : Form
    {
        public FrmSetup()
        {
            InitializeComponent();
        }

        string[] SerialPortNames1;
        string[] SerialPortNames2;
        List<String> SerialPorts;
       

        int state;

        private void FrmSetup_Shown(object sender, EventArgs e)
        {
            state = 0;
            SerialPorts = new List<String>();
        }

        private void btNext_Click(object sender, EventArgs e)
        {
            if (state == 0)
            {
                SerialPortNames1 = System.IO.Ports.SerialPort.GetPortNames();
                state = 1;
                label1.BorderStyle = BorderStyle.None;
                label2.BorderStyle = BorderStyle.FixedSingle;
                label2.Enabled = true;
            }
            else if (state == 1)
            {
                SerialPortNames2 = System.IO.Ports.SerialPort.GetPortNames();

                foreach (string sp in SerialPortNames2)
                {
                    bool contained = false;

                    foreach (string sp2 in SerialPortNames1)
                    {
                        if (sp2 == sp) contained = true;
                    }
                    if (!contained)
                    {
                        SerialPorts.Add(sp);
                    }

                }

                if (SerialPorts.Count == 1)
                {
                    state = 2;
                    label1.Visible = true;
                    label1.Text = "AqLEDS detected at port " + SerialPorts[0];
                    label2.Visible = false;
                    btNext.Text = "&Done";
                }
                else
                {

                    MessageBox.Show("There was an error in the process, please disconnect your AqLEDS");
                    state = 0;
                    label1.Enabled = true;
                    label2.Enabled = false;
                    label1.BorderStyle = BorderStyle.FixedSingle;
                    label2.BorderStyle = BorderStyle.None;
                }
            }
            else if (state == 2)
            {
                IniFile ini = new IniFile(".\\aqleds.ini");
                ini.IniWriteValue("port", "id", SerialPorts[0]);
                this.Close();

            }

        }

        private void btCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
