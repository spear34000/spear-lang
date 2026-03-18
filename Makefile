CC = gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
SETUP_LIBS ?= -lcomctl32
SETUP_LDFLAGS ?= -mwindows

all: spear setup

spearc: src/spearc.c
	$(CC) $(CFLAGS) -o build/spearc.exe src/spearc.c

spear: spearc src/spear_cli.c
	$(CC) $(CFLAGS) -o build/spear.exe src/spear_cli.c

setup: spear src/spear_setup.c
	$(CC) $(CFLAGS) $(SETUP_LDFLAGS) -o build/spear-setup.exe src/spear_setup.c $(SETUP_LIBS)

example: spear
	./build/spear.exe examples/hello.sp

check: spear
	./build/spear.exe check examples/hello.sp
	./build/spear.exe check examples/bridge.sp
	./build/spear.exe build examples/web.sp
	scripts\\check_regressions.cmd
	del /Q build\\web.c 2> NUL
	del /Q build\\web.exe 2> NUL
	del /Q build\\spear-ui.html 2> NUL
	del /Q build\\regression_run.log 2> NUL

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
	if exist build\\dist\\std rmdir /S /Q build\\dist\\std
	xcopy /E /I /Y std build\\dist\\std > NUL
	if exist build\\dist\\examples rmdir /S /Q build\\dist\\examples
	xcopy /E /I /Y examples build\\dist\\examples > NUL
	if exist build\\dist\\vscode-spear rmdir /S /Q build\\dist\\vscode-spear
	xcopy /E /I /Y vscode-spear build\\dist\\vscode-spear > NUL
	del /Q build\\dist\\vscode-spear\\*.vsix 2> NUL

clean:
	del /Q build\*.exe 2> NUL
	del /Q build\*.c 2> NUL
	del /Q build\*.log 2> NUL
	del /Q build\*.html 2> NUL
	del /Q build\*.wixpdb 2> NUL
	if exist build\\dist rmdir /S /Q build\\dist
