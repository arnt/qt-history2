Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents AxQAxWidget21 As AxmultipleaxLib.AxQAxWidget2
    Friend WithEvents AxQPushButton1 As AxwrapperaxLib.AxQPushButton
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Form1))
        Me.AxQAxWidget21 = New AxmultipleaxLib.AxQAxWidget2()
        Me.AxQPushButton1 = New AxwrapperaxLib.AxQPushButton()
        CType(Me.AxQAxWidget21, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.AxQPushButton1, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'AxQAxWidget21
        '
        Me.AxQAxWidget21.Enabled = True
        Me.AxQAxWidget21.Location = New System.Drawing.Point(8, 8)
        Me.AxQAxWidget21.Name = "AxQAxWidget21"
        Me.AxQAxWidget21.OcxState = CType(resources.GetObject("AxQAxWidget21.OcxState"), System.Windows.Forms.AxHost.State)
        Me.AxQAxWidget21.Size = New System.Drawing.Size(280, 216)
        Me.AxQAxWidget21.TabIndex = 0
        '
        'AxQPushButton1
        '
        ' Me.AxQPushButton1.enabled = True
        Me.AxQPushButton1.Location = New System.Drawing.Point(40, 240)
        Me.AxQPushButton1.Name = "AxQPushButton1"
        Me.AxQPushButton1.OcxState = CType(resources.GetObject("AxQPushButton1.OcxState"), System.Windows.Forms.AxHost.State)
        Me.AxQPushButton1.Size = New System.Drawing.Size(224, 24)
        Me.AxQPushButton1.TabIndex = 1
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(292, 273)
        Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.AxQPushButton1, Me.AxQAxWidget21})
        Me.Name = "Form1"
        Me.Text = "Form1"
        CType(Me.AxQAxWidget21, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.AxQPushButton1, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub

#End Region

    Private Sub AxQPushButton1_clicked(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles AxQPushButton1.clicked
        Me.AxQAxWidget21.lineWidth = 1
    End Sub

    Private Sub AxQAxWidget21_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles AxQAxWidget21.ClickEvent
        Me.AxQAxWidget21.lineWidth = Me.AxQAxWidget21.lineWidth + 1
    End Sub
End Class
