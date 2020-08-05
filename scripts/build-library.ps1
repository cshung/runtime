. (Join-Path $PSScriptRoot "common.ps1")

if ($env:andrew_failed -eq "1")
{
    Write-Output "$($(Split-Path $PSCommandPath -leaf)) skipped execution because previous build failed"
    return
}

switch($mode)
{
    "debug"   { $mode_part = ""                                }
    "checked" { $mode_part = "/p:RuntimeConfiguration=Checked" }
    "release" { $mode_part = "/p:RuntimeConfiguration=Release" }
    Default   { $mode_part = ""                                }
}

switch($arch)
{
    "x86"     { $arch_part = "-a x86"  }
    "x64"     { $arch_part = ""        }
    Default   { $arch_part = ""        }
}

$build_command = path ".\build$($script_suffix) -c release -s libs $($mode_part) $($arch_part)"

pushd $repo_root
Invoke-Expression $build_command
popd
