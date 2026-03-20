#ifndef SPEARC_EMIT_H
#define SPEARC_EMIT_H

static void emit_indent(Buffer *out, int depth) {
    for (int i = 0; i < depth; i++) {
        buf_append(out, "    ");
    }
}

static void emit_line(Parser *parser, const char *fmt, ...) {
    va_list args;
    emit_indent(&parser->out, parser->depth);
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
    buf_reserve(&parser->out, (size_t) needed + 1);
    vsnprintf(parser->out.data + parser->out.len, parser->out.cap - parser->out.len, fmt, args);
    va_end(args);
    parser->out.len += (size_t) needed;
    parser->out.data[parser->out.len] = '\0';
    buf_append(&parser->out, "\n");
}

static void emit_function_prototype(Buffer *out, FunctionInfo *fn) {
    if (fn->is_entry) {
        buf_appendf(out, "int %s(void);\n", fn->c_name);
        return;
    }
    buf_appendf(out, "%s %s(SpearScope *ret_scope", ctype_name(fn->return_type), fn->c_name);
    for (size_t i = 0; i < fn->param_count; i++) {
        buf_appendf(out, ", %s %s", ctype_name(fn->params[i].type), fn->params[i].name);
    }
    buf_append(out, ");\n");
}

#endif
