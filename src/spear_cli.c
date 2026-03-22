#define _CRT_SECURE_NO_WARNINGS
#include <direct.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

static char g_lang[8] = "en";

static int lang_is(const char *code) {
    return _stricmp(g_lang, code) == 0;
}

static void load_lang_from_dir(const char *dir) {
    char path[4096];
    char parent[4096];
    char *slash;
    FILE *fp;
    if (strlen(dir) + 16 >= sizeof(path)) return;
    strcpy(path, dir);
    strcat(path, "\\sharp-lang.txt");
    fp = fopen(path, "rb");
    if (!fp) {
        strcpy(path, dir);
        strcat(path, "\\spear-lang.txt");
        fp = fopen(path, "rb");
    }
    if (!fp) {
        strcpy(parent, dir);
        slash = strrchr(parent, '\\');
        if (!slash) slash = strrchr(parent, '/');
        if (!slash) return;
        *slash = '\0';
        if (strlen(parent) + 16 >= sizeof(path)) return;
        strcpy(path, parent);
        strcat(path, "\\sharp-lang.txt");
        fp = fopen(path, "rb");
        if (!fp) {
            strcpy(path, parent);
            strcat(path, "\\spear-lang.txt");
            fp = fopen(path, "rb");
            if (!fp) return;
        }
    }
    if (fgets(g_lang, sizeof(g_lang), fp)) {
        if ((unsigned char) g_lang[0] == 0xEF && (unsigned char) g_lang[1] == 0xBB && (unsigned char) g_lang[2] == 0xBF) {
            memmove(g_lang, g_lang + 3, strlen(g_lang + 3) + 1);
        }
        for (size_t i = 0; g_lang[i]; i++) {
            if (g_lang[i] == '\r' || g_lang[i] == '\n') {
                g_lang[i] = '\0';
                break;
            }
        }
        if (_stricmp(g_lang, "ko") != 0 && _stricmp(g_lang, "en") != 0) {
            strcpy(g_lang, "en");
        }
    }
    fclose(fp);
}

static const char *cli_text(const char *key) {
    if (strcmp(key, "error_prefix") == 0) {
        return lang_is("ko") ? "sharp ?ㅻ쪟" : "sharp error";
    }
    if (strcmp(key, "usage") == 0) {
        return lang_is("ko")
            ? "?ъ슜踰?\n  sharp\n  sharp file.sharp\n  sharp <folder>\n  sharp build [file.sharp|folder]\n  sharp serve [file.sharp|folder]\n  sharp check [file.sharp|folder]\n  sharp new <name>\n"
            : "usage:\n  sharp\n  sharp file.sharp\n  sharp <folder>\n  sharp build [file.sharp|folder]\n  sharp serve [file.sharp|folder]\n  sharp check [file.sharp|folder]\n  sharp new <name>\n";
    }
    if (strcmp(key, "expected_sp") == 0) {
        return lang_is("ko") ? ".sharp ?뚯뒪 ?뚯씪?대굹 Sharp ?꾨줈?앺듃 ?대뜑媛 ?꾩슂?⑸땲?? : "expected a .sharp source file or a Sharp project folder";
    }
    if (strcmp(key, "missing_spearc") == 0) {
        return lang_is("ko") ? "sharp.exe ?놁뿉??sharpc.exe瑜?李얠쓣 ???놁뒿?덈떎" : "installed sharpc.exe was not found next to sharp.exe";
    }
    if (strcmp(key, "compile_failed") == 0) {
        return lang_is("ko") ? "sharp 而댄뙆???ㅻ쪟: ?뚯뒪 而댄뙆?쇱뿉 ?ㅽ뙣?덉뒿?덈떎" : "sharp compile error: source compilation failed";
    }
    if (strcmp(key, "backend_failed") == 0) {
        return lang_is("ko") ? "sharp 諛깆뿏???ㅻ쪟: ?ㅼ씠?곕툕 鍮뚮뱶???ㅽ뙣?덉뒿?덈떎" : "sharp backend error: native build failed";
    }
    if (strcmp(key, "details") == 0) {
        return lang_is("ko") ? "?먯꽭???댁슜" : "details";
    }
    if (strcmp(key, "details_missing") == 0) {
        return lang_is("ko") ? "?먯꽭???뺣낫瑜?李얠쓣 ???놁뒿?덈떎" : "details were not available";
    }
    if (strcmp(key, "checked") == 0) {
        return lang_is("ko") ? "寃???꾨즺" : "checked";
    }
    if (strcmp(key, "built") == 0) {
        return lang_is("ko") ? "鍮뚮뱶 ?꾨즺" : "built";
    }
    if (strcmp(key, "created") == 0) {
        return lang_is("ko") ? "?꾨줈?앺듃 ?앹꽦 ?꾨즺" : "created project";
    }
    if (strcmp(key, "new_exists") == 0) {
        return lang_is("ko") ? "????대뜑媛 ?대? 議댁옱?⑸땲?? : "target folder already exists";
    }
    if (strcmp(key, "expected_project_name") == 0) {
        return lang_is("ko") ? "???꾨줈?앺듃 ?대쫫???꾩슂?⑸땲?? : "expected a project name";
    }
    if (strcmp(key, "project_not_found") == 0) {
        return lang_is("ko") ? "Sharp ?꾨줈?앺듃 吏꾩엯 ?뚯씪??李얠쓣 ???놁뒿?덈떎" : "could not find a Sharp project entry file";
    }
    if (strcmp(key, "serve_prefix") == 0) {
        return "sharp serve";
    }
    return key;
}

