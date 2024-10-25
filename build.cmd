@echo off

echo compile...
cmkr gen
cmake -B build32 -G "Visual Studio 17 2022" -A win32
cmake --build build32 --config Release
cmake -B build64 -G "Visual Studio 17 2022" -A x64
cmake --build build64 --config Release
