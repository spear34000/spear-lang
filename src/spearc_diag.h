#ifndef SPEARC_DIAG_H
#define SPEARC_DIAG_H

static bool compiler_lang_is(const char *code) {
    return strcmp(g_compiler_lang, code) == 0;
}

static void compiler_load_lang(const char *tool_dir) {
    char path[2048];
    FILE *fp;
    snprintf(path, sizeof(path), "%s\\spear-lang.txt", tool_dir);
    fp = fopen(path, "rb");
    if (!fp) return;
    if (fgets(g_compiler_lang, sizeof(g_compiler_lang), fp)) {
        if ((unsigned char) g_compiler_lang[0] == 0xEF && (unsigned char) g_compiler_lang[1] == 0xBB && (unsigned char) g_compiler_lang[2] == 0xBF) {
            memmove(g_compiler_lang, g_compiler_lang + 3, strlen(g_compiler_lang + 3) + 1);
        }
        for (size_t i = 0; g_compiler_lang[i]; i++) {
            if (g_compiler_lang[i] == '\r' || g_compiler_lang[i] == '\n') {
                g_compiler_lang[i] = '\0';
                break;
            }
        }
        if (strcmp(g_compiler_lang, "ko") != 0 && strcmp(g_compiler_lang, "en") != 0) {
            strcpy(g_compiler_lang, "en");
        }
    }
    fclose(fp);
}

static void fatal_at(int line, int col, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s [line %d:%d] ", compiler_message("error_prefix"), line, col);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    if (g_collect_errors && g_recover_env) {
        g_error_count++;
        longjmp(*g_recover_env, 1);
    }
    exit(1);
}

static void warning_at(int line, int col, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s [line %d:%d] ", compiler_message("warning_prefix"), line, col);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

#endif
