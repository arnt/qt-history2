using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace QMsNet
{
	/// <summary>
	/// Summary description for NewProject2.
	/// </summary>
	public class NewQtProject2 : System.Windows.Forms.Form
	{
	    public System.Windows.Forms.CheckBox chkAddToSolution;
	    private System.Windows.Forms.Button btnDesigner;
	    private System.Windows.Forms.Button btnCancel;
	    private System.Windows.Forms.Button btnOK;
	    public System.Windows.Forms.TextBox txtProjectName;
	    private System.Windows.Forms.Label label2;
	    private System.Windows.Forms.GroupBox groupBox1;
	    public System.Windows.Forms.RadioButton optApplication;
	    private System.Windows.Forms.Button btnBrowse;
	    public System.Windows.Forms.SaveFileDialog saveProDialog;
	    public System.Windows.Forms.RadioButton optLibrary;
	    private System.Windows.Forms.Label label1;
	    public System.Windows.Forms.Label lblComment;
	    public System.Windows.Forms.TextBox txtProjectFile;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public NewQtProject2()
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
				if(components != null)
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
		    this.chkAddToSolution = new System.Windows.Forms.CheckBox();
		    this.btnDesigner = new System.Windows.Forms.Button();
		    this.btnCancel = new System.Windows.Forms.Button();
		    this.btnOK = new System.Windows.Forms.Button();
		    this.txtProjectName = new System.Windows.Forms.TextBox();
		    this.label2 = new System.Windows.Forms.Label();
		    this.groupBox1 = new System.Windows.Forms.GroupBox();
		    this.optApplication = new System.Windows.Forms.RadioButton();
		    this.optLibrary = new System.Windows.Forms.RadioButton();
		    this.btnBrowse = new System.Windows.Forms.Button();
		    this.saveProDialog = new System.Windows.Forms.SaveFileDialog();
		    this.txtProjectFile = new System.Windows.Forms.TextBox();
		    this.label1 = new System.Windows.Forms.Label();
		    this.lblComment = new System.Windows.Forms.Label();
		    this.groupBox1.SuspendLayout();
		    this.SuspendLayout();
		    // 
		    // chkAddToSolution
		    // 
		    this.chkAddToSolution.Checked = true;
		    this.chkAddToSolution.CheckState = System.Windows.Forms.CheckState.Checked;
		    this.chkAddToSolution.Location = new System.Drawing.Point(192, 136);
		    this.chkAddToSolution.Name = "chkAddToSolution";
		    this.chkAddToSolution.Size = new System.Drawing.Size(158, 16);
		    this.chkAddToSolution.TabIndex = 19;
		    this.chkAddToSolution.Text = "Add to current S&olution";
		    // 
		    // btnDesigner
		    // 
		    this.btnDesigner.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
		    this.btnDesigner.Location = new System.Drawing.Point(8, 152);
		    this.btnDesigner.Name = "btnDesigner";
		    this.btnDesigner.Size = new System.Drawing.Size(99, 23);
		    this.btnDesigner.TabIndex = 18;
		    this.btnDesigner.Text = "&Use Designer...";
		    // 
		    // btnCancel
		    // 
		    this.btnCancel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
		    this.btnCancel.Location = new System.Drawing.Point(280, 152);
		    this.btnCancel.Name = "btnCancel";
		    this.btnCancel.TabIndex = 16;
		    this.btnCancel.Text = "&Cancel";
		    // 
		    // btnOK
		    // 
		    this.btnOK.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
		    this.btnOK.Location = new System.Drawing.Point(192, 152);
		    this.btnOK.Name = "btnOK";
		    this.btnOK.TabIndex = 15;
		    this.btnOK.Text = "&OK";
		    // 
		    // txtProjectName
		    // 
		    this.txtProjectName.Location = new System.Drawing.Point(80, 4);
		    this.txtProjectName.Name = "txtProjectName";
		    this.txtProjectName.Size = new System.Drawing.Size(245, 20);
		    this.txtProjectName.TabIndex = 13;
		    this.txtProjectName.Text = "noname";
		    // 
		    // label2
		    // 
		    this.label2.Location = new System.Drawing.Point(8, 8);
		    this.label2.Name = "label2";
		    this.label2.Size = new System.Drawing.Size(76, 16);
		    this.label2.TabIndex = 12;
		    this.label2.Text = "Project &Name:";
		    // 
		    // groupBox1
		    // 
		    this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
											    this.optApplication,
											    this.optLibrary});
		    this.groupBox1.Location = new System.Drawing.Point(8, 64);
		    this.groupBox1.Name = "groupBox1";
		    this.groupBox1.RightToLeft = System.Windows.Forms.RightToLeft.No;
		    this.groupBox1.Size = new System.Drawing.Size(136, 72);
		    this.groupBox1.TabIndex = 11;
		    this.groupBox1.TabStop = false;
		    this.groupBox1.Text = "Project Type";
		    // 
		    // optApplication
		    // 
		    this.optApplication.Checked = true;
		    this.optApplication.Location = new System.Drawing.Point(8, 16);
		    this.optApplication.Name = "optApplication";
		    this.optApplication.Size = new System.Drawing.Size(120, 24);
		    this.optApplication.TabIndex = 0;
		    this.optApplication.TabStop = true;
		    this.optApplication.Text = "&Application (.exe)";
		    // 
		    // optLibrary
		    // 
		    this.optLibrary.Location = new System.Drawing.Point(8, 40);
		    this.optLibrary.Name = "optLibrary";
		    this.optLibrary.Size = new System.Drawing.Size(120, 24);
		    this.optLibrary.TabIndex = 0;
		    this.optLibrary.Text = "&Library (.dll/.lib)";
		    // 
		    // btnBrowse
		    // 
		    this.btnBrowse.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
		    this.btnBrowse.Location = new System.Drawing.Point(328, 32);
		    this.btnBrowse.Name = "btnBrowse";
		    this.btnBrowse.Size = new System.Drawing.Size(24, 21);
		    this.btnBrowse.TabIndex = 14;
		    this.btnBrowse.Text = "...";
		    // 
		    // saveProDialog
		    // 
		    this.saveProDialog.DefaultExt = "pro";
		    this.saveProDialog.FileName = "newProject.pro";
		    this.saveProDialog.Filter = "Qt Project files|*.pro|All files|*.*";
		    this.saveProDialog.Title = "New Qt Project...";
		    // 
		    // txtProjectFile
		    // 
		    this.txtProjectFile.Location = new System.Drawing.Point(80, 32);
		    this.txtProjectFile.Name = "txtProjectFile";
		    this.txtProjectFile.Size = new System.Drawing.Size(245, 20);
		    this.txtProjectFile.TabIndex = 21;
		    this.txtProjectFile.Text = "";
		    // 
		    // label1
		    // 
		    this.label1.Location = new System.Drawing.Point(8, 40);
		    this.label1.Name = "label1";
		    this.label1.Size = new System.Drawing.Size(76, 16);
		    this.label1.TabIndex = 20;
		    this.label1.Text = "Project &File:";
		    // 
		    // lblComment
		    // 
		    this.lblComment.Location = new System.Drawing.Point(168, 64);
		    this.lblComment.Name = "lblComment";
		    this.lblComment.Size = new System.Drawing.Size(184, 72);
		    this.lblComment.TabIndex = 22;
		    this.lblComment.Text = "<comment>";
		    // 
		    // NewQtProject2
		    // 
		    this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
		    this.ClientSize = new System.Drawing.Size(368, 182);
		    this.Controls.AddRange(new System.Windows.Forms.Control[] {
										  this.chkAddToSolution,
										  this.lblComment,
										  this.txtProjectFile,
										  this.label1,
										  this.btnDesigner,
										  this.btnCancel,
										  this.btnOK,
										  this.txtProjectName,
										  this.label2,
										  this.groupBox1,
										  this.btnBrowse});
		    this.Name = "NewQtProject2";
		    this.Text = "New Qt Project";
		    this.groupBox1.ResumeLayout(false);
		    this.ResumeLayout(false);

		}
		#endregion
	}
}
