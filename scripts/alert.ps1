Param (
    [string]$msg
)

. (Join-Path $PSScriptRoot "common.ps1")

if (-not($msg))
{
    Write-Output "The msg parameter is required"
    Exit
}

Invoke-Expression "$($alert) `"$($msg)`""