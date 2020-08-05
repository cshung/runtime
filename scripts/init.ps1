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
function Build-Library      { Invoke-Expression ((Join-Path $PSScriptRoot "build-library.ps1"      ) + ""         ) }
function Build-CoreRoot     { Invoke-Expression ((Join-Path $PSScriptRoot "build-coreroot.ps1"     ) + ""         ) }

function Build-CoreLab      { Invoke-Expression ((Join-Path $PSScriptRoot "build-corelab.ps1"      ) + ""         ) }
function Run-CoreLab        { Invoke-Expression ((Join-Path $PSScriptRoot "run-corelab.ps1"        ) + ""         ) }
function Debug-CoreLab      { Invoke-Expression ((Join-Path $PSScriptRoot "debug-corelab.ps1"      ) + ""         ) }

function Enable-Stress-Log  { Invoke-Expression ((Join-Path $PSScriptRoot "enable-stress-log.ps1"  ) + ""         ) }
function Disable-Stress-Log { Invoke-Expression ((Join-Path $PSScriptRoot "disable-stress-log.ps1" ) + ""         ) }

# Set the current build flavor to debug/checked/release
Set-Alias -Name smd Set-Mode-Debug
Set-Alias -Name smc Set-Mode-Checked
Set-Alias -Name smr Set-Mode-Release

# Set the current architecture to x86/x64
Set-Alias -Name a86 Set-Arch-x86
Set-Alias -Name a64 Set-Arch-x64

# Build CoreClr/Library/CoreRoot
Set-Alias -Name bc  Build-CoreClr
Set-Alias -Name bl  Build-Library
Set-Alias -Name bcr Build-CoreRoot
Set-Alias -Name bcl Build-CoreLab

Set-Alias -Name rcl Run-CoreLab
Set-Alias -Name dcl Debug-CoreLab

Set-Alias -Name esl Enable-Stress-Log
Set-Alias -Name dsl Disable-Stress-Log
