@echo off
setlocal

if exist "%~dp0build\sharp.exe" (
    "%~dp0build\sharp.exe" %*
    exit /b %errorlevel%
)

if exist "%LOCALAPPDATA%\Programs\Sharp\bin\sharp.exe" (
    "%LOCALAPPDATA%\Programs\Sharp\bin\sharp.exe" %*
    exit /b %errorlevel%
)

if exist "%~dp0build\spear.exe" (
    "%~dp0build\spear.exe" %*
    exit /b %errorlevel%
)

if exist "%LOCALAPPDATA%\Programs\Spear\bin\spear.exe" (
    "%LOCALAPPDATA%\Programs\Spear\bin\spear.exe" %*
    exit /b %errorlevel%
)

echo sharp error: could not find a built or installed sharp.exe
exit /b 1
