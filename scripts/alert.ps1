Param (
    [string]$msg
)

if (-not($msg))
{
    Write-Output "The msg parameter is required"
    Exit
}

# TODO: Cross Platform
Invoke-Expression "c:\toolssw\slack\alert $($msg)"