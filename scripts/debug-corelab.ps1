Param (
    [string]$nested
)

$arch = $env:andrew_arch

switch($arch)
{
    "x86"     { $arch_part = "x86"   }
    "x64"     { $arch_part = "amd64" }
    Default   { $arch_part = "amd64" }
}

if ($IsWindows -or $ENV:OS)
{
    $rid_os_part = "win"
    $powershell_cmd = "powershell"
    $dotnet_script = "dotnet.cmd"
    $debugger_path = "c:\toolssw\debuggers\$($arch_part)\windbg.exe -c ``$``$>a<autodbg.script"
    #
    # To make sure the cleanup command execute after the process launch, we use the | Out-Null trick as described here
    # https://stackoverflow.com/questions/1741490/how-to-tell-powershell-to-wait-for-each-command-to-end-before-starting-the-next
    #
    $debugger_postfix = "| Out-Null"
}
else
{
    $powershell_cmd = "pwsh"
    $dotnet_script = "dotnet.sh"
    $debugger_path = "lldb"
    $debugger_postfix = ""
    #
    # To make sure lldbinit works, make sure we have this line in ~/.lldbinit
    # settings set target.load-cwd-lldbinit true
    #
    if ($IsLinux)
    {
        $rid_os_part = "linux"
    }
    else
    {
        $rid_os_part = "osx"
    }
}

if (-not($nested))
{
    $script_with_parameter = $powershell_cmd + " " + ((Join-Path $PSScriptRoot "debug-corelab.ps1") + " 1")
    Invoke-Expression $script_with_parameter
    Exit
}

$mode = $env:andrew_mode
$arch = $env:andrew_arch

switch($mode)
{
    "debug"   { $mode_part = "Debug"   }
    "checked" { $mode_part = "Checked" }
    "release" { $mode_part = "Release" }
    Default   { $mode_part = "Debug"   }
}

switch($arch)
{
    "x86"     { $arch_part = "x86"  }
    "x64"     { $arch_part = "x64"  }
    Default   { $arch_part = "x64"  }
}

$setup_path   = Join-Path $PSScriptRoot "lab-setup.ps1"
$env_path     = Join-Path $PSScriptRoot "lab-env.ps1"
$cleanup_path = Join-Path $PSScriptRoot "lab-cleanup.ps1"
$script_path  = Join-Path $PSScriptRoot (Join-Path ".." "CoreLab")

$rid = $rid_os_part + "-" + $arch_part

$corelab_binary_path =
    Join-Path $PSScriptRoot (
        Join-Path ".." (
            Join-Path "artifacts" (
                Join-Path "bin" (
                    Join-Path "CoreLab" (
                        Join-Path $mode_part (
                            Join-Path "net7.0" (
                                Join-Path $rid (
                                    Join-Path "publish" "CoreLab"
                                )
                            )
                        )
                    )
                )
            )
        )
    )

$debugger_command = $debugger_path + " "  + $corelab_binary_path + " " + $debugger_postfix

pushd $script_path
Invoke-Expression $env_path
Invoke-Expression $setup_path
Invoke-Expression $debugger_command
Invoke-Expression $cleanup_path
popd
