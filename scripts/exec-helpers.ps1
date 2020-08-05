function run([string] $command, [bool] $debug)
{
    switch($arch)
    {
        "x86"     { $arch_part_2 = "x86"    }
        "x64"     { $arch_part_2 = "amd64"  }
        Default   { $arch_part_2 = "amd64"  }
    }

    $setup_path          = path "$($PSScriptRoot)\lab-setup.ps1"
    $env_path            = path "$($PSScriptRoot)\lab-env.ps1"
    $cleanup_path        = path "$($PSScriptRoot)\lab-cleanup.ps1"
    

    if ($debug)
    {
        if ($IsWindows -or $ENV:OS)
        {
            $debugger_path = "c:\toolssw\debuggers\$($arch_part_2)\windbg.exe -c ``$``$>a<autodbg.script"
            #
            # To make sure the cleanup command execute after the process launch, we use the | Out-Null trick as described here
            # https://stackoverflow.com/questions/1741490/how-to-tell-powershell-to-wait-for-each-command-to-end-before-starting-the-next
            #
            $debugger_postfix = "| Out-Null"
        }
        else
        {
            $debugger_path = "lldb"
            $debugger_postfix = ""
            #
            # To make sure lldbinit works, make sure we have this line in ~/.lldbinit
            # settings set target.load-cwd-lldbinit true
            #
        }
        $command = "$($debugger_path) $($command) $($debugger_postfix)"
    }

    Invoke-Expression $env_path
    Invoke-Expression $setup_path
    Invoke-Expression $command
    Invoke-Expression $cleanup_path
}