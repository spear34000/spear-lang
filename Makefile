CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
SETUP_LIBS ?= -lcomctl32

all: spear setup

spearc: src/spearc.c
	$(CC) $(CFLAGS) -o build/spearc.exe src/spearc.c

spear: spearc src/spear_cli.c
	$(CC) $(CFLAGS) -o build/spear.exe src/spear_cli.c

setup: spear src/spear_setup.c
	$(CC) $(CFLAGS) -o build/spear-setup.exe src/spear_setup.c $(SETUP_LIBS)

example: spear
	./build/spear.exe examples/hello.sp

check: spear
	./build/spear.exe check examples/hello.sp
	./build/spear.exe check examples/bridge.sp
	./build/spear.exe build examples/web.sp
	scripts\\check_regressions.cmd

dist: setup
	if not exist build\\dist mkdir build\\dist
	copy /Y build\\spear.exe build\\dist\\spear.exe > NUL
	copy /Y build\\spearc.exe build\\dist\\spearc.exe > NUL
	copy /Y build\\spear-setup.exe build\\dist\\spear-setup.exe > NUL
	if not exist build\\dist\\runtime mkdir build\\dist\\runtime
	copy /Y runtime\\bridge_node.mjs build\\dist\\runtime\\bridge_node.mjs > NUL
	copy /Y runtime\\bridge_python.py build\\dist\\runtime\\bridge_python.py > NUL
	copy /Y runtime\\demo_node.cjs build\\dist\\runtime\\demo_node.cjs > NUL
	copy /Y runtime\\demo_python.py build\\dist\\runtime\\demo_python.py > NUL
	if exist build\\dist\\examples rmdir /S /Q build\\dist\\examples
	xcopy /E /I /Y examples build\\dist\\examples > NUL
	if exist build\\dist\\vscode-spear rmdir /S /Q build\\dist\\vscode-spear
	if not exist build\\dist\\vscode-spear mkdir build\\dist\\vscode-spear
	copy /Y vscode-spear\\extension.js build\\dist\\vscode-spear\\extension.js > NUL
	copy /Y vscode-spear\\language-configuration.json build\\dist\\vscode-spear\\language-configuration.json > NUL
	copy /Y vscode-spear\\package.json build\\dist\\vscode-spear\\package.json > NUL
	copy /Y vscode-spear\\README.md build\\dist\\vscode-spear\\README.md > NUL
	if not exist build\\dist\\vscode-spear\\assets mkdir build\\dist\\vscode-spear\\assets
	if not exist build\\dist\\vscode-spear\\snippets mkdir build\\dist\\vscode-spear\\snippets
	if not exist build\\dist\\vscode-spear\\syntaxes mkdir build\\dist\\vscode-spear\\syntaxes
	copy /Y vscode-spear\\assets\\spear-logo.png build\\dist\\vscode-spear\\assets\\spear-logo.png > NUL
	copy /Y vscode-spear\\assets\\spear-logo.svg build\\dist\\vscode-spear\\assets\\spear-logo.svg > NUL
	copy /Y vscode-spear\\snippets\\spear.json build\\dist\\vscode-spear\\snippets\\spear.json > NUL
	copy /Y vscode-spear\\syntaxes\\spear.tmLanguage.json build\\dist\\vscode-spear\\syntaxes\\spear.tmLanguage.json > NUL

clean:
	del /Q build\*.exe 2> NUL
	del /Q build\*.c 2> NUL
	del /Q build\*.log 2> NUL
	if exist build\\dist rmdir /S /Q build\\dist
