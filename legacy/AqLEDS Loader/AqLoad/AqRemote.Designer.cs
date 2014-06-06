namespace AqLoad
{
    partial class AqRemote
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btPower = new System.Windows.Forms.Button();
            this.btChannel = new System.Windows.Forms.Button();
            this.btSave = new System.Windows.Forms.Button();
            this.btMenu = new System.Windows.Forms.Button();
            this.btReturn = new System.Windows.Forms.Button();
            this.btDnight = new System.Windows.Forms.Button();
            this.tbLevel = new System.Windows.Forms.TrackBar();
            this.btReset = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.tbLevel)).BeginInit();
            this.SuspendLayout();
            // 
            // btPower
            // 
            this.btPower.Location = new System.Drawing.Point(46, 322);
            this.btPower.Name = "btPower";
            this.btPower.Size = new System.Drawing.Size(49, 48);
            this.btPower.TabIndex = 0;
            this.btPower.Text = "&Power";
            this.btPower.UseVisualStyleBackColor = true;
            this.btPower.Click += new System.EventHandler(this.btPower_Click);
            // 
            // btChannel
            // 
            this.btChannel.Location = new System.Drawing.Point(195, 150);
            this.btChannel.Name = "btChannel";
            this.btChannel.Size = new System.Drawing.Size(71, 48);
            this.btChannel.TabIndex = 1;
            this.btChannel.Text = "&Channel";
            this.btChannel.UseVisualStyleBackColor = true;
            this.btChannel.Click += new System.EventHandler(this.btChannel_Click);
            // 
            // btSave
            // 
            this.btSave.Location = new System.Drawing.Point(324, 150);
            this.btSave.Name = "btSave";
            this.btSave.Size = new System.Drawing.Size(61, 48);
            this.btSave.TabIndex = 2;
            this.btSave.Text = "&Save";
            this.btSave.UseVisualStyleBackColor = true;
            this.btSave.Click += new System.EventHandler(this.btSave_Click);
            // 
            // btMenu
            // 
            this.btMenu.Location = new System.Drawing.Point(195, 220);
            this.btMenu.Name = "btMenu";
            this.btMenu.Size = new System.Drawing.Size(71, 48);
            this.btMenu.TabIndex = 3;
            this.btMenu.Text = "&Menu";
            this.btMenu.UseVisualStyleBackColor = true;
            this.btMenu.Click += new System.EventHandler(this.btMenu_Click);
            // 
            // btReturn
            // 
            this.btReturn.Location = new System.Drawing.Point(324, 220);
            this.btReturn.Name = "btReturn";
            this.btReturn.Size = new System.Drawing.Size(61, 48);
            this.btReturn.TabIndex = 4;
            this.btReturn.Text = "E&xit";
            this.btReturn.UseVisualStyleBackColor = true;
            this.btReturn.Click += new System.EventHandler(this.btReturn_Click);
            // 
            // btDnight
            // 
            this.btDnight.Location = new System.Drawing.Point(195, 293);
            this.btDnight.Name = "btDnight";
            this.btDnight.Size = new System.Drawing.Size(71, 48);
            this.btDnight.TabIndex = 5;
            this.btDnight.Text = "&Day/Night";
            this.btDnight.UseVisualStyleBackColor = true;
            this.btDnight.Click += new System.EventHandler(this.btDnight_Click);
            // 
            // tbLevel
            // 
            this.tbLevel.Location = new System.Drawing.Point(59, 56);
            this.tbLevel.Maximum = 127;
            this.tbLevel.Name = "tbLevel";
            this.tbLevel.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.tbLevel.Size = new System.Drawing.Size(45, 250);
            this.tbLevel.TabIndex = 6;
            this.tbLevel.Scroll += new System.EventHandler(this.tbLevel_Scroll);
            // 
            // btReset
            // 
            this.btReset.Location = new System.Drawing.Point(390, 373);
            this.btReset.Name = "btReset";
            this.btReset.Size = new System.Drawing.Size(48, 30);
            this.btReset.TabIndex = 7;
            this.btReset.Text = "&Reset";
            this.btReset.UseVisualStyleBackColor = true;
            this.btReset.Click += new System.EventHandler(this.button1_Click);
            // 
            // AqRemote
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(450, 406);
            this.Controls.Add(this.btReset);
            this.Controls.Add(this.tbLevel);
            this.Controls.Add(this.btDnight);
            this.Controls.Add(this.btReturn);
            this.Controls.Add(this.btMenu);
            this.Controls.Add(this.btSave);
            this.Controls.Add(this.btChannel);
            this.Controls.Add(this.btPower);
            this.Name = "AqRemote";
            this.Text = "AqRemote";
            ((System.ComponentModel.ISupportInitialize)(this.tbLevel)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btPower;
        private System.Windows.Forms.Button btChannel;
        private System.Windows.Forms.Button btSave;
        private System.Windows.Forms.Button btMenu;
        private System.Windows.Forms.Button btReturn;
        private System.Windows.Forms.Button btDnight;
        private System.Windows.Forms.TrackBar tbLevel;
        private System.Windows.Forms.Button btReset;
    }
}