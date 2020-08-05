Param (
    [string]$nested
)

. (Join-Path $PSScriptRoot "common.ps1")
. (Join-Path $PSScriptRoot "exec-helpers.ps1")

if (-not($nested))
{
    $script_with_parameter = "$($powershell_cmd) $($PSScriptRoot)\debug-corelab.ps1 1"
    Invoke-Expression $script_with_parameter
    Exit
}

switch($mode)
{
    "debug"   { $mode_part = "Debug"   }
    "checked" { $mode_part = "Checked" }
    "release" { $mode_part = "Release" }
    Default   { $mode_part = "Debug"   }
}

$script_path         = path "$($PSScriptRoot)\..\CoreLab"
$corelab_binary_path = path "$($PSScriptRoot)\..\artifacts\bin\CoreLab\$($mode_part)\net7.0\$($rid)\publish\CoreLab"

pushd $script_path
run $corelab_binary_path $True
popd
