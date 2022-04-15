del C:\dev\runtime\artifacts\tests\coreclr\windows.x64.Debug\profiler\gc\gc\*.txt
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\corerun.exe" /v Debugger /t REG_SZ /d "c:\toolssw\debuggers\amd64\windbg.exe -c $$>a<c:\dev\runtime\autodbg\profiler.script" /f
call c:\dev\runtime\artifacts\tests\coreclr\windows.x64.Debug\profiler\gc\gc\gc.cmd