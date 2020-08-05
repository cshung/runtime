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
    "x86"     { $arch_part = "x86"; $arch_part_2 = "x86"   }
    "x64"     { $arch_part = "x64"; $arch_part_2 = "amd64" }
    Default   { $arch_part = "x64"; $arch_part_2 = "amd64" }
}

if ($IsWindows -or $ENV:OS)
{
    $os_part = "windows"
}
else
{
    
    if ($IsLinux)
    {
        $os_part = "linux"
    }
    else
    {
        $os_part = "osx"
    }
}

switch($os_part)
{
    "windows"
        {
            $prefix = "cmd /c '`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`" -arch=$($arch_part_2) -host_arch=$($arch_part_2) & "
            $postfix = "'"
            $sep = "\"
            $target = "clrgc.dll"
            $filename = "clrgc.dll"
        }
    "linux"
        {
            $prefix = ""
            $postfix = ""
            $sep = "/"
            $target = "clrgc"
            $filename = "libclrgc.so"
        }
    "osx"
        {
            $prefix = ""
            $postfix = ""
            $sep = "/"
            $target = "clrgc"
            $filename = "libclrgc.dylib"
        }
}

$repo_root = Join-Path $PSScriptRoot ".."

$cmd = $prefix + "cmake --build artifacts\obj\coreclr\$($os_part).$($arch_part).$($mode_part) --target $($target) --config $($mode_part)" + $postfix
$src = "artifacts/obj/coreclr/$($os_part).$($arch_part).$($mode_part)/gc/$($filename)".replace("/", $sep)
$dst = "artifacts/bin/coreclr/$($os_part).$($arch_part).$($mode_part)/".replace("/", $sep)

pushd $repo_root
Invoke-Expression $cmd
copy $src $dst -force
popd
