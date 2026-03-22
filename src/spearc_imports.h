#ifndef SPEARC_IMPORTS_H
#define SPEARC_IMPORTS_H

typedef struct {
    char **items;
    size_t count;
    size_t cap;
} PathList;

static bool path_list_has(PathList *list, const char *path) {
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], path) == 0) {
            return true;
        }
    }
    return false;
}

static void path_list_add(PathList *list, const char *path) {
    if (path_list_has(list, path)) {
        return;
    }
    if (list->count == list->cap) {
        size_t next = list->cap ? list->cap * 2 : 8;
        list->items = realloc(list->items, next * sizeof(char *));
        if (!list->items) {
            fprintf(stderr, "spearc error: out of memory\n");
            exit(1);
        }
        list->cap = next;
    }
    list->items[list->count++] = xstrdup(path);
}

static void path_list_remove(PathList *list, const char *path) {
    for (size_t i = list->count; i > 0; i--) {
        if (strcmp(list->items[i - 1], path) == 0) {
            free(list->items[i - 1]);
            if (i < list->count) {
                memmove(&list->items[i - 1], &list->items[i], (list->count - i) * sizeof(char *));
            }
            list->count--;
            return;
        }
    }
}

static void import_list_add(ImportInfo **items, size_t *count, size_t *cap, const char *path, const char *label, int line) {
    if (*count == *cap) {
        size_t next = *cap ? *cap * 2 : 8;
        *items = realloc(*items, next * sizeof(ImportInfo));
        if (!*items) {
            fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("oom"));
            exit(1);
        }
        *cap = next;
    }
    (*items)[*count].path = xstrdup(path);
    (*items)[*count].label = xstrdup(label);
    (*items)[*count].line = line;
    (*items)[*count].exported_functions = NULL;
    (*items)[*count].exported_count = 0;
    (*items)[*count].exported_cap = 0;
    (*count)++;
}

static void append_line_directive(Buffer *out, int line) {
    char marker[32];
    checked_snprintf(marker, sizeof(marker), "\x1F%d\n", line > 0 ? line : 1);
    buf_append(out, marker);
}

static void parent_dir_of(const char *path, char *out, size_t cap) {
    checked_snprintf(out, cap, "%s", path);
    char *slash = strrchr(out, '/');
    char *backslash = strrchr(out, '\\');
    char *cut = slash > backslash ? slash : backslash;
    if (cut) {
        *cut = '\0';
    } else {
        checked_snprintf(out, cap, ".");
    }
}

static void join_fs_path(char *out, size_t cap, const char *base, const char *leaf) {
    if (leaf[0] == '\\' || leaf[0] == '/' || (strlen(leaf) > 1 && leaf[1] == ':')) {
        checked_snprintf(out, cap, "%s", leaf);
    } else {
        checked_snprintf(out, cap, "%s/%s", base, leaf);
    }
}

static bool file_exists(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }
    fclose(fp);
    return true;
}

static void normalize_import_leaf(char *out, size_t cap, const char *leaf) {
    const char *start = leaf;
    size_t len = strlen(leaf);
    while (len > 0 && *start == '"') {
        start++;
        len--;
    }
    while (len > 0 && start[len - 1] == '"') {
        len--;
    }
    if (len + 1 > cap) {
        fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("path_too_long"));
        exit(1);
    }
    memcpy(out, start, len);
    out[len] = '\0';
}

static void resolve_import_path(char *out, size_t cap, const char *base, const char *leaf) {
    char clean_leaf[2048];
    char candidate[2048];
    char legacy_candidate[2048];
    normalize_import_leaf(clean_leaf, sizeof(clean_leaf), leaf);
    join_fs_path(candidate, sizeof(candidate), base, clean_leaf);
    if (file_exists(candidate)) {
        checked_snprintf(out, cap, "%s", candidate);
        return;
    }
    if (strlen(clean_leaf) > 6 && strcmp(clean_leaf + strlen(clean_leaf) - 6, ".sharp") == 0) {
        checked_snprintf(legacy_candidate, sizeof(legacy_candidate), "%.*s.sp", (int) (strlen(clean_leaf) - 6), clean_leaf);
        join_fs_path(candidate, sizeof(candidate), base, legacy_candidate);
        if (file_exists(candidate)) {
            checked_snprintf(out, cap, "%s", candidate);
            return;
        }
    }

    if (g_tool_dir[0] &&
        (strncmp(clean_leaf, "std/", 4) == 0 || strncmp(clean_leaf, "std\\", 4) == 0)) {
        checked_snprintf(candidate, sizeof(candidate), "%s\\%s", g_tool_dir, clean_leaf);
        if (file_exists(candidate)) {
            checked_snprintf(out, cap, "%s", candidate);
            return;
        }
        if (strlen(clean_leaf) > 6 && strcmp(clean_leaf + strlen(clean_leaf) - 6, ".sharp") == 0) {
            checked_snprintf(legacy_candidate, sizeof(legacy_candidate), "%.*s.sp", (int) (strlen(clean_leaf) - 6), clean_leaf);
            checked_snprintf(candidate, sizeof(candidate), "%s\\%s", g_tool_dir, legacy_candidate);
            if (file_exists(candidate)) {
                checked_snprintf(out, cap, "%s", candidate);
                return;
            }
        }
    }

    join_fs_path(candidate, sizeof(candidate), base, clean_leaf);
    checked_snprintf(out, cap, "%s", candidate);
}

