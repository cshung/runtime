################################################################################
#
# In this file, we can perform work that should happen after running CoreLab
#
################################################################################

$trace = $env:andrew_trace

if ($trace -eq "1")
{
    if ($IsWindows -or $ENV:OS)
    {
        Move-Item C:\Dev\diagnostics\src\Tools\dotnet-trace\*.nettrace -Destination C:\Dev\runtime\CoreLab
    }
    else
    {
        Move-Item "$($home)/git/diagnostics/src/Tools/dotnet-trace/*.nettrace" -Destination "$($home)/git/runtime/CoreLab"
    }
}