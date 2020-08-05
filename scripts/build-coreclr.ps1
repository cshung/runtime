$mode = $env:andrew_mode
$arch = $env:andrew_arch

switch($mode)
{
    "debug"   { $mode_part = ""           }
    "checked" { $mode_part = "-c checked" }
    "release" { $mode_part = "-c release" }
    Default   { $mode_part = ""           }
}

switch($arch)
{
    "x86"     { $arch_part = "-a x86"  }
    "x64"     { $arch_part = ""        }
    Default   { $arch_part = ""        }
}

if ($IsWindows -or $ENV:OS)
{
    $script_part = "build.cmd"
}
else
{
    $script_part = "build.sh"
}

$script_path = Join-Path $PSScriptRoot (Join-Path ".." $script_part)
$build_command = $script_path + " -s clr " + $mode_part + " " + $arch_part
# Write-Output $build_command
Invoke-Expression $build_command