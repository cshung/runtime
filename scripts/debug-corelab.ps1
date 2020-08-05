Param (
    [string]$nested
)

if ($IsWindows -or $ENV:OS)
{
    $rid_os_part = "win"
    $powershell_cmd = "powershell"
    $dotnet_script = "dotnet.cmd"
}
else
{
    $dotnet_script = "dotnet.sh"
    $powershell_cmd = "pwsh"
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

# Feel free to create environment variable here, it is scoped within this file

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

#
# To make sure the cleanup command execute after the process launch, we use the | Out-Null trick as described here
# https://stackoverflow.com/questions/1741490/how-to-tell-powershell-to-wait-for-each-command-to-end-before-starting-the-next
#
$debugger_command = "c:\toolssw\debuggers\amd64\windbg.exe -c ``$``$>a<autodbg.script " + $corelab_binary_path + " | Out-Null"

pushd $script_path
Invoke-Expression $env_path
Invoke-Expression $setup_path
Invoke-Expression $debugger_command
Invoke-Expression $cleanup_path
popd
