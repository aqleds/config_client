using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using Ini;

namespace AqLoad
{
    public partial class Form1 : Form
    {

        const byte COMMAND_NACK=0x00;
        const byte COMMAND_ACK = 0x01;
        const byte COMMAND_WRITE_PM =0x03;
        const byte COMMAND_WRITE_CM =0x07;
        const byte COMMAND_RESET    =0x08;
        const byte COMMAND_READ_ID  =0x09;


        const int FLASH_BLOCK_SIZE = 0x600;
        byte[] file_data;
        int file_size;
        int p = 0;

        string serialPortName;
        const string sProductName = "AqLEDS";
        const string sFileExtension = "aqf";

        private bool LoadFile(string filename)
        {

            try
            {
                file_data = File.ReadAllBytes(filename);
                file_size = file_data.Length;
                return true;
            }
            catch
            {
                return false;
            }

        }

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void LoadConfig()
        {
            IniFile ini = new IniFile(".\\"+sProductName+".ini");
            serialPortName = ini.IniReadValue("port", "id", "");
        }

        private void btOpenFile_Click(object sender, EventArgs e)
        {

            ShowDialogOpenFile();

        }

        private void ShowDialogOpenFile()
        {
            string filename;
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Firmware Files (*."+sFileExtension+")|*."+sFileExtension+"|All files(*.*)|*.*";
            ofd.FileName = "*."+sFileExtension;
            ofd.Title = "Open "+sProductName+" Firmware File";
            DialogResult res = ofd.ShowDialog();

            if (res == DialogResult.OK || res == DialogResult.Yes)
            {

                filename = ofd.FileName;
                if (File.Exists(filename))
                {
                    LoadFile(filename);
                }
            }
        }

        private void btProgram_Click(object sender, EventArgs e)
        {
            if (file_data == null)
            {
                MessageBox.Show("No firmware file loaded");
                return;
            }
            LoadConfig();
            if (serialPortName == "" || serialPortName == null)
            {
                MessageBox.Show(sProductName+" is not configured, we will run the\nport setup now");

                FrmSetup f = new FrmSetup();

                f.ShowDialog();
                LoadConfig();
            }
            else
            {
                try
                {
                    OpenSerialPort();
                }
                catch (System.IO.IOException ex)
                {
                    MessageBox.Show("Error abriendo " + serialPortName + "\r\n" + ex);
                    return;
                }

                btProgram.Enabled = false;
                btOpenFile.Enabled = false;



                this.backgroundFlasher.RunWorkerAsync();

            }
        }

        private void OpenSerialPort()
        {
            LoadConfig();
            if (serialPort1.IsOpen) serialPort1.Close();
            serialPort1.PortName = serialPortName;
            serialPort1.BaudRate = 115200;
            serialPort1.Handshake = System.IO.Ports.Handshake.None;
            serialPort1.Open();
        }

        private void backroundFlasher_DoWork(object sender, DoWorkEventArgs e)
        {
            p=0;
            byte[] buffer = new byte[512];
            int timeouts = 100;

            this.backgroundFlasher.ReportProgress(0, "Waiting for device");
            while (true)
            {
                
                buffer[0]=COMMAND_READ_ID;
                serialPort1.Write(buffer,0,1);
                System.Threading.Thread.Sleep(500);
                serialPort1.ReadTimeout=500;
                try
                {

                    serialPort1.Read(buffer, 0, 2);
                    if (buffer[0] == 0x03 && buffer[1] == 0xC0)
                    {

                        break;
                    }
                    else
                    {
                        while (true)
                        {
                            try
                            {
                                serialPort1.Read(buffer, 0, 512);
                                Console.WriteLine("cleanning buffer...(1)");
                            }
                            catch (System.TimeoutException)
                            {
                                break;
                            }
                        }

                    }
                }
                catch (System.TimeoutException)
                {
                    this.backgroundFlasher.ReportProgress(timeouts, "Waiting for device");
                    timeouts--;
                    if (timeouts <= 0) return;
                }

            }

            while (true)
            {
                try
                {
                    serialPort1.ReadTimeout = 200;
                    serialPort1.Read(buffer, 0, 512);
                    Console.WriteLine("cleanning buffer...(2)");
                }
                catch (System.TimeoutException)
                {
                    break;
                }
            }



            serialPort1.ReadTimeout = 1000;
            this.backgroundFlasher.ReportProgress(0, "Flashing");
            while (p < file_size)
            {
                byte cmd = file_data[p];
                int data=0;
                

                if ((p + 3) < file_size)
                {
                    data = (file_data[p + 1] << 16) +
                                   (file_data[p + 2] << 8) +
                                   (file_data[p + 3]);
                }

                if (cmd == COMMAND_WRITE_PM)
                {

                    int addr = data;

                    Console.WriteLine("Data at {0}", addr);
                    serialPort1.Write(file_data, p, 4 + FLASH_BLOCK_SIZE);
                    p += 4; p += FLASH_BLOCK_SIZE;
                    byte res=(byte)serialPort1.ReadByte();
                    if (res != COMMAND_ACK)
                    {
                        MessageBox.Show("Protocol Error");
                        return;
                    }
                }
                else if (cmd == COMMAND_WRITE_CM)
                {
                    Console.WriteLine("Write CM {0}", data);
                    serialPort1.Write(file_data, p, 8*3 + 1);
                    p += 1; p += 8 * 3;
                    byte res = (byte)serialPort1.ReadByte();
                    if (res != COMMAND_ACK)
                    {
                        MessageBox.Show("Protocol Error");
                        this.backgroundFlasher.ReportProgress(0, "Error!!");
                        return;
                    }
                }
                else if (cmd == COMMAND_READ_ID)
                {
                    Console.WriteLine("Read ID??");
                    p += 1;
                }
                else if (cmd == COMMAND_RESET)
                {
                    Console.WriteLine("Reset!!");
                    serialPort1.Write(file_data, p, 1);
                    p += 1;
                   
                }
                else
                {
                    Console.WriteLine("Unknown sequence...");
                    MessageBox.Show("Unknown command sequence in firmware");
                    this.backgroundFlasher.ReportProgress(0, "Error!!");
                    return;
                }

                this.backgroundFlasher.ReportProgress((p * 100) / file_size);
                System.Threading.Thread.Sleep(500);
            }

            this.backgroundFlasher.ReportProgress(0, "Done!!");


        }

        private void backroundFlasher_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            pBar.Value = e.ProgressPercentage;

            if (e.UserState != null)
            {
                lbState.Text = (string)e.UserState;
            }
        }

        private void backroundFlasher_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            btProgram.Enabled = true;
            btOpenFile.Enabled = true;
            serialPort1.Close();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void setupToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FrmSetup f = new FrmSetup();

            f.ShowDialog();
            LoadConfig();
        }

        private void remoteControlToolStripMenuItem_Click(object sender, EventArgs e)
        {

            AqRemote f = new AqRemote();
            OpenSerialPort();
            f.SetSerialPort(serialPort1);
            f.ShowDialog();


        }
    }
}
