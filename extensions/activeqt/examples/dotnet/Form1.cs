using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace csharp
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private AxmultipleaxLib.AxQAxWidget2 axQAxWidget21;
		private AxwrapperaxLib.AxQPushButton axQPushButton1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
			this.axQPushButton1 = new AxwrapperaxLib.AxQPushButton();
			this.axQAxWidget21 = new AxmultipleaxLib.AxQAxWidget2();
			((System.ComponentModel.ISupportInitialize)(this.axQPushButton1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.axQAxWidget21)).BeginInit();
			this.SuspendLayout();
			// 
			// axQPushButton1
			// 
			this.axQPushButton1.Enabled = true;
			this.axQPushButton1.Location = new System.Drawing.Point(48, 232);
			this.axQPushButton1.Name = "axQPushButton1";
			this.axQPushButton1.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("axQPushButton1.OcxState")));
			this.axQPushButton1.Size = new System.Drawing.Size(192, 32);
			this.axQPushButton1.TabIndex = 1;
			this.axQPushButton1.clicked += new System.EventHandler(this.axQPushButton1_clicked);
			// 
			// axQAxWidget21
			// 
			this.axQAxWidget21.Enabled = true;
			this.axQAxWidget21.Location = new System.Drawing.Point(8, 8);
			this.axQAxWidget21.Name = "axQAxWidget21";
			this.axQAxWidget21.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("axQAxWidget21.OcxState")));
			this.axQAxWidget21.Size = new System.Drawing.Size(280, 216);
			this.axQAxWidget21.TabIndex = 2;
			this.axQAxWidget21.ClickEvent += new System.EventHandler(this.axQAxWidget21_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(292, 273);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.axQAxWidget21,
																		  this.axQPushButton1});
			this.Name = "Form1";
			this.Text = "Form1";
			((System.ComponentModel.ISupportInitialize)(this.axQPushButton1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.axQAxWidget21)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		private void axQAxWidget21_Click(object sender, System.EventArgs e)
		{
			this.axQAxWidget21.lineWidth++;
		}

		private void axQPushButton1_clicked(object sender, System.EventArgs e)
		{
			this.axQAxWidget21.lineWidth = 1;
		}
	}
}
