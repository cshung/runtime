dotnet publish -r win-x64 --self-contained
xcopy /s/y c:\Dev\runtime\artifacts\bin\coreclr\Windows_NT.x64.Debug\* C:\Dev\runtime\artifacts\bin\Strings\Debug\netcoreapp3.1\win-x64\publish\
