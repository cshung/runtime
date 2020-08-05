Param (
    [string]$name
)

. (Join-Path $PSScriptRoot "common.ps1")

if (-not($name))
{
    Write-Output "The name parameter is required"
    Exit
}

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

$base_path           = path "$($PSScriptRoot)\..\.."
$repo_path           = path "$($PSScriptRoot)\..\..\GC.Analysis.API"
$configuration_path  = path "$($PSScriptRoot)\..\..\GC.Analysis.API\ExampleConfigurations"
$template_path       = path "$($PSScriptRoot)\..\..\GC.Analysis.API\ExampleConfigurations\Template.yaml"
$output_path         = path "$($PSScriptRoot)\..\..\GC.Analysis.API\ExampleConfigurations\$($name).yaml"
$script_path         = path "$($PSScriptRoot)\..\..\GC.Analysis.API\GC.Infrastructure\"


pushd $script_path
$template = Get-Content $template_path
$template = $template.replace("==Name==", $name)
$template = $template.replace("==Base==", $base_path.replace("\", "\\"))
$template = $template.replace("==Flavor==", "$($os).$($arch_part).$($mode_part)")
Set-Content $output_path $template
Invoke-Expression "dotnet run -- run --configuration $($output_path)"
popd
