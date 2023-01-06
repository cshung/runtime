# Running this script by itself is meaningless, as the alias defined are going away after the script is executed.
# In order to make these alias available in the prompt, we need to run this using dot sourcing
# . ./init.ps1

# p.cmd/p.sh make this easy, there is no need to run this script directly

# PowerShell alias do not allow invoking another script with given parameters, the recommendation is to wrap the use
# of the script as a function and provide parameter values there.

# Using $PSScriptRoot, we can make these alias runnable anywhere without hard coding paths

function Set-Mode-Debug     { Invoke-Expression ((Join-Path $PSScriptRoot "set-mode.ps1"           ) + " debug"   ) }
function Set-Mode-Checked   { Invoke-Expression ((Join-Path $PSScriptRoot "set-mode.ps1"           ) + " checked" ) }
function Set-Mode-Release   { Invoke-Expression ((Join-Path $PSScriptRoot "set-mode.ps1"           ) + " release" ) }

function Set-Arch-x86       { Invoke-Expression ((Join-Path $PSScriptRoot "set-arch.ps1"           ) + " x86"     ) }
function Set-Arch-x64       { Invoke-Expression ((Join-Path $PSScriptRoot "set-arch.ps1"           ) + " x64"     ) }

function Build-CoreClr      { Invoke-Expression ((Join-Path $PSScriptRoot "build-coreclr.ps1"      ) + ""         ) }
function Build-ClrGc        { Invoke-Expression ((Join-Path $PSScriptRoot "build-clrgc.ps1"        ) + ""         ) }
function Build-Library      { Invoke-Expression ((Join-Path $PSScriptRoot "build-library.ps1"      ) + ""         ) }
function Build-CoreRoot     { Invoke-Expression ((Join-Path $PSScriptRoot "build-coreroot.ps1"     ) + ""         ) }

function Build-CoreLab      { Invoke-Expression ((Join-Path $PSScriptRoot "build-corelab.ps1"      ) + ""         ) }
function Run-CoreLab        { Invoke-Expression ((Join-Path $PSScriptRoot "run-corelab.ps1"        ) + ""         ) }
function Debug-CoreLab      { Invoke-Expression ((Join-Path $PSScriptRoot "debug-corelab.ps1"      ) + ""         ) }

function Build-GCSample     { Invoke-Expression ((Join-Path $PSScriptRoot "build-gcsample.ps1"     ) + ""         ) }
function Run-GCSample       { Invoke-Expression ((Join-Path $PSScriptRoot "run-gcsample.ps1"       ) + ""         ) }
function Debug-GCSample     { Invoke-Expression ((Join-Path $PSScriptRoot "debug-gcsample.ps1"     ) + ""         ) }

function Build-Test         { Invoke-Expression ((Join-Path $PSScriptRoot "build-test.ps1"         ) + ""         ) }
function Run-Test           { Invoke-Expression ((Join-Path $PSScriptRoot "run-test.ps1"           ) + ""         ) }
function Debug-Test         { Invoke-Expression ((Join-Path $PSScriptRoot "debug-test.ps1"         ) + ""         ) }

function Enable-Stress-Log  { Invoke-Expression ((Join-Path $PSScriptRoot "enable-stress-log.ps1"  ) + ""         ) }
function Disable-Stress-Log { Invoke-Expression ((Join-Path $PSScriptRoot "disable-stress-log.ps1" ) + ""         ) }
function Enable-ClrGc       { Invoke-Expression ((Join-Path $PSScriptRoot "enable-clrgc.ps1"       ) + ""         ) }
function Disable-ClrGc      { Invoke-Expression ((Join-Path $PSScriptRoot "disable-clrgc.ps1"      ) + ""         ) }
function Enable-Auto-Trace  { Invoke-Expression ((Join-Path $PSScriptRoot "enable-auto-trace.ps1"  ) + ""         ) }
function Disable-Auto-Trace { Invoke-Expression ((Join-Path $PSScriptRoot "disable-auto-trace.ps1" ) + ""         ) }
function Enable-Server      { Invoke-Expression ((Join-Path $PSScriptRoot "enable-server.ps1"      ) + ""         ) }
function Disable-Server     { Invoke-Expression ((Join-Path $PSScriptRoot "disable-server.ps1"     ) + ""         ) }

# Set the current build flavor to debug/checked/release
Set-Alias -Name smd  Set-Mode-Debug
Set-Alias -Name smc  Set-Mode-Checked
Set-Alias -Name smr  Set-Mode-Release
                              
# Set the current architecture to x86/x64
Set-Alias -Name a86  Set-Arch-x86
Set-Alias -Name a64  Set-Arch-x64
                              
# Build CoreClr/ClrGc/Library/CoreRoot
Set-Alias -Name bc   Build-CoreClr
Set-Alias -Name bcg  Build-ClrGc
Set-Alias -Name bl   Build-Library
Set-Alias -Name bcr  Build-CoreRoot
 
Set-Alias -Name bcl  Build-CoreLab
Set-Alias -Name rcl  Run-CoreLab
Set-Alias -Name dcl  Debug-CoreLab
                               
Set-Alias -Name bgs  Build-GCSample
Set-Alias -Name rgs  Run-GCSample
Set-Alias -Name dgs  Debug-GCSample
                               
Set-Alias -Name bt   Build-Test
Set-Alias -Name rt   Run-Test
Set-Alias -Name dt   Debug-Test
                              
Set-Alias -Name esl  Enable-Stress-Log
Set-Alias -Name dsl  Disable-Stress-Log
Set-Alias -Name ecg  Enable-ClrGc
Set-Alias -Name dcg  Disable-ClrGc
Set-Alias -Name eat  Enable-Auto-Trace
Set-Alias -Name dat  Disable-Auto-Trace
Set-Alias -Name svr  Enable-Server
Set-Alias -Name wks  Disable-Server

function alert { param ($msg)  Invoke-Expression ((Join-Path $PSScriptRoot "alert.ps1"              ) + " " + $msg  ) }
function pcr   { param ($name) Invoke-Expression ((Join-Path $PSScriptRoot "publish-coreroot.ps1"   ) + " " + $name ) }
function infra { param ($name) Invoke-Expression ((Join-Path $PSScriptRoot "run-infra.ps1"          ) + " " + $name ) }