Param (
    [string]$nested
)

. (Join-Path $PSScriptRoot "common.ps1")
. (Join-Path $PSScriptRoot "exec-helpers.ps1")

if ($env:andrew_failed -eq "1")
{
    Write-Output "$($(Split-Path $PSCommandPath -leaf)) skipped execution because previous build failed"
    return
}

if (-not($nested))
{
    $script_with_parameter = "$($powershell_cmd) $($PSScriptRoot)\run-gcperfsim.ps1 1"
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

switch($arch)
{
    "x86"     { $arch_part = "x86"  }
    "x64"     { $arch_part = "x64"  }
    Default   { $arch_part = "x64"  }
}

$script_path         = path "$($gcperfsim_path)"
$corelab_binary_path = path "$($PSScriptRoot)\..\artifacts\tests\coreclr\$($os).$($arch_part).$($mode_part)\Tests\Core_Root\corerun $($gcperfsim_path)\GCPerfSim.dll $($gcperfsim_args)"

pushd $script_path
run $corelab_binary_path $False
Write-Output $LastExitCode
popd
