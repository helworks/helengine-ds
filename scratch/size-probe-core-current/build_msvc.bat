@echo off
setlocal
set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build\msvc
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cl /nologo /std:c++20 /EHsc /I"%SCRIPT_DIR%" /I"%SCRIPT_DIR%runtime" /c "%SCRIPT_DIR%generated_unity.cpp" /Fo"%BUILD_DIR%\generated_unity.obj"
