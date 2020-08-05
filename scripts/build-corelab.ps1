. (Join-Path $PSScriptRoot "common.ps1")

switch($mode)
{
    "debug"   { $build_mode_part = ""           ; $bin_mode_part = "Debug"   }
    "checked" { $build_mode_part = "-c Checked" ; $bin_mode_part = "Checked" }
    "release" { $build_mode_part = "-c release" ; $bin_mode_part = "Release" }
    Default   { $build_mode_part = ""           ; $bin_mode_part = "Debug"   }
}

switch($arch)
{
    "x86"     { $arch_part = "x86"  }
    "x64"     { $arch_part = "x64"  }
    Default   { $arch_part = "x64"  }
}

$script_path         = path "$($PSScriptRoot)\..\CoreLab"
$build_command       = path "$($repo_root)\dotnet$($script_suffix) publish --self-contained -r $($rid) $($build_mode_part)"
$coreclr_binary_path = path "$($PSScriptRoot)\..\artifacts\bin\coreclr\$($os).$($arch_part).$($bin_mode_part)\*"
$corelab_binary_path = path "$($PSScriptRoot)\..\artifacts\bin\CoreLab\$($bin_mode_part)\net7.0\$($rid)\publish"

pushd $script_path
Write-Output $build_command
Invoke-Expression $build_command
copy $coreclr_binary_path $corelab_binary_path -recurse -force
popd
