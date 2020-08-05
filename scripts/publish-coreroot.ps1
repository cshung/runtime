Param (
    [string]$name
)

. (Join-Path $PSScriptRoot "common.ps1")

if (-not($name))
{
    Write-Output "The name parameter is required"
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

$script_path       = path "$($PSScriptRoot)\.."
# TODO - CrossPlatform, ever?
$coreroot_dst_path = path "D:\CoreRuns\$($name)\$($os).$($arch_part).$($mode_part)\Core_Root"
$coreroot_src_path = path "$($PSScriptRoot)\..\artifacts\tests\coreclr\$($os).$($arch_part).$($mode_part)\Tests\Core_Root"

pushd $script_path
if (Test-Path -LiteralPath $coreroot_dst_path) {
    rmdir $coreroot_dst_path -recurse
}
copy $coreroot_src_path $coreroot_dst_path -recurse -force
popd