static void executable_dir(const char *argv0, char *out, size_t cap) {
    char candidate[2048];
    if (!_fullpath(out, argv0, cap)) {
        fprintf(stderr, "spearc error: cannot resolve compiler path\n");
        exit(1);
    }
    char *slash = strrchr(out, '\\');
    if (!slash) slash = strrchr(out, '/');
    if (!slash) {
        fprintf(stderr, "spearc error: cannot resolve compiler directory\n");
        exit(1);
    }
    *slash = '\0';

    checked_snprintf(candidate, sizeof(candidate), "%s\\..\\runtime\\bridge_node.mjs", out);
    FILE *fp = fopen(candidate, "rb");
    if (fp) {
        fclose(fp);
        char *parent = strrchr(out, '\\');
        if (!parent) parent = strrchr(out, '/');
        if (parent) {
            *parent = '\0';
        }
    }
}

static char *escape_c_string(const char *src) {
    Buffer out;
    buf_init(&out);
    for (size_t i = 0; src[i]; i++) {
        char c = src[i];
        if (c == '\\') buf_append(&out, "\\\\");
        else if (c == '"') buf_append(&out, "\\\"");
        else {
            char tmp[2] = { c, '\0' };
            buf_append(&out, tmp);
        }
    }
    return buf_take(&out);
}

static char *read_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("cannot_open"), path);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *data = xmalloc((size_t) size + 1);
    if (fread(data, 1, (size_t) size, fp) != (size_t) size) {
        fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("failed_read"), path);
        fclose(fp);
        exit(1);
    }
    data[size] = '\0';
    fclose(fp);
    if ((size_t) size >= 3 &&
        (unsigned char) data[0] == 0xEF &&
        (unsigned char) data[1] == 0xBB &&
        (unsigned char) data[2] == 0xBF) {
        memmove(data, data + 3, (size_t) size - 2);
    }
    return data;
}

static char *read_stream(FILE *fp, const char *label) {
    size_t cap = 4096;
    size_t len = 0;
    char *data = xmalloc(cap);
    for (;;) {
        size_t remaining = cap - len;
        size_t read = fread(data + len, 1, remaining - 1, fp);
        len += read;
        if (feof(fp)) {
            break;
        }
        if (ferror(fp)) {
            fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("failed_read"), label);
            free(data);
            exit(1);
        }
        cap *= 2;
        data = realloc(data, cap);
        if (!data) {
            fprintf(stderr, "spearc error: out of memory\n");
            exit(1);
        }
    }
    data[len] = '\0';
    if (len >= 3 &&
        (unsigned char) data[0] == 0xEF &&
        (unsigned char) data[1] == 0xBB &&
        (unsigned char) data[2] == 0xBF) {
        memmove(data, data + 3, len - 2);
    }
    return data;
}

static void write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("cannot_write"), path);
        exit(1);
    }
    size_t len = strlen(content);
    if (fwrite(content, 1, len, fp) != len) {
        fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("failed_write"), path);
        fclose(fp);
        exit(1);
    }
    fclose(fp);
}

static char *default_output_path(const char *input_path) {
    size_t len = strlen(input_path);
    Buffer out;
    buf_init(&out);
    if (len > 6 && strcmp(input_path + len - 6, ".sharp") == 0) {
        char *base = slice_dup(input_path, len - 6);
        buf_appendf(&out, "%s.c", base);
        free(base);
    } else if (len > 3 && strcmp(input_path + len - 3, ".sp") == 0) {
        char *base = slice_dup(input_path, len - 3);
        buf_appendf(&out, "%s.c", base);
        free(base);
    } else {
        buf_appendf(&out, "%s.c", input_path);
    }
    return buf_take(&out);
}

#endif

