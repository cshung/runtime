$mode = $env:andrew_mode
$arch = $env:andrew_arch

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

$script_path = Join-Path $PSScriptRoot (Join-Path ".." "CoreLab")

if ($IsWindows -or $ENV:OS)
{
    $rid_os_part = "win"
    $bin_os_part = "Windows"
    $dotnet_script = "dotnet.cmd"
}
else
{
    $dotnet_script = "dotnet.sh"
    if ($IsLinux)
    {
        $rid_os_part = "linux"
        $bin_os_part = "linux"
    }
    else
    {
        $rid_os_part = "osx"
        $bin_os_part = "osx"
    }
}

$dotnet_path = Join-Path $PSScriptRoot (Join-Path ".." $dotnet_script)
$rid = $rid_os_part + "-" + $arch_part

$coreclr_binary_path =
    Join-Path $PSScriptRoot (
        Join-Path ".." (
            Join-Path "artifacts" (
                Join-Path "bin" (
                    Join-Path "coreclr" (
                        Join-Path ($bin_os_part + "." + $arch_part + "." + $bin_mode_part) "*"
                    )
                )
            )
        )
    )

$corelab_binary_path =
    Join-Path $PSScriptRoot (
        Join-Path ".." (
            Join-Path "artifacts" (
                Join-Path "bin" (
                    Join-Path "CoreLab" (
                        Join-Path $bin_mode_part (
                            Join-Path "net7.0" (
                                Join-Path $rid "publish"
                            )
                        )
                    )
                )
            )
        )
    )

$build_command = $dotnet_path + " publish --self-contained -r " + $rid + " " + $build_mode_part

pushd $script_path
# Write-Output $build_command
Invoke-Expression $build_command
copy $coreclr_binary_path $corelab_binary_path -recurse -force
popd