static void fail(const char *message) {
    fprintf(stderr, "%s: %s\n", cli_text("error_prefix"), message);
    exit(1);
}

static void join_path(char *out, size_t cap, const char *a, const char *b) {
    if (cap == 0) fail("invalid output buffer");
    int written = snprintf(out, cap, "%s\\%s", a, b);
    if (written < 0 || (size_t) written >= cap || out[0] == '\0') {
        fail("failed to build a file path");
    }
}

static void format_text(char *out, size_t cap, const char *fmt, ...) {
    va_list args;
    if (cap == 0) fail("invalid output buffer");
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
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

static void path_basename(const char *path, char *out, size_t cap) {
    const char *name = strrchr(path, '\\');
    if (!name) name = strrchr(path, '/');
    name = name ? name + 1 : path;
    format_text(out, cap, "%s", name);
}

static void exe_dir(char *out, size_t cap) {
    DWORD len = GetModuleFileNameA(NULL, out, (DWORD) cap);
    if (len == 0 || len >= cap) fail("cannot resolve sharp executable path");
    char *slash = strrchr(out, '\\');
    if (!slash) fail("cannot resolve sharp executable directory");
    *slash = '\0';
}

static void parent_dir_inplace(char *path) {
    char *slash = strrchr(path, '\\');
    if (!slash) slash = strrchr(path, '/');
    if (!slash) fail("cannot resolve parent directory");
    *slash = '\0';
}

static void parent_dir_of(const char *path, char *out, size_t cap) {
    format_text(out, cap, "%s", path);
    parent_dir_inplace(out);
}

static bool path_exists(const char *path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

static bool dir_exists(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static bool file_exists(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static bool has_sharp_extension(const char *path) {
    size_t len = strlen(path);
    if (len >= 6 && _stricmp(path + len - 6, ".sharp") == 0) return true;
    if (len >= 3 && _stricmp(path + len - 3, ".sp") == 0) return true;
    return false;
}

static void ensure_dir(const char *path) {
    if (dir_exists(path)) return;
    if (_mkdir(path) != 0) {
        fail("failed to create directory");
    }
}

static void write_text_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fail("failed to create file");
    }
    size_t len = strlen(content);
    if (fwrite(content, 1, len, fp) != len) {
        fclose(fp);
        fail("failed to write file");
    }
    fclose(fp);
}

static bool parse_manifest_value(const char *manifest_path, const char *key, char *out, size_t cap) {
    FILE *fp = fopen(manifest_path, "rb");
    char line[2048];
    if (!fp) return false;
    while (fgets(line, sizeof(line), fp)) {
        char *cursor = line;
        while (*cursor == ' ' || *cursor == '\t') cursor++;
        size_t key_len = strlen(key);
        if (strncmp(cursor, key, key_len) != 0) continue;
        cursor += key_len;
        while (*cursor == ' ' || *cursor == '\t') cursor++;
        if (*cursor != '=') continue;
        cursor++;
        while (*cursor == ' ' || *cursor == '\t') cursor++;
        if (*cursor != '"') continue;
        cursor++;
        char *end = strchr(cursor, '"');
        if (!end) continue;
        *end = '\0';
        format_text(out, cap, "%s", cursor);
        fclose(fp);
        return true;
    }
    fclose(fp);
    return false;
}

static bool resolve_project_source(const char *raw_input, char *source_out, size_t source_cap, char *project_root_out, size_t root_cap, char *stem_out, size_t stem_cap) {
    char full_input[4096];
    char manifest_path[4096];
    char entry_rel[1024];
    char entry_path[4096];
    char candidate[4096];

    if (!raw_input || raw_input[0] == '\0') {
        raw_input = ".";
    }

    if (!_fullpath(full_input, raw_input, sizeof(full_input))) {
        return false;
    }

    if (file_exists(full_input) && has_sharp_extension(full_input)) {
        format_text(source_out, source_cap, "%s", full_input);
        parent_dir_of(full_input, project_root_out, root_cap);
        file_stem(full_input, stem_out, stem_cap);
        return true;
    }

    if (!dir_exists(full_input)) {
        return false;
    }

    format_text(project_root_out, root_cap, "%s", full_input);
    join_path(manifest_path, sizeof(manifest_path), full_input, "sharp.toml");
    if (!file_exists(manifest_path)) {
        join_path(manifest_path, sizeof(manifest_path), full_input, "spear.toml");
    }
    if (parse_manifest_value(manifest_path, "entry", entry_rel, sizeof(entry_rel))) {
        join_path(entry_path, sizeof(entry_path), full_input, entry_rel);
        if (file_exists(entry_path) && has_sp_extension(entry_path)) {
            format_text(source_out, source_cap, "%s", entry_path);
            if (!parse_manifest_value(manifest_path, "name", stem_out, stem_cap)) {
                path_basename(full_input, stem_out, stem_cap);
            }
            return true;
        }
    }

    join_path(candidate, sizeof(candidate), full_input, "main.sharp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }
    join_path(candidate, sizeof(candidate), full_input, "app\\main.sharp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }
    join_path(candidate, sizeof(candidate), full_input, "src\\main.sharp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }
    join_path(candidate, sizeof(candidate), full_input, "main.sp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }
    join_path(candidate, sizeof(candidate), full_input, "app\\main.sp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }
    join_path(candidate, sizeof(candidate), full_input, "src\\main.sp");
    if (file_exists(candidate)) {
        format_text(source_out, source_cap, "%s", candidate);
        path_basename(full_input, stem_out, stem_cap);
        return true;
    }

    return false;
}

static void create_project(const char *name) {
    char root[4096];
    char manifest[4096];
    char main_sp[4096];
    char gitignore[4096];
    char source[16384];
    char project_name[512];

    if (!name || !name[0]) {
        fail(cli_text("expected_project_name"));
    }
    if (path_exists(name)) {
        fail(cli_text("new_exists"));
    }

    if (!_fullpath(root, name, sizeof(root))) {
        fail("cannot resolve project path");
    }
    path_basename(root, project_name, sizeof(project_name));
    ensure_dir(root);

    join_path(manifest, sizeof(manifest), root, "sharp.toml");
    join_path(main_sp, sizeof(main_sp), root, "main.sharp");
    join_path(gitignore, sizeof(gitignore), root, ".gitignore");

    {
        char manifest_body[1024];
        format_text(
            manifest_body,
            sizeof(manifest_body),
            "name = \"%s\"\nentry = \"main.sharp\"\nkind = \"web\"\n",
            project_name
        );
        write_text_file(manifest, manifest_body);
    }

    format_text(
        source,
        sizeof(source),
        "import \"std/web.sharp\";\n"
        "\n"
        "run {\n"
        "    const title = \"%s\";\n"
        "    string html = landing_page(title, theme_product_bg()) {\n"
        "        centered(box(glass_panel_mod()) {\n"
        "            column_box(gap_space(space_4())) {\n"
        "                markup(\"span\", style_attr(badge_mod())) {\n"
        "                    \"Sharp App\";\n"
        "                };\n"
        "                markup(\"h1\", style_attr(hero_title_mod())) {\n"
        "                    \"Build sharp web pages with readable blocks.\";\n"
        "                };\n"
        "                markup(\"p\", style_attr(lead_copy_mod())) {\n"
        "                    \"Sharp gives you strong defaults, composable design tokens, and production-friendly page primitives.\";\n"
        "                };\n"
        "                row_box(gap_space(space_2())) {\n"
        "                    button_link(button_primary_mod(), \"#start\", \"Start building\");\n"
        "                    button_link(button_secondary_mod(), \"#signals\", \"See the layout\");\n"
        "                };\n"
        "            };\n"
        "        });\n"
        "        centered(split(\n"
        "            box(paper_panel_mod()) {\n"
        "                metric_bars(\n"
        "                    \"Quality floor\",\n"
        "                    \"A simple page should still land well.\",\n"
        "                    pack(\"Readability\", \"Visual quality\", \"Remix freedom\"),\n"
        "                    pack(92, 88, 90),\n"
        "                    pack(tone_brand(), tone_warm(), tone_ink())\n"
        "                );\n"
        "            },\n"
        "            form_card(\n"
        "                \"Ship the first version\",\n"
        "                \"This starter already has real form controls and action primitives.\",\n"
        "                column_box(gap_space(space_2())) {\n"
        "                    text_input(\"Project\", \"project\", title, \"Project name\");\n"
        "                    email_input(\"Owner\", \"owner\", \"\", \"team@example.com\");\n"
        "                    text_area(\"Notes\", \"notes\", \"\", \"What should this page launch with?\", \"4\");\n"
        "                },\n"
        "                \"/start\",\n"
        "                \"Create prototype\"\n"
        "            )\n"
        "        ));\n"
        "    };\n"
        "    write(\"build/sharp-ui.html\", html);\n"
        "}\n",
        project_name
    );
    write_text_file(main_sp, source);
    write_text_file(gitignore, "build/\n");

    printf("%s %s\n", cli_text("created"), root);
}

static void resolve_bundled_gcc(char *out, size_t cap, const char *bin_dir) {
    char install_root[4096];
    char candidate[4096];
    format_text(install_root, sizeof(install_root), "%s", bin_dir);
    parent_dir_inplace(install_root);

    join_path(candidate, sizeof(candidate), install_root, "toolchain\\mingw64\\bin\\gcc.exe");
    if (GetFileAttributesA(candidate) != INVALID_FILE_ATTRIBUTES) {
        format_text(out, cap, "%s", candidate);
        return;
    }

    if (SearchPathA(NULL, "gcc.exe", NULL, (DWORD) cap, out, NULL) && out[0]) {
        return;
    }

    fail("gcc.exe was not found. Re-run setup and install the bundled toolchain.");
}

static void resolve_runtime_script(char *out, size_t cap, const char *bin_dir, const char *leaf) {
    char install_root[4096];
    char runtime_dir[4096];
    format_text(install_root, sizeof(install_root), "%s", bin_dir);
    parent_dir_inplace(install_root);
    join_path(runtime_dir, sizeof(runtime_dir), install_root, "runtime");
    join_path(out, cap, runtime_dir, leaf);
}

static void delete_if_exists(const char *path) {
    if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
        for (int i = 0; i < 20; i++) {
            if (DeleteFileA(path)) return;
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
        fprintf(stderr, "%s\n", cli_text("details_missing"));
        return;
    }
    for (;;) {
        char buffer[1024];
        size_t read = fread(buffer, 1, sizeof(buffer), fp);
        if (read == 0) break;
        fwrite(buffer, 1, read, stderr);
    }
    fclose(fp);
    if (prefix[0] && prefix[strlen(prefix) - 1] != '\n') {
        fputc('\n', stderr);
    }
}

static int run_process_capture(const char *command_line, const char *log_path, const char *working_dir) {
    SECURITY_ATTRIBUTES sa;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    HANDLE log_handle = INVALID_HANDLE_VALUE;

    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) fail("failed to create build log pipe");
    if (!SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        fail("failed to configure build log pipe");
    }

    log_handle = CreateFileA(log_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

    BOOL ok = CreateProcessA(NULL, mutable_cmd, NULL, NULL, TRUE, 0, NULL, working_dir, &si, &pi);
    CloseHandle(write_pipe);
    if (!ok) {
        CloseHandle(read_pipe);
        CloseHandle(log_handle);
        return -1;
    }

    for (;;) {
        char buffer[4096];
        DWORD bytes_read = 0;
        if (!ReadFile(read_pipe, buffer, sizeof(buffer), &bytes_read, NULL) || bytes_read == 0) break;
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

static int run_process_console(const char *command_line, const char *working_dir) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    char mutable_cmd[4096];
    format_text(mutable_cmd, sizeof(mutable_cmd), "%s", command_line);

    if (!CreateProcessA(NULL, mutable_cmd, NULL, NULL, TRUE, 0, NULL, working_dir, &si, &pi)) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int) exit_code;
}

static int run_process_quiet(const char *command_line, const char *working_dir) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    HANDLE null_in = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE null_out = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (null_in == INVALID_HANDLE_VALUE || null_out == INVALID_HANDLE_VALUE) {
        if (null_in != INVALID_HANDLE_VALUE) CloseHandle(null_in);
        if (null_out != INVALID_HANDLE_VALUE) CloseHandle(null_out);
        return run_process_console(command_line, working_dir);
    }

    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = null_in;
    si.hStdOutput = null_out;
    si.hStdError = null_out;

    char mutable_cmd[4096];
    format_text(mutable_cmd, sizeof(mutable_cmd), "%s", command_line);

    BOOL ok = CreateProcessA(NULL, mutable_cmd, NULL, NULL, TRUE, 0, NULL, working_dir, &si, &pi);
    CloseHandle(null_in);
    CloseHandle(null_out);
    if (!ok) return -1;

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int) exit_code;
}

int main(int argc, char **argv) {
    const char *mode = "run";
    const char *raw_input = ".";
    char install_dir[4096];
    char spearc_path[4096];
    char project_root[4096];
    char input[4096];
    char stem[512];
    char build_dir[4096];
    char temp_dir[4096];
    char c_out[4096];
    char exe_out[4096];
    char front_log[4096];
    char back_log[4096];
    char command[4096];
    char gcc_path[4096];
    char serve_script[4096];

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    exe_dir(install_dir, sizeof(install_dir));
    load_lang_from_dir(install_dir);

    if (argc >= 2 && _stricmp(argv[1], "new") == 0) {
        if (argc < 3) {
            fail(cli_text("expected_project_name"));
        }
        create_project(argv[2]);
        return 0;
    }

    if (argc >= 2 &&
        (_stricmp(argv[1], "build") == 0 ||
         _stricmp(argv[1], "serve") == 0 ||
         _stricmp(argv[1], "service") == 0 ||
         _stricmp(argv[1], "check") == 0)) {
        mode = _stricmp(argv[1], "service") == 0 ? "serve" : argv[1];
        raw_input = argc >= 3 ? argv[2] : ".";
    } else if (argc >= 2) {
        raw_input = argv[1];
    }

    if (!resolve_project_source(raw_input, input, sizeof(input), project_root, sizeof(project_root), stem, sizeof(stem))) {
        fail(cli_text("project_not_found"));
    }

    join_path(spearc_path, sizeof(spearc_path), install_dir, "sharpc.exe");
    if (GetFileAttributesA(spearc_path) == INVALID_FILE_ATTRIBUTES) {
        join_path(spearc_path, sizeof(spearc_path), install_dir, "spearc.exe");
    }
    if (GetFileAttributesA(spearc_path) == INVALID_FILE_ATTRIBUTES) {
        fail(cli_text("missing_spearc"));
    }

    join_path(build_dir, sizeof(build_dir), project_root, "build");
    ensure_dir(build_dir);

    if (_stricmp(mode, "build") == 0) {
        format_text(c_out, sizeof(c_out), "%s\\%s.c", build_dir, stem);
        format_text(exe_out, sizeof(exe_out), "%s\\%s.exe", build_dir, stem);
    format_text(front_log, sizeof(front_log), "%s\\%s.sharpc.log", build_dir, stem);
        format_text(back_log, sizeof(back_log), "%s\\%s.native.log", build_dir, stem);
    } else {
        DWORD pid = GetCurrentProcessId();
        temp_runtime_dir(temp_dir, sizeof(temp_dir));
        format_text(c_out, sizeof(c_out), "%s\\%s-%lu.c", temp_dir, stem, (unsigned long) pid);
        format_text(exe_out, sizeof(exe_out), "%s\\%s-%lu.exe", temp_dir, stem, (unsigned long) pid);
    format_text(front_log, sizeof(front_log), "%s\\%s-%lu.sharpc.log", temp_dir, stem, (unsigned long) pid);
        format_text(back_log, sizeof(back_log), "%s\\%s-%lu.native.log", temp_dir, stem, (unsigned long) pid);
    }

    if (_stricmp(mode, "check") == 0) {
        format_text(command, sizeof(command), "\"%s\" --check \"%s\"", spearc_path, input);
    } else {
        format_text(command, sizeof(command), "\"%s\" \"%s\" -o \"%s\"", spearc_path, input, c_out);
    }

    if (run_process_capture(command, front_log, project_root) != 0) {
        if (_stricmp(mode, "build") == 0) {
            fprintf(stderr, "%s\n", cli_text("compile_failed"));
            fprintf(stderr, "%s: %s\n", cli_text("details"), front_log);
        } else {
            print_log_and_cleanup(cli_text("compile_failed"), front_log);
            cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
        }
        return 1;
    }

    if (_stricmp(mode, "check") == 0) {
        cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
        printf("%s %s\n", cli_text("checked"), input);
        return 0;
    }

    resolve_bundled_gcc(gcc_path, sizeof(gcc_path), install_dir);
    format_text(command, sizeof(command), "\"%s\" -O3 -w -std=c11 -o \"%s\" \"%s\"", gcc_path, exe_out, c_out);
    if (run_process_capture(command, back_log, project_root) != 0) {
        if (_stricmp(mode, "build") == 0) {
            fprintf(stderr, "%s\n", cli_text("backend_failed"));
            fprintf(stderr, "%s: %s\n", cli_text("details"), back_log);
        } else {
            print_log_and_cleanup(cli_text("backend_failed"), back_log);
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
        printf("%s %s\n", cli_text("built"), exe_out);
        return 0;
    }

    format_text(command, sizeof(command), "\"%s\"", exe_out);
    int run_exit = _stricmp(mode, "serve") == 0 ? run_process_quiet(command, project_root) : run_process_console(command, project_root);
    cleanup_temp_dir(temp_dir, c_out, exe_out, front_log, back_log);
    if (run_exit != 0) {
        return 1;
    }

    if (_stricmp(mode, "serve") == 0) {
        char html_path[4096];
        const char *html = "http://127.0.0.1:4173/";
        join_path(html_path, sizeof(html_path), build_dir, "sharp-ui.html");
        if (GetFileAttributesA(html_path) != INVALID_FILE_ATTRIBUTES) {
            html = "http://127.0.0.1:4173/sharp-ui.html";
        } else {
            join_path(html_path, sizeof(html_path), build_dir, "spear-ui.html");
            if (GetFileAttributesA(html_path) != INVALID_FILE_ATTRIBUTES) {
                html = "http://127.0.0.1:4173/spear-ui.html";
            }
        }
        printf("%s: %s\n", cli_text("serve_prefix"), html);
        resolve_runtime_script(serve_script, sizeof(serve_script), install_dir, "serve_static.ps1");
        format_text(command, sizeof(command), "cmd /c start \"\" cmd /c \"ping -n 3 127.0.0.1 >nul && start \"\" \"%s\"\"", html);
        run_process_console(command, project_root);
        format_text(command, sizeof(command), "powershell -NoProfile -ExecutionPolicy Bypass -File \"%s\" -Root \"%s\" -Port 4173", serve_script, build_dir);
        return run_process_console(command, project_root);
    }

    return 0;
}

