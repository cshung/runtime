################################################################################
#
# In this file, we can define environment variables for the CoreLab execution
# These environment variables will be active only during CoreLab execution
# These environment variables will be active for both rcl and dcl
#
################################################################################

$log = $env:andrew_log
if ($log -eq "1")
{
    $env:COMPlus_StressLogFilename="D:\CoreLab.{pid}.log"
    $env:COMPlus_LogLevel="9"
    $env:COMPlus_StressLog="1"
    $env:COMPlus_StressLogSize="10000000"
    $env:COMPlus_TotalStressLogSize="20"
}