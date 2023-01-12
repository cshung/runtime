################################################################################
#
# In this file, we can perform work that should happen before running CoreLab
#
################################################################################

$log = $env:andrew_log
if ($log -eq "1")
{
    remove-item "$($logPath)*" -include CoreLab.*.log
}

# TODO
# reg add    "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\KnownManagedDebuggingDlls" /v "C:\Dev\diagnostics\.dotnet-test\shared\Microsoft.NETCore.App\8.0.0-rc.1.23406.6\mscordaccore.dll" /t REG_DWORD /d 0
# reg add    "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\MiniDumpAuxiliaryDlls" /v "C:\Dev\diagnostics\.dotnet-test\shared\Microsoft.NETCore.App\8.0.0-rc.1.23406.6\coreclr.dll" /t REG_SZ /d "C:\Dev\diagnostics\.dotnet-test\shared\Microsoft.NETCore.App\8.0.0-rc.1.23406.6\mscordaccore.dll"
