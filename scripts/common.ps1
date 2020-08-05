$mode = $env:andrew_mode
$arch = $env:andrew_arch
$repo_root = Join-Path $PSScriptRoot ".."

if ($IsWindows -or $ENV:OS)
{
    $os = "windows"
    $sep = "\"
    $script_suffix = ".cmd"
    $rid_os_part = "win"
    $powershell_cmd = "powershell"
}
else
{
    $sep = "/"
    $script_suffix = ".sh"
    $powershell_cmd = "pwsh"
    if ($IsLinux)
    {
        $os = "linux"
        $rid_os_part = "linux"
    }
    else
    {
        $os = "osx"
        $rid_os_part = "osx"
    }
}

switch($arch)
{
    "x86"     { $rid = $rid_os_part + "-x86"  }
    "x64"     { $rid = $rid_os_part + "-x64"  }
    Default   { $rid = $rid_os_part + "-x64"  }
}

function path([string] $s)
{
    return $s.replace("\", $sep)
}