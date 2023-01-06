################################################################################
#
# In this file, we can perform work that should happen after running CoreLab
#
################################################################################

$trace = $env:andrew_trace

if ($trace -eq "1")
{
    # TODO, Linux
    Move-Item C:\Dev\diagnostics\src\Tools\dotnet-trace\*.nettrace -Destination C:\Dev\runtime\CoreLab
}