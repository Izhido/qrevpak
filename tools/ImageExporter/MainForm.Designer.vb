<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class MainForm
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.SourceTextBox = New System.Windows.Forms.TextBox
        Me.SourceButton = New System.Windows.Forms.Button
        Me.SourceFolderBrowserDialog = New System.Windows.Forms.FolderBrowserDialog
        Me.TargetButton = New System.Windows.Forms.Button
        Me.TargetTextBox = New System.Windows.Forms.TextBox
        Me.TargetFolderBrowserDialog = New System.Windows.Forms.FolderBrowserDialog
        Me.SourceGroupBox = New System.Windows.Forms.GroupBox
        Me.SourceCheckedListBox = New System.Windows.Forms.CheckedListBox
        Me.GroupBox1 = New System.Windows.Forms.GroupBox
        Me.Format40BitAY1CbY2CrTransparentBLabel = New System.Windows.Forms.Label
        Me.Format40BitAY1CbY2CrTransparentGLabel = New System.Windows.Forms.Label
        Me.Format40BitAY1CbY2CrTransparentRLabel = New System.Windows.Forms.Label
        Me.Format40BitAY1CbY2CrTransparentLabel = New System.Windows.Forms.Label
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown = New System.Windows.Forms.NumericUpDown
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown = New System.Windows.Forms.NumericUpDown
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown = New System.Windows.Forms.NumericUpDown
        Me.Format40BitAY1CbY2CrRadioButton = New System.Windows.Forms.RadioButton
        Me.Format24bitRGBRadioButton = New System.Windows.Forms.RadioButton
        Me.Format32bitRGBARadioButton = New System.Windows.Forms.RadioButton
        Me.FormatLabel = New System.Windows.Forms.Label
        Me.Format16bitRGBARadioButton = New System.Windows.Forms.RadioButton
        Me.ExportButton = New System.Windows.Forms.Button
        Me.SourceGroupBox.SuspendLayout()
        Me.GroupBox1.SuspendLayout()
        CType(Me.Format40BitAY1CbY2CrTransparentBNumericUpDown, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Format40BitAY1CbY2CrTransparentGNumericUpDown, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.Format40BitAY1CbY2CrTransparentRNumericUpDown, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'SourceTextBox
        '
        Me.SourceTextBox.Location = New System.Drawing.Point(6, 20)
        Me.SourceTextBox.Name = "SourceTextBox"
        Me.SourceTextBox.Size = New System.Drawing.Size(254, 20)
        Me.SourceTextBox.TabIndex = 0
        '
        'SourceButton
        '
        Me.SourceButton.Location = New System.Drawing.Point(266, 19)
        Me.SourceButton.Name = "SourceButton"
        Me.SourceButton.Size = New System.Drawing.Size(28, 23)
        Me.SourceButton.TabIndex = 1
        Me.SourceButton.Text = "..."
        Me.SourceButton.UseVisualStyleBackColor = True
        '
        'TargetButton
        '
        Me.TargetButton.Location = New System.Drawing.Point(260, 18)
        Me.TargetButton.Name = "TargetButton"
        Me.TargetButton.Size = New System.Drawing.Size(34, 23)
        Me.TargetButton.TabIndex = 3
        Me.TargetButton.Text = "..."
        Me.TargetButton.UseVisualStyleBackColor = True
        '
        'TargetTextBox
        '
        Me.TargetTextBox.Location = New System.Drawing.Point(6, 20)
        Me.TargetTextBox.Name = "TargetTextBox"
        Me.TargetTextBox.Size = New System.Drawing.Size(248, 20)
        Me.TargetTextBox.TabIndex = 2
        '
        'SourceGroupBox
        '
        Me.SourceGroupBox.Controls.Add(Me.SourceCheckedListBox)
        Me.SourceGroupBox.Controls.Add(Me.SourceTextBox)
        Me.SourceGroupBox.Controls.Add(Me.SourceButton)
        Me.SourceGroupBox.Location = New System.Drawing.Point(12, 12)
        Me.SourceGroupBox.Name = "SourceGroupBox"
        Me.SourceGroupBox.Size = New System.Drawing.Size(300, 273)
        Me.SourceGroupBox.TabIndex = 4
        Me.SourceGroupBox.TabStop = False
        Me.SourceGroupBox.Text = "Source:"
        '
        'SourceCheckedListBox
        '
        Me.SourceCheckedListBox.FormattingEnabled = True
        Me.SourceCheckedListBox.Location = New System.Drawing.Point(6, 56)
        Me.SourceCheckedListBox.Name = "SourceCheckedListBox"
        Me.SourceCheckedListBox.Size = New System.Drawing.Size(288, 214)
        Me.SourceCheckedListBox.TabIndex = 2
        '
        'GroupBox1
        '
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentBLabel)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentGLabel)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentRLabel)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentLabel)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentBNumericUpDown)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentGNumericUpDown)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrTransparentRNumericUpDown)
        Me.GroupBox1.Controls.Add(Me.Format40BitAY1CbY2CrRadioButton)
        Me.GroupBox1.Controls.Add(Me.Format24bitRGBRadioButton)
        Me.GroupBox1.Controls.Add(Me.Format32bitRGBARadioButton)
        Me.GroupBox1.Controls.Add(Me.FormatLabel)
        Me.GroupBox1.Controls.Add(Me.Format16bitRGBARadioButton)
        Me.GroupBox1.Controls.Add(Me.TargetTextBox)
        Me.GroupBox1.Controls.Add(Me.TargetButton)
        Me.GroupBox1.Location = New System.Drawing.Point(318, 12)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(300, 273)
        Me.GroupBox1.TabIndex = 5
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Target:"
        '
        'Format40BitAY1CbY2CrTransparentBLabel
        '
        Me.Format40BitAY1CbY2CrTransparentBLabel.Location = New System.Drawing.Point(254, 159)
        Me.Format40BitAY1CbY2CrTransparentBLabel.Name = "Format40BitAY1CbY2CrTransparentBLabel"
        Me.Format40BitAY1CbY2CrTransparentBLabel.Size = New System.Drawing.Size(40, 16)
        Me.Format40BitAY1CbY2CrTransparentBLabel.TabIndex = 15
        Me.Format40BitAY1CbY2CrTransparentBLabel.Text = "B"
        Me.Format40BitAY1CbY2CrTransparentBLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Format40BitAY1CbY2CrTransparentBLabel.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentGLabel
        '
        Me.Format40BitAY1CbY2CrTransparentGLabel.Location = New System.Drawing.Point(198, 159)
        Me.Format40BitAY1CbY2CrTransparentGLabel.Name = "Format40BitAY1CbY2CrTransparentGLabel"
        Me.Format40BitAY1CbY2CrTransparentGLabel.Size = New System.Drawing.Size(40, 16)
        Me.Format40BitAY1CbY2CrTransparentGLabel.TabIndex = 14
        Me.Format40BitAY1CbY2CrTransparentGLabel.Text = "G"
        Me.Format40BitAY1CbY2CrTransparentGLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Format40BitAY1CbY2CrTransparentGLabel.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentRLabel
        '
        Me.Format40BitAY1CbY2CrTransparentRLabel.Location = New System.Drawing.Point(139, 159)
        Me.Format40BitAY1CbY2CrTransparentRLabel.Name = "Format40BitAY1CbY2CrTransparentRLabel"
        Me.Format40BitAY1CbY2CrTransparentRLabel.Size = New System.Drawing.Size(40, 16)
        Me.Format40BitAY1CbY2CrTransparentRLabel.TabIndex = 13
        Me.Format40BitAY1CbY2CrTransparentRLabel.Text = "R"
        Me.Format40BitAY1CbY2CrTransparentRLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Format40BitAY1CbY2CrTransparentRLabel.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentLabel
        '
        Me.Format40BitAY1CbY2CrTransparentLabel.Location = New System.Drawing.Point(139, 143)
        Me.Format40BitAY1CbY2CrTransparentLabel.Name = "Format40BitAY1CbY2CrTransparentLabel"
        Me.Format40BitAY1CbY2CrTransparentLabel.Size = New System.Drawing.Size(159, 16)
        Me.Format40BitAY1CbY2CrTransparentLabel.TabIndex = 12
        Me.Format40BitAY1CbY2CrTransparentLabel.Text = "Transparent color:"
        Me.Format40BitAY1CbY2CrTransparentLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        Me.Format40BitAY1CbY2CrTransparentLabel.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentBNumericUpDown
        '
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown.Location = New System.Drawing.Point(254, 178)
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown.Name = "Format40BitAY1CbY2CrTransparentBNumericUpDown"
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown.Size = New System.Drawing.Size(40, 20)
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown.TabIndex = 11
        Me.Format40BitAY1CbY2CrTransparentBNumericUpDown.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentGNumericUpDown
        '
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown.Location = New System.Drawing.Point(198, 178)
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown.Name = "Format40BitAY1CbY2CrTransparentGNumericUpDown"
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown.Size = New System.Drawing.Size(40, 20)
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown.TabIndex = 10
        Me.Format40BitAY1CbY2CrTransparentGNumericUpDown.Visible = False
        '
        'Format40BitAY1CbY2CrTransparentRNumericUpDown
        '
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown.Location = New System.Drawing.Point(139, 178)
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown.Name = "Format40BitAY1CbY2CrTransparentRNumericUpDown"
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown.Size = New System.Drawing.Size(38, 20)
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown.TabIndex = 9
        Me.Format40BitAY1CbY2CrTransparentRNumericUpDown.Visible = False
        '
        'Format40BitAY1CbY2CrRadioButton
        '
        Me.Format40BitAY1CbY2CrRadioButton.AutoSize = True
        Me.Format40BitAY1CbY2CrRadioButton.Location = New System.Drawing.Point(9, 178)
        Me.Format40BitAY1CbY2CrRadioButton.Name = "Format40BitAY1CbY2CrRadioButton"
        Me.Format40BitAY1CbY2CrRadioButton.Size = New System.Drawing.Size(113, 17)
        Me.Format40BitAY1CbY2CrRadioButton.TabIndex = 8
        Me.Format40BitAY1CbY2CrRadioButton.TabStop = True
        Me.Format40BitAY1CbY2CrRadioButton.Text = "40-bit A-Y1CbY2Cr"
        Me.Format40BitAY1CbY2CrRadioButton.UseVisualStyleBackColor = True
        '
        'Format24bitRGBRadioButton
        '
        Me.Format24bitRGBRadioButton.AutoSize = True
        Me.Format24bitRGBRadioButton.Location = New System.Drawing.Point(9, 106)
        Me.Format24bitRGBRadioButton.Name = "Format24bitRGBRadioButton"
        Me.Format24bitRGBRadioButton.Size = New System.Drawing.Size(77, 17)
        Me.Format24bitRGBRadioButton.TabIndex = 7
        Me.Format24bitRGBRadioButton.TabStop = True
        Me.Format24bitRGBRadioButton.Text = "24-bit RGB"
        Me.Format24bitRGBRadioButton.UseVisualStyleBackColor = True
        '
        'Format32bitRGBARadioButton
        '
        Me.Format32bitRGBARadioButton.AutoSize = True
        Me.Format32bitRGBARadioButton.Location = New System.Drawing.Point(9, 129)
        Me.Format32bitRGBARadioButton.Name = "Format32bitRGBARadioButton"
        Me.Format32bitRGBARadioButton.Size = New System.Drawing.Size(84, 17)
        Me.Format32bitRGBARadioButton.TabIndex = 6
        Me.Format32bitRGBARadioButton.TabStop = True
        Me.Format32bitRGBARadioButton.Text = "32-bit RGBA"
        Me.Format32bitRGBARadioButton.UseVisualStyleBackColor = True
        '
        'FormatLabel
        '
        Me.FormatLabel.AutoSize = True
        Me.FormatLabel.Location = New System.Drawing.Point(6, 56)
        Me.FormatLabel.Name = "FormatLabel"
        Me.FormatLabel.Size = New System.Drawing.Size(42, 13)
        Me.FormatLabel.TabIndex = 5
        Me.FormatLabel.Text = "Format:"
        '
        'Format16bitRGBARadioButton
        '
        Me.Format16bitRGBARadioButton.AutoSize = True
        Me.Format16bitRGBARadioButton.Location = New System.Drawing.Point(9, 83)
        Me.Format16bitRGBARadioButton.Name = "Format16bitRGBARadioButton"
        Me.Format16bitRGBARadioButton.Size = New System.Drawing.Size(84, 17)
        Me.Format16bitRGBARadioButton.TabIndex = 4
        Me.Format16bitRGBARadioButton.TabStop = True
        Me.Format16bitRGBARadioButton.Text = "16-bit RGBA"
        Me.Format16bitRGBARadioButton.UseVisualStyleBackColor = True
        '
        'ExportButton
        '
        Me.ExportButton.Location = New System.Drawing.Point(276, 292)
        Me.ExportButton.Name = "ExportButton"
        Me.ExportButton.Size = New System.Drawing.Size(75, 23)
        Me.ExportButton.TabIndex = 6
        Me.ExportButton.Text = "Export"
        Me.ExportButton.UseVisualStyleBackColor = True
        '
        'MainForm
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(630, 327)
        Me.Controls.Add(Me.ExportButton)
        Me.Controls.Add(Me.GroupBox1)
        Me.Controls.Add(Me.SourceGroupBox)
        Me.Name = "MainForm"
        Me.Text = "Image exporter"
        Me.SourceGroupBox.ResumeLayout(False)
        Me.SourceGroupBox.PerformLayout()
        Me.GroupBox1.ResumeLayout(False)
        Me.GroupBox1.PerformLayout()
        CType(Me.Format40BitAY1CbY2CrTransparentBNumericUpDown, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Format40BitAY1CbY2CrTransparentGNumericUpDown, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.Format40BitAY1CbY2CrTransparentRNumericUpDown, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents SourceTextBox As System.Windows.Forms.TextBox
    Friend WithEvents SourceButton As System.Windows.Forms.Button
    Friend WithEvents SourceFolderBrowserDialog As System.Windows.Forms.FolderBrowserDialog
    Friend WithEvents TargetButton As System.Windows.Forms.Button
    Friend WithEvents TargetTextBox As System.Windows.Forms.TextBox
    Friend WithEvents TargetFolderBrowserDialog As System.Windows.Forms.FolderBrowserDialog
    Friend WithEvents SourceGroupBox As System.Windows.Forms.GroupBox
    Friend WithEvents GroupBox1 As System.Windows.Forms.GroupBox
    Friend WithEvents ExportButton As System.Windows.Forms.Button
    Friend WithEvents SourceCheckedListBox As System.Windows.Forms.CheckedListBox
    Friend WithEvents FormatLabel As System.Windows.Forms.Label
    Friend WithEvents Format16bitRGBARadioButton As System.Windows.Forms.RadioButton
    Friend WithEvents Format32bitRGBARadioButton As System.Windows.Forms.RadioButton
    Friend WithEvents Format24bitRGBRadioButton As System.Windows.Forms.RadioButton
    Friend WithEvents Format40BitAY1CbY2CrRadioButton As System.Windows.Forms.RadioButton
    Friend WithEvents Format40BitAY1CbY2CrTransparentBNumericUpDown As System.Windows.Forms.NumericUpDown
    Friend WithEvents Format40BitAY1CbY2CrTransparentGNumericUpDown As System.Windows.Forms.NumericUpDown
    Friend WithEvents Format40BitAY1CbY2CrTransparentRNumericUpDown As System.Windows.Forms.NumericUpDown
    Friend WithEvents Format40BitAY1CbY2CrTransparentBLabel As System.Windows.Forms.Label
    Friend WithEvents Format40BitAY1CbY2CrTransparentGLabel As System.Windows.Forms.Label
    Friend WithEvents Format40BitAY1CbY2CrTransparentRLabel As System.Windows.Forms.Label
    Friend WithEvents Format40BitAY1CbY2CrTransparentLabel As System.Windows.Forms.Label

End Class
