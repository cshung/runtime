. (Join-Path $PSScriptRoot "common.ps1")

switch($mode)
{
    "debug"   { $mode_part = "Debug"   }
    "checked" { $mode_part = "Checked" }
    "release" { $mode_part = "Release" }
    Default   { $mode_part = "Debug"   }
}

switch($arch)
{
    "x86"     { $arch_part = "x86";   $arch_part_2 = "x86"   }
    "x64"     { $arch_part = "x64";   $arch_part_2 = "amd64" }
    "arm64"   { $arch_part = "arm64"; $arch_part_2 = "arm64" }
    Default   { $arch_part = "x64";   $arch_part_2 = "amd64" }
}

$exp = $env:andrew_exp

if ($exp -eq "0")
{
    $clrgc_favor="clrgc"
}
else
{
    $clrgc_favor="clrgcexp"
}

switch($os)
{
    "windows"
        {
            $prefix = "cmd /c '`"$($vspath)`" -arch=$($arch_part_2) -host_arch=$($arch_part_2) & "
            $postfix = "'"
            $target = "$($clrgc_favor).dll"
            $binary = "$($clrgc_favor).dll"
            $symbol = "$($clrgc_favor).pdb"
        }
    "linux"
        {
            $prefix = ""
            $postfix = ""
            $target = "$($clrgc_favor)"
            $binary = "lib$($clrgc_favor).so"
            $symbol = "lib$($clrgc_favor).so.dbg"
        }
    "osx"
        {
            $prefix = ""
            $postfix = ""
            $target = "$($clrgc_favor)"
            $binary = "lib$($clrgc_favor).dylib"
            $symbol = "lib$($clrgc_favor).dylib.dwarf"
        }
}

# The built symbol needs to go to the bin
# ./obj/coreclr/osx.x64.Debug/gc/libclrgcexp.dylib.dwarf

$build_command = $prefix + (path "cmake --build artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part) --target $($target) --config $($mode_part) $($postfix)")
$binary_src    = path "artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part)\gc\$($binary)"
$symbol_src    = path "artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part)\gc\$($symbol)"
$dst           = path "artifacts\bin\coreclr\$($os).$($arch_part).$($mode_part)\"

pushd $repo_root
$env:andrew_failed=""
Invoke-Expression $build_command
if ($LastExitCode -ne "0")
{
    $env:andrew_failed="1"
}
copy $binary_src $dst -force
copy $symbol_src $dst -force
popd
