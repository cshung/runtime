################################################################################
#
# In this file, we can perform work that should happen before running CoreLab
#
################################################################################

$log = $env:andrew_log
if ($log -eq "1")
{
    remove-item D:\* -include CoreLab.*.log
}