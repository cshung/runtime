Param (
    [string]$arch
)
$arch = $arch.ToLower()
if (-not($arch))
{
    Write-Output "The arch parameter is required"
    Exit
}

#
# TODO - Support other architectures
#
if (
    ($arch -ne "x86") -and
    ($arch -ne "x64")
)
{
    Write-Output "Unsupported architecture"
    Exit
}
$env:andrew_arch = $arch