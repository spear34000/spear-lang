#ifndef SPEARC_SCAN_H
#define SPEARC_SCAN_H

static void warn_constant_condition(Token token, const char *kind) {
    if (token.kind != TOK_NUMBER) {
        return;
    }
    warning_at(
        token.line,
        token.col,
        compiler_message(token.number ? "condition_always_true" : "condition_always_false"),
        kind
    );
}

static char *make_scope_name(int scope_id) {
    char temp[32];
    snprintf(temp, sizeof(temp), "_scope_%d", scope_id);
    return xstrdup(temp);
}

static char *new_temp(Parser *parser) {
    char temp[32];
    snprintf(temp, sizeof(temp), "_tmp_%d", parser->temp_counter++);
    return xstrdup(temp);
}

static void scan_skip_block(Lexer *lexer) {
    Token token = lexer_next(lexer);
    if (token.kind != TOK_LBRACE) {
        fatal_at(token.line, token.col, "expected '{' after function header");
    }
    int depth = 1;
    while (depth > 0) {
        token = lexer_next(lexer);
        if (token.kind == TOK_EOF) {
            fatal_at(token.line, token.col, "unterminated block");
        }
        if (token.kind == TOK_LBRACE) depth++;
        if (token.kind == TOK_RBRACE) depth--;
    }
}

static void scan_skip_definition_body(Lexer *lexer) {
    Token token;
    for (;;) {
        token = lexer_next(lexer);
        if (token.kind == TOK_EOF) {
            fatal_at(token.line, token.col, "unterminated block");
        }
        if (token.kind == TOK_LBRACE) {
            break;
        }
    }
    int depth = 1;
    while (depth > 0) {
        token = lexer_next(lexer);
        if (token.kind == TOK_EOF) {
            fatal_at(token.line, token.col, "unterminated block");
        }
        if (token.kind == TOK_LBRACE) depth++;
        if (token.kind == TOK_RBRACE) depth--;
    }
}

static void scan_skip_meta(Lexer *lexer, Token first) __attribute__((unused));
static void scan_skip_meta(Lexer *lexer, Token first) {
    Token next = lexer_next(lexer);
    if (next.kind != TOK_IDENT) {
        fatal_at(next.line, next.col, "expected name after %s", token_text(first));
    }
    Token semi = lexer_next(lexer);
    if (semi.kind != TOK_SEMI) {
        fatal_at(semi.line, semi.col, "expected ';' after metadata");
    }
}

#endif
