################################################################################
#
# In this file, we can perform work that should happen before running CoreLab
#
################################################################################

$trace = $env:andrew_trace
$log = $env:andrew_log

if ($trace -eq "1")
{
    # TODO Linux
    remove-item C:\Dev\runtime\CoreLab\* -include *.nettrace
    remove-item C:\Dev\diagnostics\src\Tools\dotnet-trace\* -include *.nettrace
}

if ($log -eq "1")
{
    remove-item D:\* -include CoreLab.*.log
}