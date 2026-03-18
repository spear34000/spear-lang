#define _CRT_SECURE_NO_WARNINGS
#include <direct.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

static void fail(const char *message) {
    fprintf(stderr, "spear error: %s\n", message);
    exit(1);
}

static void join_path(char *out, size_t cap, const char *a, const char *b) {
    if (cap == 0) {
        fail("invalid output buffer");
    }
    int written = snprintf(out, cap, "%s\\%s", a, b);
    if (written < 0 || (size_t) written >= cap || out[0] == '\0') {
        fail("failed to build a file path");
    }
}

static void format_text(char *out, size_t cap, const char *fmt, ...) {
    va_list args;
    if (cap == 0) {
        fail("invalid output buffer");
    }
    va_start(args, fmt);
    int written = vsnprintf(out, cap, fmt, args);
    va_end(args);
    if (written < 0 || (size_t) written >= cap || out[0] == '\0') {
        fail("failed to build command or path");
    }
}

static void file_stem(const char *path, char *out, size_t cap) {
    const char *name = strrchr(path, '\\');
    if (!name) name = strrchr(path, '/');
    name = name ? name + 1 : path;
    format_text(out, cap, "%s", name);
    if (out[0] == '\0') {
        fail("failed to resolve source file name");
    }
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

static void exe_dir(char *out, size_t cap) {
    DWORD len = GetModuleFileNameA(NULL, out, (DWORD) cap);
    if (len == 0 || len >= cap) fail("cannot resolve spear executable path");
    char *slash = strrchr(out, '\\');
    if (!slash) fail("cannot resolve spear executable directory");
    *slash = '\0';
}

static void ensure_build_dir(void) {
    _mkdir("build");
}

static void delete_if_exists(const char *path) {
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
        for (int i = 0; i < 20; i++) {
            if (DeleteFileA(path)) {
                return;
            }
            Sleep(25);
        }
    }
}

static void remove_dir_if_exists(const char *path) {
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
        RemoveDirectoryA(path);
    }
}

static void temp_runtime_dir(char *out, size_t cap) {
    char temp_root[1024];
    char temp_name[1024];
    DWORD len = GetTempPathA((DWORD) sizeof(temp_root), temp_root);
    if (len == 0 || len >= sizeof(temp_root)) {
        fail("cannot resolve temp directory");
    }
    if (!GetTempFileNameA(temp_root, "spr", 0, temp_name)) {
        fail("cannot create temp working area");
    }
    DeleteFileA(temp_name);
    if (!CreateDirectoryA(temp_name, NULL)) {
        fail("cannot create temp working area");
    }
    format_text(out, cap, "%s", temp_name);
}

static void cleanup_temp_dir(const char *dir, const char *c_out, const char *exe_out, const char *front_log, const char *back_log) {
    delete_if_exists(c_out);
    delete_if_exists(exe_out);
    delete_if_exists(front_log);
    delete_if_exists(back_log);
    remove_dir_if_exists(dir);
}

static void print_log_and_cleanup(const char *prefix, const char *log_path) {
    FILE *fp = fopen(log_path, "rb");
    fprintf(stderr, "%s\n", prefix);
    if (!fp) {
        fprintf(stderr, "details were not available\n");
        return;
    }
    for (;;) {
        char buffer[1024];
        size_t read = fread(buffer, 1, sizeof(buffer), fp);
        if (read == 0) {
            break;
        }
        fwrite(buffer, 1, read, stderr);
    }
    fclose(fp);
    if (prefix[0] && prefix[strlen(prefix) - 1] != '\n') {
        fputc('\n', stderr);
    }
}

