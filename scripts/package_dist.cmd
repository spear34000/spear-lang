@echo off
setlocal

if exist build\dist rmdir /S /Q build\dist
mkdir build\dist
mkdir build\dist\runtime
mkdir build\dist\toolchain
mkdir build\dist\toolchain\mingw64
mkdir build\dist\vscode-spear
mkdir build\dist\vscode-spear\assets
mkdir build\dist\vscode-spear\snippets
mkdir build\dist\vscode-spear\syntaxes
mkdir build\dist\vscode-spear\themes

copy /Y build\spear.exe build\dist\spear.exe >NUL
copy /Y build\spearc.exe build\dist\spearc.exe >NUL
copy /Y build\spear-setup.exe build\dist\spear-setup.exe >NUL

copy /Y runtime\bridge_node.mjs build\dist\runtime\bridge_node.mjs >NUL
copy /Y runtime\bridge_python.py build\dist\runtime\bridge_python.py >NUL
copy /Y runtime\demo_node.cjs build\dist\runtime\demo_node.cjs >NUL
copy /Y runtime\demo_python.py build\dist\runtime\demo_python.py >NUL
copy /Y runtime\serve_static.ps1 build\dist\runtime\serve_static.ps1 >NUL

xcopy /E /I /Y std build\dist\std >NUL
xcopy /E /I /Y examples build\dist\examples >NUL

xcopy /E /I /Y vscode-spear build\dist\vscode-spear >NUL
del /Q build\dist\vscode-spear\*.vsix 2>NUL

if defined MINGW_ROOT (
  set "MINGW_SRC=%MINGW_ROOT%"
) else if exist C:\mingw64 (
  set "MINGW_SRC=C:\mingw64"
) else (
  echo MinGW toolchain was not found. Set MINGW_ROOT or install C:\mingw64.
  exit /b 1
)

xcopy /E /I /Y "%MINGW_SRC%\bin" build\dist\toolchain\mingw64\bin >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\lib" build\dist\toolchain\mingw64\lib >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\libexec" build\dist\toolchain\mingw64\libexec >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\x86_64-w64-mingw32" build\dist\toolchain\mingw64\x86_64-w64-mingw32 >NUL || exit /b 1
if exist "%MINGW_SRC%\mingwvars.bat" copy /Y "%MINGW_SRC%\mingwvars.bat" build\dist\toolchain\mingw64\mingwvars.bat >NUL
if exist "%MINGW_SRC%\version_info.txt" copy /Y "%MINGW_SRC%\version_info.txt" build\dist\toolchain\mingw64\version_info.txt >NUL

exit /b 0
