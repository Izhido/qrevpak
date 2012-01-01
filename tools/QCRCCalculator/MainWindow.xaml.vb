Class MainWindow 

    Private Sub EnteredTextBox_TextChanged(sender As System.Object, e As System.Windows.Controls.TextChangedEventArgs) Handles EnteredTextBox.TextChanged

        ResultLabel.Content = ""

    End Sub

    Private Sub GetCRCButton_Click(sender As System.Object, e As System.Windows.RoutedEventArgs) Handles GetCRCButton.Click

        Dim crc As Integer
        Dim ac As Integer

        crc = 0
        For Each c In EnteredTextBox.Text
            ac = AscW(c)
            If (ac > 32) AndAlso (ac <= 127) Then
                crc = crc + ac
                While crc > 65535
                    crc = crc - 65536
                End While
            End If
        Next
        ResultLabel.Content = "CRC = " + crc.ToString

    End Sub

End Class
