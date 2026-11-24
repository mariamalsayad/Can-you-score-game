@echo off
cd /d "%~dp0"
cmake -S . -B build
if errorlevel 1 goto fail
cmake --build build --config Release --parallel 4
if errorlevel 1 goto fail
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 goto fail
if exist build\Release\can_you_score.exe (build\Release\can_you_score.exe) else (build\can_you_score.exe)
exit /b
:fail
 echo Install CMake and Visual Studio Build Tools with Desktop development with C++.
 pause
