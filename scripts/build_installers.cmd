@echo off
setlocal

if not exist build mkdir build
if not exist dist mkdir dist
if not exist dist\installers mkdir dist\installers

gcc -O2 -Wall -Wextra -std=c11 -o build\spearc.exe src\spearc.c || exit /b 1
gcc -O2 -Wall -Wextra -std=c11 -o build\spear.exe src\spear_cli.c || exit /b 1
gcc -O2 -Wall -Wextra -std=c11 -mwindows -o build\spear-setup.exe src\spear_setup.c -lcomctl32 || exit /b 1

call scripts\package_dist.cmd || exit /b 1

if not exist "%USERPROFILE%\.dotnet\tools\wix.exe" (
  echo WiX tool was not found at %USERPROFILE%\.dotnet\tools\wix.exe
  exit /b 1
)

"%USERPROFILE%\.dotnet\tools\wix.exe" build installer\SpearInstaller.wxs -d SourceDir="%CD%\build\dist" -o build\SpearSetup.msi || exit /b 1

copy /Y build\spear-setup.exe dist\installers\SpearSetup.exe >NUL || exit /b 1
copy /Y build\SpearSetup.msi dist\installers\SpearSetup.msi >NUL || exit /b 1

echo Built:
echo   dist\installers\SpearSetup.exe
echo   dist\installers\SpearSetup.msi
exit /b 0
