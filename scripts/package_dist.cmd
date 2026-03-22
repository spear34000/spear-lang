@echo off
setlocal

if exist build\dist rmdir /S /Q build\dist
mkdir build\dist
mkdir build\dist\runtime
mkdir build\dist\toolchain
mkdir build\dist\toolchain\mingw64
mkdir build\dist\toolchain\mingw64\bin
mkdir build\dist\toolchain\mingw64\lib
mkdir build\dist\toolchain\mingw64\lib\gcc
mkdir build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32
mkdir build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0
mkdir build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0\include
mkdir build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0\include-fixed
mkdir build\dist\toolchain\mingw64\lib\bfd-plugins
mkdir build\dist\toolchain\mingw64\libexec
mkdir build\dist\toolchain\mingw64\libexec\gcc
mkdir build\dist\toolchain\mingw64\libexec\gcc\x86_64-w64-mingw32
mkdir build\dist\toolchain\mingw64\libexec\gcc\x86_64-w64-mingw32\15.2.0
mkdir build\dist\toolchain\mingw64\x86_64-w64-mingw32
mkdir build\dist\toolchain\mingw64\x86_64-w64-mingw32\bin
mkdir build\dist\toolchain\mingw64\x86_64-w64-mingw32\include
mkdir build\dist\toolchain\mingw64\x86_64-w64-mingw32\lib
mkdir build\dist\vscode-spear
mkdir build\dist\vscode-spear\assets
mkdir build\dist\vscode-spear\snippets
mkdir build\dist\vscode-spear\syntaxes
mkdir build\dist\vscode-spear\themes

copy /Y build\sharp.exe build\dist\sharp.exe >NUL
copy /Y build\sharpc.exe build\dist\sharpc.exe >NUL
copy /Y build\sharp-setup.exe build\dist\sharp-setup.exe >NUL
copy /Y build\spear.exe build\dist\spear.exe >NUL
copy /Y build\spearc.exe build\dist\spearc.exe >NUL
copy /Y build\spear-setup.exe build\dist\spear-setup.exe >NUL

copy /Y runtime\bridge_python.py build\dist\runtime\bridge_python.py >NUL
copy /Y runtime\demo_python.py build\dist\runtime\demo_python.py >NUL
copy /Y runtime\bridge_node.mjs build\dist\runtime\bridge_node.mjs >NUL
copy /Y runtime\demo_node.cjs build\dist\runtime\demo_node.cjs >NUL
copy /Y runtime\serve_static.ps1 build\dist\runtime\serve_static.ps1 >NUL
if exist runtime\sqlite3.exe copy /Y runtime\sqlite3.exe build\dist\runtime\sqlite3.exe >NUL

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

for %%F in (gcc.exe as.exe ld.exe libiconv-2.dll libintl-8.dll libwinpthread-1.dll libgmp-10.dll libisl-23.dll libmpc-3.dll libmpfr-6.dll zlib1.dll libzstd.dll) do (
  copy /Y "%MINGW_SRC%\bin\%%F" build\dist\toolchain\mingw64\bin\%%F >NUL || exit /b 1
)
for %%F in (cc1.exe collect2.exe libgcc_s_seh-1.dll liblto_plugin.dll) do (
  copy /Y "%MINGW_SRC%\libexec\gcc\x86_64-w64-mingw32\15.2.0\%%F" build\dist\toolchain\mingw64\libexec\gcc\x86_64-w64-mingw32\15.2.0\%%F >NUL || exit /b 1
)
copy /Y "%MINGW_SRC%\libexec\gcc\x86_64-w64-mingw32\15.2.0\liblto_plugin.dll" build\dist\toolchain\mingw64\lib\bfd-plugins\liblto_plugin.dll >NUL || exit /b 1
for %%F in (crtbegin.o crtend.o libgcc.a libgcc_eh.a) do (
  copy /Y "%MINGW_SRC%\lib\gcc\x86_64-w64-mingw32\15.2.0\%%F" build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0\%%F >NUL || exit /b 1
)
xcopy /E /I /Y "%MINGW_SRC%\lib\gcc\x86_64-w64-mingw32\15.2.0\include" build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0\include >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\lib\gcc\x86_64-w64-mingw32\15.2.0\include-fixed" build\dist\toolchain\mingw64\lib\gcc\x86_64-w64-mingw32\15.2.0\include-fixed >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\x86_64-w64-mingw32\bin" build\dist\toolchain\mingw64\x86_64-w64-mingw32\bin >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\x86_64-w64-mingw32\include" build\dist\toolchain\mingw64\x86_64-w64-mingw32\include >NUL || exit /b 1
xcopy /E /I /Y "%MINGW_SRC%\x86_64-w64-mingw32\lib" build\dist\toolchain\mingw64\x86_64-w64-mingw32\lib >NUL || exit /b 1
if exist "%MINGW_SRC%\mingwvars.bat" copy /Y "%MINGW_SRC%\mingwvars.bat" build\dist\toolchain\mingw64\mingwvars.bat >NUL
if exist "%MINGW_SRC%\version_info.txt" copy /Y "%MINGW_SRC%\version_info.txt" build\dist\toolchain\mingw64\version_info.txt >NUL

exit /b 0
