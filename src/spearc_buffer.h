#ifndef SPEARC_BUFFER_H
#define SPEARC_BUFFER_H

static void buf_init(Buffer *buf) {
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

static void buf_reserve(Buffer *buf, size_t extra) {
    size_t need = buf->len + extra + 1;
    if (need <= buf->cap) {
        return;
    }
    size_t next = buf->cap ? buf->cap * 2 : 256;
    while (next < need) {
        next *= 2;
    }
    buf->data = realloc(buf->data, next);
    if (!buf->data) {
        fprintf(stderr, "spearc error: out of memory\n");
        exit(1);
    }
    buf->cap = next;
}

static void buf_append(Buffer *buf, const char *text) {
    size_t len = strlen(text);
    buf_reserve(buf, len);
    memcpy(buf->data + buf->len, text, len);
    buf->len += len;
    buf->data[buf->len] = '\0';
}

static void buf_appendf(Buffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list copy;
    va_copy(copy, args);
    int needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0) {
        va_end(args);
        fprintf(stderr, "spearc error: formatting error\n");
        exit(1);
    }
    buf_reserve(buf, (size_t) needed);
    vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, args);
    va_end(args);
    buf->len += (size_t) needed;
}

static char *buf_take(Buffer *buf) {
    if (!buf->data) {
        return xstrdup("");
    }
    char *result = buf->data;
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
    return result;
}

#endif
