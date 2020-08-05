Param (
    [string]$mode
)
$mode = $mode.ToLower()
if (-not($mode))
{
    Write-Output "The mode parameter is required"
    Exit
}
if (
    ($mode -ne "debug") -and
    ($mode -ne "checked") -and
    ($mode -ne "release")
)
{
    Write-Output "Unsupported build flavor specified"
    Exit
}
$env:andrew_mode = $mode