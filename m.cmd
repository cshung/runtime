@echo off
setlocal
if "%ANDREW_OPTIMIZED%"=="1" (
  set ANDREW_ARCH_PART=artifacts\obj\coreclr\Windows.x64.Release\vm\wks\cee_wks_core.vcxproj
) else (
  set ANDREW_ARCH_PART=artifacts\obj\coreclr\Windows.x64.Debug\vm\wks\cee_wks_core.vcxproj
)
msbuild %ANDREW_ARCH_PART%
endlocal

