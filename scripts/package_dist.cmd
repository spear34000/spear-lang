@echo off
setlocal

if exist build\dist rmdir /S /Q build\dist
mkdir build\dist
mkdir build\dist\runtime
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

xcopy /E /I /Y std build\dist\std >NUL
xcopy /E /I /Y examples build\dist\examples >NUL

xcopy /E /I /Y vscode-spear build\dist\vscode-spear >NUL
del /Q build\dist\vscode-spear\*.vsix 2>NUL

exit /b 0
