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

switch($os)
{
    "windows"
        {
            $prefix = "cmd /c '`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=$($arch_part_2) -host_arch=$($arch_part_2) & "
            $postfix = "'"
            $target = "clrgc.dll"
            $filename = "clrgc.dll"
        }
    "linux"
        {
            $prefix = ""
            $postfix = ""
            $target = "clrgc"
            $filename = "libclrgc.so"
        }
    "osx"
        {
            $prefix = ""
            $postfix = ""
            $target = "clrgc"
            $filename = "libclrgc.dylib"
        }
}

$build_command = $prefix + (path "cmake --build artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part) --target $($target) --config $($mode_part) $($postfix)")
$src           = path "artifacts\obj\coreclr\$($os).$($arch_part).$($mode_part)\gc\$($filename)"
$dst           = path "artifacts\bin\coreclr\$($os).$($arch_part).$($mode_part)\"

pushd $repo_root
Invoke-Expression $build_command
copy $src $dst -force
popd
