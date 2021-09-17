pushd src\tests\profiler\native
rd /s/q build
mkdir build
cd build
cmake ..
msbuild ALL_BUILD.vcxproj
popd
del /q src\coreclr\debug\CompilerIdC.exe.recipe
del /q src\coreclr\debug\CompilerIdCXX.exe.recipe
