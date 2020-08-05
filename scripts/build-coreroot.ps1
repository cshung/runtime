$mode = $env:andrew_mode
$arch = $env:andrew_arch

switch($mode)
{
    "debug"   { $mode_part = ""        }
    "checked" { $mode_part = "checked" }
    "release" { $mode_part = "release" }
    Default   { $mode_part = ""        }
}

switch($arch)
{
    "x86"     { $arch_part = "x86"  }
    "x64"     { $arch_part = ""     }
    Default   { $arch_part = ""     }
}

if ($IsWindows -or $ENV:OS)
{
    $script_part = "build.cmd"
}
else
{
    $script_part = "build.sh"
}

$script_path = Join-Path $PSScriptRoot (Join-Path ".." (Join-Path "src" (Join-Path "tests" $script_part)))
$build_command = $script_path + " generatelayoutonly " + $mode_part + " " + $arch_part
# Write-Output $build_command
Invoke-Expression $build_command