@echo off
rem This is just the path of vcvarsall on my machine. Your mileage may vary.
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall" x64

rem %~dp0 resolves to the path of build.bat; this makes it so that build.bat can
rem be called from anywhere and still work.
rmdir /Q /S "%~dp0..\build\"
mkdir "%~dp0..\build"
pushd "%~dp0..\build"
echo * > .gitignore
cl -FC -Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib
popd

call "%~dp0..\build\win32_handmade.exe"