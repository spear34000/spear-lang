@echo off
setlocal

where cargo >NUL 2>NUL
if errorlevel 1 (
  echo cargo was not found. Install Rust and Cargo first.
  exit /b 1
)

cargo build --release -p sharp || exit /b 1

if not exist build mkdir build
copy /Y target\release\sharp.exe build\sharp-rs.exe >NUL || exit /b 1

echo Built:
echo   build\sharp-rs.exe
exit /b 0
