. (Join-Path $PSScriptRoot "common.ps1")

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

$build_command = ".\build$($script_suffix) -s clr $($mode_part) $($arch_part)"

pushd $repo_root
Invoke-Expression $build_command
popd
