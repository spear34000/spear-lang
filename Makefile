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

audit:
	powershell -ExecutionPolicy Bypass -File scripts\\audit_runtime.ps1

check: spear
	./build/spear.exe check examples/hello.sp
	./build/spear.exe check examples/bridge.sp
	./build/spear.exe check examples/map_demo.sp
	./build/spear.exe check examples/platform.sp
	./build/spear.exe check examples/security.sp
	./build/spear.exe check examples/http_native.sp
	./build/spear.exe check examples/native_security.sp
	./build/spear.exe check examples/native_runtime.sp
	./build/spear.exe check examples/native_collections.sp
	./build/spear.exe check examples/net_native.sp
	./build/spear.exe check examples/mobile.sp
	./build/spear.exe check examples/ui.sp
	./build/spear.exe check examples/ui_web_native.sp
	./build/spear.exe check examples/result_demo.sp
	./build/spear.exe check examples/result_flow.sp
	./build/spear.exe check examples/sharp.sp
	./build/spear.exe check examples/tool.sp
	./build/spear.exe check examples/server.sp
	./build/spear.exe build examples/web.sp
	scripts\\check_regressions.cmd
	del /Q build\\web.c 2> NUL
	del /Q build\\web.exe 2> NUL
	del /Q build\\spear-ui.html 2> NUL
	del /Q build\\regression_run.log 2> NUL

dist: setup
	scripts\\package_dist.cmd

clean:
	del /Q build\*.exe 2> NUL
	del /Q build\*.c 2> NUL
	del /Q build\*.log 2> NUL
	del /Q build\*.html 2> NUL
	del /Q build\*.wixpdb 2> NUL
	if exist build\\dist rmdir /S /Q build\\dist
