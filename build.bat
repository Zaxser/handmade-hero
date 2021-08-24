"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall" x64 & powershell
mkdir build
pushd build
pwd
cl code\win32_handmade.cpp
popd