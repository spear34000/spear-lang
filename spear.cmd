@echo off
setlocal

if exist "%~dp0build\spear.exe" (
    "%~dp0build\spear.exe" %*
    exit /b %errorlevel%
)

if exist "%LOCALAPPDATA%\Programs\Spear\bin\spear.exe" (
    "%LOCALAPPDATA%\Programs\Spear\bin\spear.exe" %*
    exit /b %errorlevel%
)

echo spear error: could not find a built or installed spear.exe
exit /b 1
