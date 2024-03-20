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
$datas = $env:andrew_datas
$exp = $env:andrew_exp

if ($log -eq "1")
{
    $env:DOTNET_StressLogFilename="$($logPath)CoreLab.{pid}.log"
    $env:DOTNET_LogLevel="9"
    $env:DOTNET_StressLog="1"
    $env:DOTNET_StressLogSize="10000000"
    $env:DOTNET_TotalStressLogSize="20"
}

if ($exp -eq "0")
{
    $clrgc_favor="clrgc"
}
else
{
    $clrgc_favor="clrgcexp"
}

if ($clrgc -eq "1")
{
    if ($IsWindows -or $ENV:OS)
    {
        $env:DOTNET_GCName="$($clrgc_favor).dll"
    }
    else
    {
        if ($IsLinux)
        {
            $env:DOTNET_GCName="lib$($clrgc_favor).so"
        }
        else
        {
            $env:DOTNET_GCName="lib$($clrgc_favor).dylib"
        }
    }
}

if ($trace -eq "1")
{
    $env:COMPlus_AutoTrace_N_Tracers="1"
    if ($IsWindows -or $ENV:OS)
    {
        $env:COMPlus_AutoTrace_Command="C:\Dev\diagnostics\src\Tools\dotnet-trace\test.cmd"
    }
    else
    {
        $env:COMPlus_AutoTrace_Command="/bin/bash $($home)/git/diagnostics/src/Tools/dotnet-trace/test.sh"
    }
}

if ($server -eq "1")
{
    $env:DOTNET_gcServer="1"
}

if ($datas -eq "1")
{
    $env:DOTNET_GCDynamicAdaptationMode="1"
}