static int run_process_capture(const char *command_line, const char *log_path) {
    SECURITY_ATTRIBUTES sa;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    HANDLE log_handle = INVALID_HANDLE_VALUE;

    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        fail("failed to create build log pipe");
    }
    if (!SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        fail("failed to configure build log pipe");
    }

    log_handle = CreateFileA(
        log_path,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (log_handle == INVALID_HANDLE_VALUE) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        fail("failed to open build log");
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;

    char mutable_cmd[4096];
    format_text(mutable_cmd, sizeof(mutable_cmd), "%s", command_line);

    BOOL ok = CreateProcessA(
        NULL,
        mutable_cmd,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
    CloseHandle(write_pipe);

    if (!ok) {
        CloseHandle(read_pipe);
        CloseHandle(log_handle);
        return -1;
    }

    for (;;) {
        char buffer[4096];
        DWORD bytes_read = 0;
        if (!ReadFile(read_pipe, buffer, sizeof(buffer), &bytes_read, NULL) || bytes_read == 0) {
            break;
        }
        DWORD bytes_written = 0;
        WriteFile(log_handle, buffer, bytes_read, &bytes_written, NULL);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(read_pipe);
    CloseHandle(log_handle);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int) exit_code;
}

static int run_process_console(const char *command_line) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    char mutable_cmd[4096];
    format_text(mutable_cmd, sizeof(mutable_cmd), "%s", command_line);

    if (!CreateProcessA(NULL, mutable_cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int) exit_code;
}

int main(int argc, char **argv) {
    const char *mode = "run";
    const char *input = NULL;
    char install_dir[4096];
    char spearc_path[4096];
    char temp_dir[4096];
    char stem[512];
    char c_out[4096];
    char exe_out[4096];
    char front_log[4096];
    char back_log[4096];
    char command[4096];

    if (argc < 2) {
        fprintf(stderr, "usage:\n  spear file.sp\n  spear build file.sp\n  spear serve file.sp\n  spear check file.sp\n");
        return 1;
    }

    if (_stricmp(argv[1], "build") == 0 || _stricmp(argv[1], "serve") == 0 || _stricmp(argv[1], "check") == 0) {
        if (argc < 3) {
            fprintf(stderr, "usage:\n  spear file.sp\n  spear build file.sp\n  spear serve file.sp\n  spear check file.sp\n");
            return 1;
        }
        mode = argv[1];
        input = argv[2];
    } else {
        input = argv[1];
    }

    size_t input_len = strlen(input);
    if (input_len < 3 || _stricmp(input + input_len - 3, ".sp") != 0) {
        fail("expected a .sp source file");
    }

    exe_dir(install_dir, sizeof(install_dir));
    join_path(spearc_path, sizeof(spearc_path), install_dir, "spearc.exe");
    if (GetFileAttributesA(spearc_path) == INVALID_FILE_ATTRIBUTES) {
        fail("installed spearc.exe was not found next to spear.exe");
    }

    ensure_build_dir();
    file_stem(input, stem, sizeof(stem));

    if (_stricmp(mode, "build") == 0) {
        format_text(c_out, sizeof(c_out), "build\\%s.c", stem);
        format_text(exe_out, sizeof(exe_out), "build\\%s.exe", stem);
    } else {
        DWORD pid = GetCurrentProcessId();
        temp_runtime_dir(temp_dir, sizeof(temp_dir));
        format_text(c_out, sizeof(c_out), "%s\\%s-%lu.c", temp_dir, stem, (unsigned long) pid);
        format_text(exe_out, sizeof(exe_out), "%s\\%s-%lu.exe", temp_dir, stem, (unsigned long) pid);
    }
    if (_stricmp(mode, "build") == 0) {
        format_text(front_log, sizeof(front_log), "build\\%s.spearc.log", stem);
        format_text(back_log, sizeof(back_log), "build\\%s.native.log", stem);
    } else {
        format_text(front_log, sizeof(front_log), "%s\\%s-%lu.spearc.log", temp_dir, stem, (unsigned long) GetCurrentProcessId());
        format_text(back_log, sizeof(back_log), "%s\\%s-%lu.native.log", temp_dir, stem, (unsigned long) GetCurrentProcessId());
    }

    if (_stricmp(mode, "check") == 0) {
        format_text(command, sizeof(command), "\"%s\" --check \"%s\"", spearc_path, input);
    } else {
        format_text(command, sizeof(command), "\"%s\" \"%s\" -o \"%s\"", spearc_path, input, c_out);
    }
    if (run_process_capture(command, front_log) != 0) {
        if (_stricmp(mode, "build") == 0) {
            fprintf(stderr, "spear compile error: source compilation failed\n");
            fprintf(stderr, "details: %s\n", front_log);
        } else {
            print_log_and_cleanup("spear compile error: source compilation failed", front_log);
            cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
        }
        return 1;
    }
    if (_stricmp(mode, "check") == 0) {
        cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
        printf("checked %s\n", input);
        return 0;
    }

    format_text(command, sizeof(command), "gcc -O3 -w -std=c11 -o \"%s\" \"%s\"", exe_out, c_out);
    if (run_process_capture(command, back_log) != 0) {
        if (_stricmp(mode, "build") == 0) {
            fprintf(stderr, "spear backend error: native build failed\n");
            fprintf(stderr, "details: %s\n", back_log);
        } else {
            print_log_and_cleanup("spear backend error: native build failed", back_log);
            cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
        }
        return 1;
    }
    if (_stricmp(mode, "build") != 0) {
        delete_if_exists(c_out);
    }
    delete_if_exists(front_log);
    delete_if_exists(back_log);

    if (_stricmp(mode, "build") == 0) {
        printf("built %s\n", exe_out);
        return 0;
    }

    format_text(command, sizeof(command), "\"%s\"", exe_out);
    int run_exit = run_process_console(command);
    cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
    if (run_exit != 0) {
        return 1;
    }

    if (_stricmp(mode, "serve") == 0) {
        const char *html = GetFileAttributesA("build\\spear-ui.html") != INVALID_FILE_ATTRIBUTES
            ? "http://127.0.0.1:4173/spear-ui.html"
            : "http://127.0.0.1:4173/";
        printf("spear serve: %s\n", html);
        format_text(command, sizeof(command), "cmd /c start \"\" \"%s\"", html);
        run_process_console(command);
        return run_process_console("python -m http.server 4173 --directory build");
    }

    return 0;
}
