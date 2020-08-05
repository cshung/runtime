. (Join-Path $PSScriptRoot "common.ps1")

switch($mode)
{
    "debug"   { $build_mode_part = ""           ; }
    "checked" { $build_mode_part = "-c Checked" ; }
    "release" { $build_mode_part = "-c release" ; }
    Default   { $build_mode_part = ""           ; }
}

switch($arch)
{
    "x86"     { $arch_part = "x86"  }
    "x64"     { $arch_part = "x64"  }
    Default   { $arch_part = "x64"  }
}

$script_path         = path "$($PSScriptRoot)\..\src\tests\GC\API\NoGCRegion"
$build_command       = path "$($repo_root)\dotnet$($script_suffix) build /p:TargetArchitecture=$($arch_part) $($build_mode_part) Callback_Svr.csproj"

pushd $script_path
Write-Output $build_command
Invoke-Expression $build_command
popd
