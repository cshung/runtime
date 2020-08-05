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
    "x86"     { $arch_part = "x86"; $arch_part_2 = "x86"   }
    "x64"     { $arch_part = "x64"; $arch_part_2 = "amd64" }
    Default   { $arch_part = "x64"; $arch_part_2 = "amd64" }
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
            $filename = "$($clrgc_favor).dll"
        }
    "linux"
        {
            $prefix = ""
            $postfix = ""
            $target = "$($clrgc_favor)"
            $filename = "lib$($clrgc_favor).so"
        }
    "osx"
        {
            $prefix = ""
            $postfix = ""
            $target = "$($clrgc_favor)"
            $filename = "lib$($clrgc_favor).dylib"
        }
}

$build_command = $prefix + (path "cmake --build artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part) --target $($target) --config $($mode_part) $($postfix)")
$src           = path "artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part)\gc\$($filename)"
$dst           = path "artifacts\bin\coreclr\$($os).$($arch_part).$($mode_part)\"

pushd $repo_root
Invoke-Expression $build_command
copy $src $dst -force
popd
