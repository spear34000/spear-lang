#ifndef SPEARC_UTIL_H
#define SPEARC_UTIL_H

static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("oom"));
        exit(1);
    }
    return ptr;
}

static char *xstrdup(const char *src) {
    size_t len = strlen(src);
    char *copy = xmalloc(len + 1);
    memcpy(copy, src, len + 1);
    return copy;
}

static void checked_snprintf(char *out, size_t cap, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(out, cap, fmt, args);
    va_end(args);
    if (written < 0 || (size_t) written >= cap) {
        fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("path_too_long"));
        exit(1);
    }
}

static char *slice_dup(const char *start, size_t len) {
    char *copy = xmalloc(len + 1);
    memcpy(copy, start, len);
    copy[len] = '\0';
    return copy;
}

#endif
