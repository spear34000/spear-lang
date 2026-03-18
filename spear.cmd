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

if /I "%~1"=="serve" (
    if "%~2"=="" goto :usage
    set "mode=serve"
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
if /I "%mode%"=="serve" set "name=%~n2"

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

if /I "%mode%"=="serve" (
    "%exe_out%"
    if errorlevel 1 exit /b 1
    echo serving build at http://127.0.0.1:4173/
    if exist "build\spear-ui.html" (
        echo page: http://127.0.0.1:4173/spear-ui.html
    )
    python -m http.server 4173 --directory build
    exit /b %errorlevel%
)

"%exe_out%"
exit /b %errorlevel%

:usage
echo usage:
echo   spear file.sp
echo   spear build file.sp
echo   spear serve file.sp
exit /b 1
