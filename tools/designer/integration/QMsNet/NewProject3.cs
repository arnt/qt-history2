using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace QMsNet
{
    /// <summary>
    /// Summary description for NewProject3.
    /// </summary>
    public class NewQtProject3 : System.Windows.Forms.Form
    {
	public System.Windows.Forms.CheckBox chkAddToSolution;
	public System.Windows.Forms.TextBox txtProjectFile;
	private System.Windows.Forms.Label label1;
	private System.Windows.Forms.Label label2;
	private System.Windows.Forms.Button btnBrowse;
	public System.Windows.Forms.SaveFileDialog saveProDialog;
	private System.Windows.Forms.Button btnCancel;
	private System.Windows.Forms.Button btnOK;
	public System.Windows.Forms.TextBox txtProjectName;
	private System.Windows.Forms.Label label3;
	private System.Windows.Forms.Panel panel1;
	public System.Windows.Forms.ListBox lstTemplates;
	private System.Windows.Forms.Splitter splitter1;
	public System.Windows.Forms.RichTextBox txtDescription;
	private System.ComponentModel.Container components = null;
	private bool autoUpdateFilename = true;
	private bool autoUpdateProjectName = true;

	public NewQtProject3()
	{
	    InitializeComponent();
	}

	protected override void Dispose( bool disposing )
	{
	    if( disposing ) {
		if(components != null) {
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
	    this.txtProjectFile = new System.Windows.Forms.TextBox();
	    this.label1 = new System.Windows.Forms.Label();
	    this.label2 = new System.Windows.Forms.Label();
	    this.btnBrowse = new System.Windows.Forms.Button();
	    this.saveProDialog = new System.Windows.Forms.SaveFileDialog();
	    this.btnCancel = new System.Windows.Forms.Button();
	    this.btnOK = new System.Windows.Forms.Button();
	    this.txtProjectName = new System.Windows.Forms.TextBox();
	    this.label3 = new System.Windows.Forms.Label();
	    this.panel1 = new System.Windows.Forms.Panel();
	    this.txtDescription = new System.Windows.Forms.RichTextBox();
	    this.splitter1 = new System.Windows.Forms.Splitter();
	    this.lstTemplates = new System.Windows.Forms.ListBox();
	    this.panel1.SuspendLayout();
	    this.SuspendLayout();
	    // 
	    // chkAddToSolution
	    // 
	    this.chkAddToSolution.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
	    this.chkAddToSolution.Checked = true;
	    this.chkAddToSolution.CheckState = System.Windows.Forms.CheckState.Checked;
	    this.chkAddToSolution.Location = new System.Drawing.Point(8, 195);
	    this.chkAddToSolution.Name = "chkAddToSolution";
	    this.chkAddToSolution.Size = new System.Drawing.Size(144, 16);
	    this.chkAddToSolution.TabIndex = 4;
	    this.chkAddToSolution.Text = "Add to current S&olution";
	    // 
	    // txtProjectFile
	    // 
	    this.txtProjectFile.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
		| System.Windows.Forms.AnchorStyles.Right);
	    this.txtProjectFile.Location = new System.Drawing.Point(88, 32);
	    this.txtProjectFile.Name = "txtProjectFile";
	    this.txtProjectFile.Size = new System.Drawing.Size(232, 20);
	    this.txtProjectFile.TabIndex = 1;
	    this.txtProjectFile.Text = "";
	    this.txtProjectFile.TextChanged += new System.EventHandler(this.txtProjectFile_TextChanged);
	    // 
	    // label1
	    // 
	    this.label1.Location = new System.Drawing.Point(8, 34);
	    this.label1.Name = "label1";
	    this.label1.Size = new System.Drawing.Size(76, 16);
	    this.label1.TabIndex = 31;
	    this.label1.Text = "Project &File:";
	    // 
	    // label2
	    // 
	    this.label2.Location = new System.Drawing.Point(8, 10);
	    this.label2.Name = "label2";
	    this.label2.Size = new System.Drawing.Size(76, 16);
	    this.label2.TabIndex = 24;
	    this.label2.Text = "Project &Name:";
	    // 
	    // btnBrowse
	    // 
	    this.btnBrowse.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
	    this.btnBrowse.BackColor = System.Drawing.SystemColors.Control;
	    this.btnBrowse.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
	    this.btnBrowse.Location = new System.Drawing.Point(320, 32);
	    this.btnBrowse.Name = "btnBrowse";
	    this.btnBrowse.Size = new System.Drawing.Size(24, 21);
	    this.btnBrowse.TabIndex = 2;
	    this.btnBrowse.Text = "...";
	    this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
	    // 
	    // saveProDialog
	    // 
	    this.saveProDialog.DefaultExt = "pro";
	    this.saveProDialog.FileName = "newProject.pro";
	    this.saveProDialog.Filter = "Qt Project files|*.pro|All files|*.*";
	    this.saveProDialog.Title = "New Qt project file...";
	    // 
	    // btnCancel
	    // 
	    this.btnCancel.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
	    this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
	    this.btnCancel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
	    this.btnCancel.Location = new System.Drawing.Point(264, 192);
	    this.btnCancel.Name = "btnCancel";
	    this.btnCancel.TabIndex = 7;
	    this.btnCancel.Text = "&Cancel";
	    // 
	    // btnOK
	    // 
	    this.btnOK.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
	    this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
	    this.btnOK.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
	    this.btnOK.Location = new System.Drawing.Point(176, 192);
	    this.btnOK.Name = "btnOK";
	    this.btnOK.TabIndex = 6;
	    this.btnOK.Text = "&OK";
	    // 
	    // txtProjectName
	    // 
	    this.txtProjectName.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
		| System.Windows.Forms.AnchorStyles.Right);
	    this.txtProjectName.Location = new System.Drawing.Point(88, 8);
	    this.txtProjectName.Name = "txtProjectName";
	    this.txtProjectName.Size = new System.Drawing.Size(256, 20);
	    this.txtProjectName.TabIndex = 0;
	    this.txtProjectName.Text = "newProject";
	    this.txtProjectName.TextChanged += new System.EventHandler(this.txtProjectName_TextChanged);
	    // 
	    // label3
	    // 
	    this.label3.Location = new System.Drawing.Point(8, 56);
	    this.label3.Name = "label3";
	    this.label3.Size = new System.Drawing.Size(56, 16);
	    this.label3.TabIndex = 35;
	    this.label3.Text = "Template:";
	    // 
	    // panel1
	    // 
	    this.panel1.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
		| System.Windows.Forms.AnchorStyles.Left) 
		| System.Windows.Forms.AnchorStyles.Right);
	    this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
	    this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
										 this.txtDescription,
										 this.splitter1,
										 this.lstTemplates});
	    this.panel1.Location = new System.Drawing.Point(8, 72);
	    this.panel1.Name = "panel1";
	    this.panel1.Size = new System.Drawing.Size(336, 112);
	    this.panel1.TabIndex = 3;
	    // 
	    // txtDescription
	    // 
	    this.txtDescription.AcceptsTab = true;
	    this.txtDescription.AutoSize = true;
	    this.txtDescription.BackColor = System.Drawing.SystemColors.Control;
	    this.txtDescription.BorderStyle = System.Windows.Forms.BorderStyle.None;
	    this.txtDescription.Dock = System.Windows.Forms.DockStyle.Fill;
	    this.txtDescription.Location = new System.Drawing.Point(120, 0);
	    this.txtDescription.Name = "txtDescription";
	    this.txtDescription.ReadOnly = true;
	    this.txtDescription.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
	    this.txtDescription.Size = new System.Drawing.Size(214, 110);
	    this.txtDescription.TabIndex = 2;
	    this.txtDescription.TabStop = false;
	    this.txtDescription.Text = "<description>";
	    // 
	    // splitter1
	    // 
	    this.splitter1.BackColor = System.Drawing.SystemColors.Control;
	    this.splitter1.Location = new System.Drawing.Point(112, 0);
	    this.splitter1.Name = "splitter1";
	    this.splitter1.Size = new System.Drawing.Size(8, 110);
	    this.splitter1.TabIndex = 0;
	    this.splitter1.TabStop = false;
	    // 
	    // lstTemplates
	    // 
	    this.lstTemplates.BorderStyle = System.Windows.Forms.BorderStyle.None;
	    this.lstTemplates.Dock = System.Windows.Forms.DockStyle.Left;
	    this.lstTemplates.IntegralHeight = false;
	    this.lstTemplates.Name = "lstTemplates";
	    this.lstTemplates.Size = new System.Drawing.Size(112, 110);
	    this.lstTemplates.Sorted = true;
	    this.lstTemplates.TabIndex = 0;
	    this.lstTemplates.SelectedIndexChanged += new System.EventHandler(this.lstTemplates_SelectedIndexChanged);
	    // 
	    // NewQtProject3
	    // 
	    this.AcceptButton = this.btnOK;
	    this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
	    this.CancelButton = this.btnCancel;
	    this.ClientSize = new System.Drawing.Size(352, 222);
	    this.Controls.AddRange(new System.Windows.Forms.Control[] {
									  this.label3,
									  this.chkAddToSolution,
									  this.btnCancel,
									  this.btnOK,
									  this.txtProjectFile,
									  this.label1,
									  this.label2,
									  this.btnBrowse,
									  this.txtProjectName,
									  this.panel1});
	    this.MinimumSize = new System.Drawing.Size(304, 216);
	    this.Name = "NewQtProject3";
	    this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
	    this.Text = "New Qt project...";
	    this.panel1.ResumeLayout(false);
	    this.ResumeLayout(false);

	}
	#endregion

	private void btnBrowse_Click(object sender, System.EventArgs e) {
	    if ( saveProDialog.ShowDialog( this ) == DialogResult.OK ) {
		autoUpdateFilename = false;
		txtProjectFile.Text = saveProDialog.FileName;
	    }
	}

        private void lstTemplates_SelectedIndexChanged(object sender, System.EventArgs e) {
	    Template tl = (Template)Res.templates[lstTemplates.SelectedItem];
	    txtDescription.Clear();
	    txtDescription.Text  = "Current Qt configuration:\n   ";
	    txtDescription.Text += (Res.qtDebug  ? "Debug" : "Release") + ", ";
	    txtDescription.Text += (Res.qtShared ? "Shared" : "Static") + ", ";
	    txtDescription.Text += (Res.qtThread ? "Multi threaded\n" : "Single threaded\n");
	    txtDescription.Text += "---\n";
	    if ( tl != null )
		txtDescription.Text += tl.description;
	    txtDescription.Select( 0, 29 );
	    txtDescription.SelectionColor = Color.FromKnownColor(KnownColor.ControlDarkDark);
	    txtDescription.Select( 0, 0 );
	}

        private void txtProjectFile_TextChanged(object sender, System.EventArgs e) {
	    if ( txtProjectFile.Focused )
		autoUpdateFilename = false;

	    if ( autoUpdateProjectName ) {
		string newName = txtProjectFile.Text;
		int index = newName.LastIndexOf( "\\" ) + 1;
		if ( index != 0 )
		    newName = newName.Substring( index, newName.Length - index );
		if ( newName.EndsWith( ".pro" ) )
		    newName = newName.Substring( 0, newName.Length - 4 );
		txtProjectName.Text = newName;
	    }
	}

        private void txtProjectName_TextChanged(object sender, System.EventArgs e) {
	    if ( txtProjectName.Focused )
		autoUpdateProjectName = false;

	    if ( autoUpdateFilename ) {
		txtProjectFile.Text = txtProjectName.Text + ".pro";
	    }
	}
    }
}
