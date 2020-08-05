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
$server = $env:andrew_server
$exp = $env:andrew_exp

if ($log -eq "1")
{
    # TODO, andrewau, make this a config
    if ($IsWindows -or $ENV:OS)
    {
        $env:COMPlus_StressLogFilename="D:\CoreLab.{pid}.log"
    }
    else
    {
        $env:COMPlus_StressLogFilename="$($home)/CoreLab.{pid}.log"
    }
    $env:COMPlus_LogLevel="9"
    $env:COMPlus_StressLog="1"
    $env:COMPlus_StressLogSize="10000000"
    $env:COMPlus_TotalStressLogSize="20"
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
        $env:COMPlus_GCName="$($clrgc_favor).dll"
    }
    else
    {
        if ($IsLinux)
        {
            $env:COMPlus_GCName="lib$($clrgc_favor).so"
        }
        else
        {
            $env:COMPlus_GCName="lib$($clrgc_favor).dylib"
        }
    }
}

if ($server -eq "1")
{
    $env:COMPlus_gcServer="1"
}