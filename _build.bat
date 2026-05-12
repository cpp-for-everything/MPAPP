@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cd /d %~dp0
if exist build rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMPAPP_BUILD_TOOLS=ON
if errorlevel 1 exit /b 1
cmake --build build
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure
