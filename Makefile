CC = gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
SETUP_LIBS ?= -lcomctl32
SETUP_LDFLAGS ?= -mwindows

all: sharp setup

sharpc-backend: src/spearc.c
	$(CC) $(CFLAGS) -o build/sharpc-c.exe src/spearc.c

rust-bins: Cargo.toml rust-toolchain.toml crates/sharp-common/src/lib.rs crates/sharp/src/main.rs crates/sharpc/Cargo.toml crates/sharpc/src/main.rs
	scripts\\build_rust.cmd

sharpc: sharpc-backend rust-bins

spearc: sharpc

sharp: sharpc

spear: sharp

setup: sharp src/spear_setup.c
	$(CC) $(CFLAGS) $(SETUP_LDFLAGS) -o build/sharp-setup.exe src/spear_setup.c $(SETUP_LIBS)
	$(CC) $(CFLAGS) $(SETUP_LDFLAGS) -o build/spear-setup.exe src/spear_setup.c $(SETUP_LIBS)

example: sharp
	./build/sharp.exe examples/hello.sp

audit:
	powershell -ExecutionPolicy Bypass -File scripts\\audit_runtime.ps1

check: sharp
	./build/sharp.exe check examples/hello.sp
	./build/sharp.exe check examples/bridge.sp
	./build/sharp.exe check examples/map_demo.sp
	./build/sharp.exe check examples/platform.sp
	./build/sharp.exe check examples/security.sp
	./build/sharp.exe check examples/http_native.sp
	./build/sharp.exe check examples/native_security.sp
	./build/sharp.exe check examples/native_runtime.sp
	./build/sharp.exe check examples/native_collections.sp
	./build/sharp.exe check examples/net_native.sp
	./build/sharp.exe check examples/mobile.sp
	./build/sharp.exe check examples/ui.sp
	./build/sharp.exe check examples/ui_web_native.sp
	./build/sharp.exe check examples/result_demo.sp
	./build/sharp.exe check examples/result_flow.sp
	./build/sharp.exe check examples/sharp.sp
	./build/sharp.exe check examples/tool.sp
	./build/sharp.exe check examples/server.sp
	./build/sharp.exe build examples/web.sp
	scripts\\check_regressions.cmd
	del /Q build\\web.c 2> NUL
	del /Q build\\web.exe 2> NUL
	del /Q build\\sharp-ui.html 2> NUL
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
