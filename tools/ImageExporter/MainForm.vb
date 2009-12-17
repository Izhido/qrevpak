Public Class MainForm

    Private Sub MainForm_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

        Dim dts As Data.DataSet
        Dim ds As IO.Compression.DeflateStream
        Dim dtr As Data.DataRow
        Dim itm As String
        Dim lst As List(Of String)

        SourceTextBox.Text = My.Settings.Source
        ScanSourceFiles()
        If My.Settings.Checked <> "" Then
            dts = New Data.DataSet
            ds = New IO.Compression.DeflateStream(New IO.MemoryStream(Convert.FromBase64String(My.Settings.Checked)), IO.Compression.CompressionMode.Decompress)
            dts.ReadXml(ds)
            ds.Close()
            For Each dtr In dts.Tables(0).Rows
                If SourceCheckedListBox.Items.Contains(dtr("File")) Then
                    SourceCheckedListBox.SetItemChecked(SourceCheckedListBox.Items.IndexOf(dtr("File")), True)
                End If
            Next
            lst = New List(Of String)
            For Each itm In SourceCheckedListBox.CheckedItems
                lst.Add(itm)
            Next
            For Each itm In lst
                If dts.Tables(0).Select("File = '" & itm & "'").Length = 0 Then
                    SourceCheckedListBox.SetItemChecked(SourceCheckedListBox.Items.IndexOf(itm), False)
                End If
            Next
        End If
        TargetTextBox.Text = My.Settings.Target
        Select Case My.Settings.Format
            Case 0
                Format16bitRGBARadioButton.Checked = True
            Case 1
                Format24bitRGBRadioButton.Checked = True
            Case 2
                Format32bitRGBARadioButton.Checked = True
            Case 3
                Format40BitAY1CbY2CrRadioButton.Checked = True
        End Select

    End Sub

    Private Sub ScanSourceFiles()

        Dim FileList As List(Of String)
        Dim Entry As String

        SourceCheckedListBox.Items.Clear()
        Try
            FileList = New List(Of String)
            FileList.AddRange(IO.Directory.GetFiles(SourceTextBox.Text, "*.bmp"))
            FileList.AddRange(IO.Directory.GetFiles(SourceTextBox.Text, "*.gif"))
            FileList.AddRange(IO.Directory.GetFiles(SourceTextBox.Text, "*.jpg"))
            FileList.AddRange(IO.Directory.GetFiles(SourceTextBox.Text, "*.pcx"))
            FileList.AddRange(IO.Directory.GetFiles(SourceTextBox.Text, "*.png"))
            For Each Entry In FileList
                SourceCheckedListBox.Items.Add(Entry, True)
            Next
        Catch
        End Try

    End Sub

    Private Sub SaveInfo()

        Dim dts As Data.DataSet
        Dim itm As String
        Dim dtr As Data.DataRow
        Dim ms As IO.MemoryStream
        Dim dr As IO.Compression.DeflateStream

        My.Settings("Source") = SourceTextBox.Text
        dts = New Data.DataSet
        dts.Tables.Add("Checked")
        dts.Tables(0).Columns.Add("File")
        For Each itm In SourceCheckedListBox.CheckedItems
            dtr = dts.Tables(0).NewRow
            dtr("File") = itm
            dts.Tables(0).Rows.Add(dtr)
        Next
        ms = New IO.MemoryStream()
        dr = New IO.Compression.DeflateStream(ms, IO.Compression.CompressionMode.Compress)
        dts.WriteXml(dr, XmlWriteMode.WriteSchema)
        dr.Close()
        My.Settings("Checked") = Convert.ToBase64String(ms.ToArray)
        My.Settings("Target") = TargetTextBox.Text
        If Format16bitRGBARadioButton.Checked Then
            My.Settings("Format") = 0
        ElseIf Format16bitRGBARadioButton.Checked Then
            My.Settings("Format") = 1
        ElseIf Format16bitRGBARadioButton.Checked Then
            My.Settings("Format") = 2
        ElseIf Format16bitRGBARadioButton.Checked Then
            My.Settings("Format") = 3
        End If
        My.Settings.Save()

    End Sub

    Private Sub SourceButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SourceButton.Click

        SourceFolderBrowserDialog.SelectedPath = SourceTextBox.Text
        If SourceFolderBrowserDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
            SourceTextBox.Text = SourceFolderBrowserDialog.SelectedPath
            ScanSourceFiles()
            SaveInfo()
        End If

    End Sub

    Private Sub TargetButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TargetButton.Click

        TargetFolderBrowserDialog.SelectedPath = TargetTextBox.Text
        If TargetFolderBrowserDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
            TargetTextBox.Text = TargetFolderBrowserDialog.SelectedPath
            SaveInfo()
        End If

    End Sub

    Private Sub MainForm_FormClosing(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing

        SaveInfo()

    End Sub

    Private Function RGBToY1CbY2Cr(ByVal r1 As Integer, ByVal g1 As Integer, ByVal b1 As Integer, ByVal r2 As Integer, ByVal g2 As Integer, ByVal b2 As Integer) As Integer

        Dim y1 As Integer
        Dim cb1 As Integer
        Dim cr1 As Integer
        Dim y2 As Integer
        Dim cb2 As Integer
        Dim cr2 As Integer
        Dim cb As Integer
        Dim cr As Integer

        y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000
        cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000
        cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000

        y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000
        cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000
        cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000

        cb = (cb1 + cb2) >> 1
        cr = (cr1 + cr2) >> 1

        Return (y1 << 24) Or (cb << 16) Or (y2 << 8) Or cr

    End Function

    Private Sub ExportButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ExportButton.Click

        Dim lst As List(Of String)
        Dim itm As String
        Dim bmp As Drawing.Bitmap
        Dim fs As IO.FileStream
        Dim i As Integer
        Dim j As Integer
        Dim m As Integer
        Dim n As Integer
        Dim c As Color
        Dim r1 As Integer
        Dim g1 As Integer
        Dim b1 As Integer
        Dim r2 As Integer
        Dim g2 As Integer
        Dim b2 As Integer
        Dim fc As Integer

        Cursor = Cursors.WaitCursor
        Try
            lst = New List(Of String)
            For Each itm In SourceCheckedListBox.CheckedItems
                lst.Add(itm)
            Next
            For Each itm In lst
                SourceCheckedListBox.SelectedItem = itm
                Application.DoEvents()
                bmp = New Drawing.Bitmap(itm)
                fs = New IO.FileStream(IO.Path.Combine(TargetTextBox.Text, IO.Path.GetFileNameWithoutExtension(itm) & ".img"), IO.FileMode.Create, IO.FileAccess.Write, IO.FileShare.Read)
                If Format16bitRGBARadioButton.Checked Then
                    m = bmp.Width
                    n = bmp.Height
                    For j = 0 To n - 1
                        For i = 0 To m - 1
                            c = bmp.GetPixel(i, j)
                            fs.WriteByte((((c.G \ 8) And 7) * 32) Or (c.R \ 8))
                            fs.WriteByte(((c.A \ 128) * 128) Or ((c.B \ 8) * 4) Or (c.G \ 64))
                        Next
                    Next
                ElseIf Format24bitRGBRadioButton.Checked Then
                    m = bmp.Width
                    n = bmp.Height
                    For j = 0 To n - 1
                        For i = 0 To m - 1
                            c = bmp.GetPixel(i, j)
                            fs.WriteByte(c.R)
                            fs.WriteByte(c.G)
                            fs.WriteByte(c.B)
                        Next
                    Next
                ElseIf Format32bitRGBARadioButton.Checked Then
                    m = bmp.Width
                    n = bmp.Height
                    For j = 0 To n - 1
                        For i = 0 To m - 1
                            c = bmp.GetPixel(i, j)
                            fs.WriteByte(c.R)
                            fs.WriteByte(c.G)
                            fs.WriteByte(c.B)
                            fs.WriteByte(c.A)
                        Next
                    Next
                ElseIf Format40BitAY1CbY2CrRadioButton.Checked Then
                    m = bmp.Width
                    If (m And 1) = 1 Then
                        MsgBox("The image " & itm & " has a non-even width (" & m & ") and will not be converted.")
                    Else
                        n = bmp.Height
                        For j = 0 To n - 1
                            i = 0
                            While i < m
                                c = bmp.GetPixel(i, j)
                                r1 = c.R
                                g1 = c.G
                                b1 = c.B
                                i = i + 1
                                c = bmp.GetPixel(i, j)
                                r2 = c.R
                                g2 = c.G
                                b2 = c.B
                                i = i + 1
                                fc = RGBToY1CbY2Cr(r1, g1, b1, r2, g2, b2)
                                If r1 = Format40BitAY1CbY2CrTransparentRNumericUpDown.Value _
                                And g1 = Format40BitAY1CbY2CrTransparentGNumericUpDown.Value _
                                And b1 = Format40BitAY1CbY2CrTransparentBNumericUpDown.Value _
                                And r2 = Format40BitAY1CbY2CrTransparentRNumericUpDown.Value _
                                And g2 = Format40BitAY1CbY2CrTransparentGNumericUpDown.Value _
                                And b2 = Format40BitAY1CbY2CrTransparentBNumericUpDown.Value _
                                Then
                                    fs.WriteByte(0)
                                Else
                                    fs.WriteByte(255)
                                End If
                                fs.WriteByte((fc >> 24) And 255)
                                fs.WriteByte((fc >> 16) And 255)
                                fs.WriteByte((fc >> 8) And 255)
                                fs.WriteByte((fc >> 0) And 255)
                            End While
                        Next
                    End If
                End If
                fs.Close()
            Next
            MsgBox("Export successful.")
        Finally
            Cursor = Cursors.Default
        End Try

    End Sub

    Private Sub Format40BitAY1CbY2CrRadioButton_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Format40BitAY1CbY2CrRadioButton.CheckedChanged

        Format40BitAY1CbY2CrTransparentLabel.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentRLabel.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentGLabel.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentBLabel.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentRNumericUpDown.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentGNumericUpDown.Visible = Format40BitAY1CbY2CrRadioButton.Checked
        Format40BitAY1CbY2CrTransparentBNumericUpDown.Visible = Format40BitAY1CbY2CrRadioButton.Checked

    End Sub

End Class
