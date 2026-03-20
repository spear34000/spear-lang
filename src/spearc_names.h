#ifndef SPEARC_NAMES_H
#define SPEARC_NAMES_H

static void import_export_add(ImportInfo *info, const char *name) {
    for (size_t i = 0; i < info->exported_count; i++) {
        if (strcmp(info->exported_functions[i], name) == 0) {
            return;
        }
    }
    if (info->exported_count == info->exported_cap) {
        size_t next = info->exported_cap ? info->exported_cap * 2 : 8;
        info->exported_functions = realloc(info->exported_functions, next * sizeof(char *));
        if (!info->exported_functions) {
            fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("oom"));
            exit(1);
        }
        info->exported_cap = next;
    }
    info->exported_functions[info->exported_count++] = xstrdup(name);
}

static char *sanitize_symbol_part(const char *src) {
    Buffer out;
    buf_init(&out);
    if (!src || !src[0]) {
        buf_append(&out, "global");
        return buf_take(&out);
    }
    for (size_t i = 0; src[i]; i++) {
        char c = src[i];
        if (isalnum((unsigned char) c) || c == '_') {
            char tmp[2] = { c, '\0' };
            buf_append(&out, tmp);
        } else {
            buf_append(&out, "_");
        }
    }
    return buf_take(&out);
}

static char *make_function_c_name(const char *package_name, const char *module_name, const char *name, bool is_entry) {
    if (is_entry) {
        return xstrdup("__spear_entry_main");
    }
    char *pkg = sanitize_symbol_part(package_name);
    char *mod = sanitize_symbol_part(module_name);
    char *fn = sanitize_symbol_part(name);
    Buffer out;
    buf_init(&out);
    buf_appendf(&out, "spear_fn_%s_%s_%s", pkg, mod, fn);
    free(pkg);
    free(mod);
    free(fn);
    return buf_take(&out);
}

static char *token_text(Token token) {
    return slice_dup(token.start, token.length);
}

static bool token_is_ident_text(Token token, const char *text) {
    size_t len = strlen(text);
    return token.kind == TOK_IDENT && token.length == len && strncmp(token.start, text, len) == 0;
}

#endif
