$mode = $env:andrew_mode
$arch = $env:andrew_arch
$repo_root = Join-Path $PSScriptRoot ".."

$devbox = [System.Net.Dns]::GetHostName().StartsWith("CPC-andre-")

if ($IsWindows -or $ENV:OS)
{
    $os = "windows"
    $sep = "\"
    $script_suffix = ".cmd"
    $rid_os_part = "win"
    $powershell_cmd = "powershell"
    $alert = "c:\toolssw\slack\alert"
    $gcperfsim_path = "C:\Dev\performance\artifacts\bin\GCPerfSim\Release\net7.0"
    if ($devbox)
    {
        $logPath="C:\Logs\"
    }
    else
    {
        $logPath="D:\"
    }
}
else
{
    $sep = "/"
    $script_suffix = ".sh"
    $powershell_cmd = "pwsh"
    $alert = "python3 ~/toolssw/alert.py"
    $gcperfsim_path = "/home/andrewau/git/performance/artifacts/bin/GCPerfSim/Release/net7.0"
    if ($IsLinux)
    {
        $os = "linux"
        $rid_os_part = "linux"
        $logPath="/home/andrewau/"
    }
    else
    {
        $os = "osx"
        $rid_os_part = "osx"
        $logPath="/Users/andrewau/"
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

# Configuration for build/run/debug test
$test_source_path = "baseservices\finalization"
$test_proj_name   = "CriticalFinalizer"

# Configuration for GCPerfSim
$gcperfsim_args = "-tc 6 -tagb 100.0 -tlgb 0.01 -lohar 0 -pohar 0 -sohsi 10 -lohsi 0 -pohsi 0 -sohsr 100-4000 -lohsr 102400-204800 -pohsr 100-4000 -sohpi 10 -lohpi 0 -sohfi 0 -lohfi 0 -pohfi 0 -allocType reference -testKind time"

if ([System.Net.Dns]::GetHostName() -eq "ANDREWAU-LHS20")
{
    $vspath = "C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\Tools\VsDevCmd.bat"
}
else 
{
    if ($devbox)
    {
        $vspath = "C:\Program Files\Microsoft Visual Studio\2022\IntPreview\Common7\Tools\VsDevCmd.bat"
    }
    else
    {
        $vspath = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    }
}