################################################################################
#
# In this file, we can define environment variables for the CoreLab execution
# These environment variables will be active only during CoreLab execution
# These environment variables will be active for both rcl and dcl
#
# Powershell notes - be careful and always quote the environment variable values
# if they are not quoted, they are evaluated, and that may not be what you want (e.g. hexadecimals)
#
################################################################################

$log = $env:andrew_log
$clrgc = $env:andrew_clrgc
$trace = $env:andrew_trace
$server = $env:andrew_server

if ($log -eq "1")
{
    $env:COMPlus_StressLogFilename="D:\CoreLab.{pid}.log"
    $env:COMPlus_LogLevel="9"
    $env:COMPlus_StressLog="1"
    $env:COMPlus_StressLogSize="10000000"
    $env:COMPlus_TotalStressLogSize="20"
}

if ($clrgc -eq "1")
{
    if ($IsWindows -or $ENV:OS)
    {
        $env:COMPlus_GCName="clrgc.dll"
    }
    else
    {
        if ($IsLinux)
        {
            $env:COMPlus_GCName="libclrgc.so"
        }
        else
        {
            $env:COMPlus_GCName="libclrgc.dylib"
        }
    }
}

if ($trace -eq "1")
{
    # TODO, Linux
    $env:COMPlus_AutoTrace_N_Tracers="1"
    $env:COMPlus_AutoTrace_Command="C:\Dev\diagnostics\src\Tools\dotnet-trace\test.cmd"
}

if ($server -eq "1")
{
    $env:COMPlus_gcServer="1"
}