@echo off
setlocal

if exist build\dist rmdir /S /Q build\dist
mkdir build\dist
mkdir build\dist\runtime
mkdir build\dist\vscode-spear
mkdir build\dist\vscode-spear\assets
mkdir build\dist\vscode-spear\snippets
mkdir build\dist\vscode-spear\syntaxes

copy /Y build\spear.exe build\dist\spear.exe >NUL
copy /Y build\spearc.exe build\dist\spearc.exe >NUL
copy /Y build\spear-setup.exe build\dist\spear-setup.exe >NUL

copy /Y runtime\bridge_node.mjs build\dist\runtime\bridge_node.mjs >NUL
copy /Y runtime\bridge_python.py build\dist\runtime\bridge_python.py >NUL
copy /Y runtime\demo_node.cjs build\dist\runtime\demo_node.cjs >NUL
copy /Y runtime\demo_python.py build\dist\runtime\demo_python.py >NUL

xcopy /E /I /Y examples build\dist\examples >NUL

copy /Y vscode-spear\extension.js build\dist\vscode-spear\extension.js >NUL
copy /Y vscode-spear\language-configuration.json build\dist\vscode-spear\language-configuration.json >NUL
copy /Y vscode-spear\package.json build\dist\vscode-spear\package.json >NUL
copy /Y vscode-spear\README.md build\dist\vscode-spear\README.md >NUL
copy /Y vscode-spear\assets\spear-logo.png build\dist\vscode-spear\assets\spear-logo.png >NUL
copy /Y vscode-spear\assets\spear-logo.svg build\dist\vscode-spear\assets\spear-logo.svg >NUL
copy /Y vscode-spear\snippets\spear.json build\dist\vscode-spear\snippets\spear.json >NUL
copy /Y vscode-spear\syntaxes\spear.tmLanguage.json build\dist\vscode-spear\syntaxes\spear.tmLanguage.json >NUL

exit /b 0
