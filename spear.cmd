@echo off
setlocal

if "%~1"=="" goto :usage

set "mode=run"
set "input=%~1"

if /I "%~1"=="build" (
    if "%~2"=="" goto :usage
    set "mode=build"
    set "input=%~2"
)

if not exist "%input%" (
    echo input not found: %input%
    exit /b 1
)

if /I not "%input:~-3%"==".sp" (
    echo expected a .sp source file
    exit /b 1
)

if not exist "build" mkdir build

if not exist "build\spearc.exe" (
    gcc -O3 -Wall -Wextra -std=c11 -o build\spearc.exe src\spearc.c
    if errorlevel 1 exit /b 1
)

set "name=%~n1"
if /I "%mode%"=="build" set "name=%~n2"

set "c_out=build\%name%.c"
set "exe_out=build\%name%.exe"

build\spearc.exe "%input%" -o "%c_out%"
if errorlevel 1 exit /b 1

gcc -O3 -Wall -Wextra -std=c11 -o "%exe_out%" "%c_out%"
if errorlevel 1 exit /b 1

if /I "%mode%"=="build" (
    echo built %exe_out%
    exit /b 0
)

"%exe_out%"
exit /b %errorlevel%

:usage
echo usage:
echo   spear file.sp
echo   spear build file.sp
exit /b 1
