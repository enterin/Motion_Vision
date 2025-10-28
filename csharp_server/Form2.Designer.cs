namespace MFCServer1
{
    partial class Form2
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.lblTcpStatus = new System.Windows.Forms.Label();
            this.lblPythonStatus = new System.Windows.Forms.Label();
            this.lblPythonError = new System.Windows.Forms.Label();
            this.lblLastClient = new System.Windows.Forms.Label();
            this.lblLastResult = new System.Windows.Forms.Label();
            this.picTop = new System.Windows.Forms.PictureBox();
            this.picSide = new System.Windows.Forms.PictureBox();
            this.lblTopImage = new System.Windows.Forms.Label();
            this.lblSideImage = new System.Windows.Forms.Label();

            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picTop)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picSide)).BeginInit();
            this.SuspendLayout();
            // 
            // lblTcpStatus
            // 
            this.lblTcpStatus.AutoSize = true;
            this.lblTcpStatus.Location = new System.Drawing.Point(20, 20);
            this.lblTcpStatus.Name = "lblTcpStatus";
            this.lblTcpStatus.Size = new System.Drawing.Size(95, 15);
            this.lblTcpStatus.TabIndex = 1;
            this.lblTcpStatus.Text = "TCP: Unknown";
            // 
            // lblPythonStatus
            // 
            this.lblPythonStatus.AutoSize = true;
            this.lblPythonStatus.Location = new System.Drawing.Point(20, 45);
            this.lblPythonStatus.Name = "lblPythonStatus";
            this.lblPythonStatus.Size = new System.Drawing.Size(113, 15);
            this.lblPythonStatus.TabIndex = 2;
            this.lblPythonStatus.Text = "Python: Unknown";
            // 
            // lblPythonError
            // 
            this.lblPythonError.AutoSize = true;
            this.lblPythonError.Location = new System.Drawing.Point(20, 70);
            this.lblPythonError.Name = "lblPythonError";
            this.lblPythonError.Size = new System.Drawing.Size(91, 15);
            this.lblPythonError.TabIndex = 3;
            this.lblPythonError.Text = "LastError: None";
            // 
            // lblLastClient
            // 
            this.lblLastClient.AutoSize = true;
            this.lblLastClient.Location = new System.Drawing.Point(20, 95);
            this.lblLastClient.Name = "lblLastClient";
            this.lblLastClient.Size = new System.Drawing.Size(106, 15);
            this.lblLastClient.TabIndex = 4;
            this.lblLastClient.Text = "LastClient: None";
            // 
            // lblLastResult
            // 
            this.lblLastResult.AutoSize = true;
            this.lblLastResult.Location = new System.Drawing.Point(20, 120);
            this.lblLastResult.Name = "lblLastResult";
            this.lblLastResult.Size = new System.Drawing.Size(106, 15);
            this.lblLastResult.TabIndex = 5;
            this.lblLastResult.Text = "LastResult: None";
            // 
            // picTop
            // 
            this.picTop.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.picTop.Location = new System.Drawing.Point(680, 40);
            this.picTop.Name = "picTop";
            this.picTop.Size = new System.Drawing.Size(200, 200);
            this.picTop.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.picTop.TabIndex = 6;
            this.picTop.TabStop = false;
            // 
            // picSide
            // 
            this.picSide.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.picSide.Location = new System.Drawing.Point(900, 40);
            this.picSide.Name = "picSide";
            this.picSide.Size = new System.Drawing.Size(200, 200);
            this.picSide.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.picSide.TabIndex = 7;
            this.picSide.TabStop = false;
            // 
            // lblTopImage
            // 
            this.lblTopImage.AutoSize = true;
            this.lblTopImage.Location = new System.Drawing.Point(680, 20);
            this.lblTopImage.Name = "lblTopImage";
            this.lblTopImage.Size = new System.Drawing.Size(73, 15);
            this.lblTopImage.TabIndex = 8;
            this.lblTopImage.Text = "TOP IMAGE";
            // 
            // lblSideImage
            // 
            this.lblSideImage.AutoSize = true;
            this.lblSideImage.Location = new System.Drawing.Point(900, 20);
            this.lblSideImage.Name = "lblSideImage";
            this.lblSideImage.Size = new System.Drawing.Size(79, 15);
            this.lblSideImage.TabIndex = 9;
            this.lblSideImage.Text = "SIDE IMAGE";
            // 
            // dataGridView1
            // 
            this.dataGridView1.Location = new System.Drawing.Point(12, 250);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.Size = new System.Drawing.Size(1088, 250);
            this.dataGridView1.TabIndex = 0;
            this.dataGridView1.ColumnHeadersHeightSizeMode =
                System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowHeadersVisible = false;
            this.dataGridView1.SelectionMode =
                System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridView1.MultiSelect = false;
            this.dataGridView1.CellClick +=
                new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridView1_CellClick);
            // 
            // Form2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1112, 512);
            this.Controls.Add(this.lblSideImage);
            this.Controls.Add(this.lblTopImage);
            this.Controls.Add(this.picSide);
            this.Controls.Add(this.picTop);
            this.Controls.Add(this.lblLastResult);
            this.Controls.Add(this.lblLastClient);
            this.Controls.Add(this.lblPythonError);
            this.Controls.Add(this.lblPythonStatus);
            this.Controls.Add(this.lblTcpStatus);
            this.Controls.Add(this.dataGridView1);
            this.Name = "Form2";
            this.Text = "MFC Server Monitor";

            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picTop)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picSide)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        #endregion

        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.Label lblTcpStatus;
        private System.Windows.Forms.Label lblPythonStatus;
        private System.Windows.Forms.Label lblPythonError;
        private System.Windows.Forms.Label lblLastClient;
        private System.Windows.Forms.Label lblLastResult;

        private System.Windows.Forms.PictureBox picTop;
        private System.Windows.Forms.PictureBox picSide;
        private System.Windows.Forms.Label lblTopImage;
        private System.Windows.Forms.Label lblSideImage;
    }
}
