Param (
    [string]$nested
)

. (Join-Path $PSScriptRoot "common.ps1")
. (Join-Path $PSScriptRoot "exec-helpers.ps1")

if (-not($nested))
{
    $script_with_parameter = "$($powershell_cmd) $($PSScriptRoot)\run-gcsample.ps1 1"
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

$script_path          = path "$($PSScriptRoot)\..\src\coreclr\gc\sample"
$gcsample_binary_path = path "$($PSScriptRoot)\..\artifacts\bin\coreclr\$($os).$($arch_part).$($mode_part)\gcsample.exe"

pushd $script_path
run $gcsample_binary_path $False
popd
