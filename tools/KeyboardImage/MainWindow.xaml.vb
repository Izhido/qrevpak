Class Window1 

    Private Sub Window1_PreviewKeyUp(ByVal sender As Object, ByVal e As System.Windows.Input.KeyEventArgs) Handles Me.PreviewKeyUp

        Dim sb As System.Text.StringBuilder
        Dim i As Integer
        Dim m As Integer
        Dim j As Integer
        Dim n As Integer
        Dim Visuals As List(Of DependencyObject)
        Dim h As HitTestResult
        Dim o As DependencyObject
        Dim k As Integer

        If e.Key = Key.Space Then
            sb = New System.Text.StringBuilder
            m = Me.Width
            n = Me.Height
            For k = 0 To 3
                Visuals = New List(Of DependencyObject)
                For j = 0 To n - 1
                    For i = 0 To m - 1
                        h = VisualTreeHelper.HitTest(Me, New Point(i, j))
                        If h IsNot Nothing Then
                            o = h.VisualHit
                            If o IsNot Nothing Then
                                If Not Visuals.Contains(o) Then
                                    Visuals.Add(o)
                                    If TypeOf o Is Rectangle Then
                                        sb.Append(k)
                                        sb.Append(",")
                                        sb.Append(i)
                                        sb.Append(",")
                                        sb.Append(j)
                                        sb.Append(",")
                                        sb.Append(i + Convert.ToInt32(DirectCast(o, Rectangle).ActualWidth))
                                        sb.Append(",")
                                        sb.Append(j + Convert.ToInt32(DirectCast(o, Rectangle).ActualHeight))
                                        sb.AppendLine(": ;")
                                    End If
                                End If
                            End If
                        End If
                    Next
                Next
            Next
            Using fs As IO.FileStream = IO.File.Create("Keys.txt")
                Using sw As New IO.StreamWriter(fs)
                    sw.Write(sb.ToString)
                End Using
            End Using
        End If

    End Sub

End Class
