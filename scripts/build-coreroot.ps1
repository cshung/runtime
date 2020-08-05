. (Join-Path $PSScriptRoot "common.ps1")

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

$script_path   = path "$($PSScriptRoot)\..\src\tests"
$build_command = "./build$($script_suffix) generatelayoutonly $($mode_part) $($arch_part)"

pushd $script_path
Invoke-Expression $build_command
popd
