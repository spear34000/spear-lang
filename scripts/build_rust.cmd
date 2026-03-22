@echo off
setlocal

if exist "%USERPROFILE%\.cargo\bin\cargo.exe" set "PATH=%USERPROFILE%\.cargo\bin;%PATH%"

where cargo >NUL 2>NUL
if errorlevel 1 (
  echo cargo was not found. Install Rust and Cargo first.
  exit /b 1
)

cargo build --release -p sharp -p sharpc || exit /b 1

if not exist build mkdir build
copy /Y target\release\sharp.exe build\sharp.exe >NUL || exit /b 1
copy /Y target\release\sharp.exe build\spear.exe >NUL || exit /b 1
copy /Y target\release\sharpc.exe build\sharpc.exe >NUL || exit /b 1
copy /Y target\release\sharpc.exe build\spearc.exe >NUL || exit /b 1

echo Built:
echo   build\sharp.exe
echo   build\sharpc.exe
exit /b 0
