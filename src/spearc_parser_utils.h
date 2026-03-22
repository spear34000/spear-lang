#ifndef SPEARC_PARSER_UTILS_H
#define SPEARC_PARSER_UTILS_H

static void parser_init(Parser *parser, const char *src) {
    lexer_init(&parser->lexer, src);
    buf_init(&parser->out);
    buf_init(&parser->bodies);
    parser->symbols = NULL;
    parser->symbol_count = 0;
    parser->symbol_cap = 0;
    parser->functions = NULL;
    parser->function_count = 0;
    parser->function_cap = 0;
    parser->depth = 0;
    parser->scope_counter = 0;
    parser->temp_counter = 0;
    parser->current_return_type = TYPE_NUM;
    parser->current_is_entry = false;
    parser->loop_depth = 0;
    parser->active_scope_count = 0;
    parser->defer_list_count = 0;
    parser->in_defer_capture = false;
    memset(parser->defer_lists, 0, sizeof(parser->defer_lists));
    parser->sharp_type_count = 0;
    memset(parser->sharp_types, 0, sizeof(parser->sharp_types));
    parser->current_package_name = xstrdup("");
    parser->current_module_name = xstrdup("");
    parser->lexer.current = lexer_next(&parser->lexer);
}

static void advance(Parser *parser) {
    parser->lexer.current = lexer_next(&parser->lexer);
}

static bool match(Parser *parser, TokenKind kind) {
    if (parser->lexer.current.kind != kind) {
        return false;
    }
    advance(parser);
    return true;
}

static void expect(Parser *parser, TokenKind kind, const char *message) {
    if (parser->lexer.current.kind != kind) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "%s", message);
    }
    advance(parser);
}

static bool is_top_level_starter(TokenKind kind) {
    return kind == TOK_FUNCTION ||
        kind == TOK_RUN ||
        kind == TOK_SPEAR ||
        kind == TOK_NUM ||
        kind == TOK_TEXT ||
        kind == TOK_VIEW ||
        kind == TOK_MODULE ||
        kind == TOK_PACKAGE ||
        kind == TOK_CLASS;
}

static void recover_statement(Parser *parser) {
    int brace_depth = 0;
    while (parser->lexer.current.kind != TOK_EOF) {
        if (brace_depth == 0 && (parser->lexer.current.kind == TOK_SEMI || parser->lexer.current.kind == TOK_RBRACE)) {
            if (parser->lexer.current.kind == TOK_SEMI) {
                advance(parser);
            }
            return;
        }
        if (parser->lexer.current.kind == TOK_LBRACE) {
            brace_depth++;
        } else if (parser->lexer.current.kind == TOK_RBRACE) {
            if (brace_depth > 0) {
                brace_depth--;
            } else {
                return;
            }
        }
        advance(parser);
    }
}

static void recover_top_level(Parser *parser) {
    int brace_depth = 0;
    while (parser->lexer.current.kind != TOK_EOF) {
        if (brace_depth == 0 && is_top_level_starter(parser->lexer.current.kind)) {
            return;
        }
        if (parser->lexer.current.kind == TOK_LBRACE) {
            brace_depth++;
        } else if (parser->lexer.current.kind == TOK_RBRACE && brace_depth > 0) {
            brace_depth--;
        }
        advance(parser);
    }
}

static bool parse_statement_with_recovery(Parser *parser, int scope_id, bool *terminated) {
    if (g_collect_errors) {
        jmp_buf env;
        jmp_buf *previous = g_recover_env;
        g_recover_env = &env;
        if (setjmp(env) == 0) {
            if (*terminated) {
                warning_at(parser->lexer.current.line, parser->lexer.current.col, compiler_message("unreachable_code"));
            }
            *terminated = parse_statement(parser, scope_id);
            g_recover_env = previous;
            return true;
        }
        g_recover_env = previous;
        *terminated = false;
        recover_statement(parser);
        return false;
    }
    if (*terminated) {
        warning_at(parser->lexer.current.line, parser->lexer.current.col, compiler_message("unreachable_code"));
    }
    *terminated = parse_statement(parser, scope_id);
    return true;
}

static Token peek_token(Parser *parser) {
    Lexer copy = parser->lexer;
    return lexer_next(&copy);
}

#endif
