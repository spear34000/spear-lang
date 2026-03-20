#ifndef SPEARC_SYMBOLS_H
#define SPEARC_SYMBOLS_H

static void add_symbol(Parser *parser, const char *name, ValueType type, bool is_const, int line, int col, bool used, bool is_parameter) {
    for (size_t i = parser->symbol_count; i > 0; i--) {
        Symbol *sym = &parser->symbols[i - 1];
        if (sym->depth == parser->depth && strcmp(sym->name, name) == 0) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "redeclaration of '%s'", name);
        }
    }

    if (parser->symbol_count == parser->symbol_cap) {
        size_t next = parser->symbol_cap ? parser->symbol_cap * 2 : 32;
        parser->symbols = realloc(parser->symbols, next * sizeof(Symbol));
        if (!parser->symbols) {
            fprintf(stderr, "spearc error: out of memory\n");
            exit(1);
        }
        parser->symbol_cap = next;
    }

    parser->symbols[parser->symbol_count].name = xstrdup(name);
    parser->symbols[parser->symbol_count].type = type;
    parser->symbols[parser->symbol_count].depth = parser->depth;
    parser->symbols[parser->symbol_count].is_const = is_const;
    parser->symbols[parser->symbol_count].used = used;
    parser->symbols[parser->symbol_count].is_parameter = is_parameter;
    parser->symbols[parser->symbol_count].line = line;
    parser->symbols[parser->symbol_count].col = col;
    parser->symbol_count++;
}

static ValueType lookup_symbol(Parser *parser, const char *name) {
    for (size_t i = parser->symbol_count; i > 0; i--) {
        Symbol *sym = &parser->symbols[i - 1];
        if (strcmp(sym->name, name) == 0) {
            sym->used = true;
            return sym->type;
        }
    }
    fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unknown variable '%s'", name);
    return TYPE_NUM;
}

static bool symbol_is_const(Parser *parser, const char *name) {
    for (size_t i = parser->symbol_count; i > 0; i--) {
        Symbol *sym = &parser->symbols[i - 1];
        if (strcmp(sym->name, name) == 0) {
            return sym->is_const;
        }
    }
    fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unknown variable '%s'", name);
    return false;
}

static void pop_symbols(Parser *parser, int depth) {
    while (parser->symbol_count > 0 && parser->symbols[parser->symbol_count - 1].depth >= depth) {
        Symbol *sym = &parser->symbols[parser->symbol_count - 1];
        if (!sym->used && sym->line > 0 && sym->name[0] != '_') {
            warning_at(sym->line, sym->col, compiler_message(sym->is_parameter ? "unused_parameter" : "unused_variable"), sym->name);
        }
        free(parser->symbols[parser->symbol_count - 1].name);
        parser->symbol_count--;
    }
}

static void add_function_info(Parser *parser, FunctionInfo info) {
    for (size_t i = 0; i < parser->function_count; i++) {
        if (strcmp(parser->functions[i].name, info.name) == 0 &&
            strcmp(parser->functions[i].package_name, info.package_name) == 0 &&
            strcmp(parser->functions[i].module_name, info.module_name) == 0) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "duplicate function '%s'", info.name);
        }
    }

    if (parser->function_count == parser->function_cap) {
        size_t next = parser->function_cap ? parser->function_cap * 2 : 16;
        parser->functions = realloc(parser->functions, next * sizeof(FunctionInfo));
        if (!parser->functions) {
            fprintf(stderr, "spearc error: out of memory\n");
            exit(1);
        }
        parser->function_cap = next;
    }

    parser->functions[parser->function_count++] = info;
}

static FunctionInfo *find_function(Parser *parser, const char *name) {
    for (size_t i = parser->function_count; i > 0; i--) {
        if (strcmp(parser->functions[i - 1].name, name) == 0) {
            return &parser->functions[i - 1];
        }
    }
    return NULL;
}

static FunctionInfo *find_function_exact(Parser *parser, const char *name, const char *package_name, const char *module_name) {
    for (size_t i = parser->function_count; i > 0; i--) {
        FunctionInfo *fn = &parser->functions[i - 1];
        if (strcmp(fn->name, name) == 0 &&
            strcmp(fn->package_name, package_name ? package_name : "") == 0 &&
            strcmp(fn->module_name, module_name ? module_name : "") == 0) {
            return fn;
        }
    }
    return NULL;
}

static void mark_function_used(FunctionInfo *fn) {
    if (fn) {
        fn->used = true;
    }
}

#endif
