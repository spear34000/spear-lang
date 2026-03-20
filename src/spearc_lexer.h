#ifndef SPEARC_LEXER_H
#define SPEARC_LEXER_H

static bool is_ident_start(char c) {
    return isalpha((unsigned char) c) || c == '_';
}

static bool is_ident_char(char c) {
    return isalnum((unsigned char) c) || c == '_';
}

static void lexer_init(Lexer *lexer, const char *src) {
    lexer->src = src;
    lexer->len = strlen(src);
    lexer->pos = (lexer->len >= 3 &&
                  (unsigned char) src[0] == 0xEF &&
                  (unsigned char) src[1] == 0xBB &&
                  (unsigned char) src[2] == 0xBF) ? 3 : 0;
    lexer->line = 1;
    lexer->col = 1;
}

static char lexer_peek(Lexer *lexer) {
    if (lexer->pos >= lexer->len) {
        return '\0';
    }
    return lexer->src[lexer->pos];
}

static char lexer_peek_next(Lexer *lexer) {
    if (lexer->pos + 1 >= lexer->len) {
        return '\0';
    }
    return lexer->src[lexer->pos + 1];
}

static char lexer_advance(Lexer *lexer) {
    char c = lexer_peek(lexer);
    if (c == '\0') {
        return c;
    }
    lexer->pos++;
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static void lexer_skip_ws(Lexer *lexer) {
    for (;;) {
        char c = lexer_peek(lexer);
        if ((unsigned char) c == 0x1F) {
            lexer_advance(lexer);
            int next_line = 0;
            while (isdigit((unsigned char) lexer_peek(lexer))) {
                next_line = next_line * 10 + (lexer_advance(lexer) - '0');
            }
            if (lexer_peek(lexer) == '\r') {
                lexer_advance(lexer);
            }
            if (lexer_peek(lexer) == '\n') {
                lexer_advance(lexer);
            }
            lexer->line = next_line > 0 ? next_line : 1;
            lexer->col = 1;
            continue;
        }
        if (isspace((unsigned char) c)) {
            lexer_advance(lexer);
            continue;
        }
        if (c == '/' && lexer_peek_next(lexer) == '/') {
            while (lexer_peek(lexer) && lexer_peek(lexer) != '\n') {
                lexer_advance(lexer);
            }
            continue;
        }
        break;
    }
}

static TokenKind keyword_kind(const char *text, size_t len) {
    if (len == 5 && strncmp(text, "spear", len) == 0) return TOK_SPEAR;
    if (len == 5 && strncmp(text, "sharp", len) == 0) return TOK_SHARP;
    if (len == 4 && strncmp(text, "keep", len) == 0) return TOK_KEEP;
    if (len == 5 && strncmp(text, "defer", len) == 0) return TOK_DEFER;
    if (len == 3 && strncmp(text, "num", len) == 0) return TOK_NUM;
    if (len == 6 && strncmp(text, "number", len) == 0) return TOK_NUM;
    if (len == 4 && strncmp(text, "text", len) == 0) return TOK_TEXT;
    if (len == 3 && strncmp(text, "str", len) == 0) return TOK_TEXT;
    if (len == 6 && strncmp(text, "string", len) == 0) return TOK_TEXT;
    if (len == 7 && strncmp(text, "numlist", len) == 0) return TOK_NUMLIST;
    if (len == 4 && strncmp(text, "nums", len) == 0) return TOK_NUMLIST;
    if (len == 7 && strncmp(text, "numbers", len) == 0) return TOK_NUMLIST;
    if (len == 8 && strncmp(text, "textlist", len) == 0) return TOK_TEXTLIST;
    if (len == 5 && strncmp(text, "texts", len) == 0) return TOK_TEXTLIST;
    if (len == 7 && strncmp(text, "strings", len) == 0) return TOK_TEXTLIST;
    if (len == 3 && strncmp(text, "map", len) == 0) return TOK_MAP;
    if (len == 6 && strncmp(text, "result", len) == 0) return TOK_RESULT;
    if (len == 3 && strncmp(text, "let", len) == 0) return TOK_LET;
    if (len == 3 && strncmp(text, "val", len) == 0) return TOK_LET;
    if (len == 3 && strncmp(text, "var", len) == 0) return TOK_VAR;
    if (len == 3 && strncmp(text, "mut", len) == 0) return TOK_VAR;
    if (len == 5 && strncmp(text, "const", len) == 0) return TOK_CONST;
    if (len == 4 && strncmp(text, "view", len) == 0) return TOK_VIEW;
    if (len == 8 && strncmp(text, "function", len) == 0) return TOK_FUNCTION;
    if (len == 2 && strncmp(text, "fn", len) == 0) return TOK_FUNCTION;
    if (len == 3 && strncmp(text, "say", len) == 0) return TOK_SAY;
    if (len == 5 && strncmp(text, "print", len) == 0) return TOK_SAY;
    if (len == 4 && strncmp(text, "show", len) == 0) return TOK_SAY;
    if (len == 5 && strncmp(text, "write", len) == 0) return TOK_WRITE;
    if (len == 2 && strncmp(text, "if", len) == 0) return TOK_IF;
    if (len == 4 && strncmp(text, "else", len) == 0) return TOK_ELSE;
    if (len == 3 && strncmp(text, "for", len) == 0) return TOK_FOR;
    if (len == 5 && strncmp(text, "while", len) == 0) return TOK_WHILE;
    if (len == 5 && strncmp(text, "break", len) == 0) return TOK_BREAK;
    if (len == 8 && strncmp(text, "continue", len) == 0) return TOK_CONTINUE;
    if (len == 4 && strncmp(text, "each", len) == 0) return TOK_EACH;
    if (len == 2 && strncmp(text, "in", len) == 0) return TOK_IN;
    if (len == 6 && strncmp(text, "return", len) == 0) return TOK_RETURN;
    if (len == 6 && strncmp(text, "import", len) == 0) return TOK_IMPORT;
    if (len == 6 && strncmp(text, "module", len) == 0) return TOK_MODULE;
    if (len == 7 && strncmp(text, "package", len) == 0) return TOK_PACKAGE;
    if (len == 3 && strncmp(text, "try", len) == 0) return TOK_TRY;
    if (len == 5 && strncmp(text, "catch", len) == 0) return TOK_CATCH;
    if (len == 5 && strncmp(text, "throw", len) == 0) return TOK_THROW;
    if (len == 5 && strncmp(text, "class", len) == 0) return TOK_CLASS;
    if (len == 8 && strncmp(text, "nodecall", len) == 0) return TOK_NODECALL;
    if (len == 6 && strncmp(text, "pycall", len) == 0) return TOK_PYCALL;
    if (len == 4 && strncmp(text, "join", len) == 0) return TOK_JOIN;
    if (len == 4 && strncmp(text, "read", len) == 0) return TOK_READ;
    if (len == 4 && strncmp(text, "size", len) == 0) return TOK_SIZE;
    if (len == 4 && strncmp(text, "same", len) == 0) return TOK_SAME;
    if (len == 4 && strncmp(text, "pack", len) == 0) return TOK_PACK;
    if (len == 4 && strncmp(text, "push", len) == 0) return TOK_PUSH;
    if (len == 5 && strncmp(text, "count", len) == 0) return TOK_COUNT;
    if (len == 2 && strncmp(text, "at", len) == 0) return TOK_AT;
    if (len == 9 && strncmp(text, "arg_count", len) == 0) return TOK_ARGCOUNT;
    if (len == 6 && strncmp(text, "arg_at", len) == 0) return TOK_ARGAT;
    if (len == 3 && strncmp(text, "put", len) == 0) return TOK_PUT;
    if (len == 3 && strncmp(text, "get", len) == 0) return TOK_GET;
    if (len == 3 && strncmp(text, "has", len) == 0) return TOK_HAS;
    if (len == 4 && strncmp(text, "drop", len) == 0) return TOK_DROP;
    if (len == 2 && strncmp(text, "ok", len) == 0) return TOK_OK;
    if (len == 4 && strncmp(text, "fail", len) == 0) return TOK_FAIL;
    if (len == 5 && strncmp(text, "is_ok", len) == 0) return TOK_ISOK;
    if (len == 6 && strncmp(text, "unwrap", len) == 0) return TOK_UNWRAP;
    if (len == 10 && strncmp(text, "error_text", len) == 0) return TOK_ERRORTEXT;
    if (len == 5 && strncmp(text, "guard", len) == 0) return TOK_GUARD;
    if (len == 6 && strncmp(text, "escape", len) == 0) return TOK_ESCAPE;
    if (len == 6 && strncmp(text, "markup", len) == 0) return TOK_MARKUP;
    if (len == 4 && strncmp(text, "page", len) == 0) return TOK_PAGE;
    if (len == 5 && strncmp(text, "stack", len) == 0) return TOK_STACK;
    if (len == 6 && strncmp(text, "inline", len) == 0) return TOK_INLINE;
    if (len == 6 && strncmp(text, "column", len) == 0) return TOK_COLUMN;
    if (len == 3 && strncmp(text, "row", len) == 0) return TOK_ROW;
    if (len == 3 && strncmp(text, "box", len) == 0) return TOK_BOX;
    if (len == 11 && strncmp(text, "surface_box", len) == 0) return TOK_SURFACEBOX;
    if (len == 10 && strncmp(text, "column_box", len) == 0) return TOK_COLUMNBOX;
    if (len == 7 && strncmp(text, "row_box", len) == 0) return TOK_ROWBOX;
    if (len == 6 && strncmp(text, "action", len) == 0) return TOK_ACTION;
    if (len == 4 && strncmp(text, "attr", len) == 0) return TOK_ATTR;
    if (len == 5 && strncmp(text, "attrs", len) == 0) return TOK_ATTRS;
    if (len == 10 && strncmp(text, "class_name", len) == 0) return TOK_CLASSNAME;
    if (len == 7 && strncmp(text, "id_name", len) == 0) return TOK_IDNAME;
    if (len == 10 && strncmp(text, "style_attr", len) == 0) return TOK_STYLEATTR;
    if (len == 9 && strncmp(text, "href_attr", len) == 0) return TOK_HREFATTR;
    if (len == 8 && strncmp(text, "src_attr", len) == 0) return TOK_SRCATTR;
    if (len == 8 && strncmp(text, "alt_attr", len) == 0) return TOK_ALTATTR;
    if (len == 3 && strncmp(text, "ask", len) == 0) return TOK_ASK;
    if (len == 3 && strncmp(text, "run", len) == 0) return TOK_RUN;
    if (len == 3 && strncmp(text, "app", len) == 0) return TOK_RUN;
    return TOK_IDENT;
}

static Token lexer_next(Lexer *lexer) {
    lexer_skip_ws(lexer);

    Token token;
    token.start = lexer->src + lexer->pos;
    token.length = 1;
    token.number = 0;
    token.string_value = NULL;
    token.line = lexer->line;
    token.col = lexer->col;

    char c = lexer_advance(lexer);
    if (c == '\0') {
        token.kind = TOK_EOF;
        token.length = 0;
        return token;
    }

    switch (c) {
        case '{': token.kind = TOK_LBRACE; return token;
        case '}': token.kind = TOK_RBRACE; return token;
        case '(': token.kind = TOK_LPAREN; return token;
        case ')': token.kind = TOK_RPAREN; return token;
        case ';': token.kind = TOK_SEMI; return token;
        case ',': token.kind = TOK_COMMA; return token;
        case '+': token.kind = TOK_PLUS; return token;
        case '-': token.kind = TOK_MINUS; return token;
        case '*': token.kind = TOK_STAR; return token;
        case '/': token.kind = TOK_SLASH; return token;
        case '%': token.kind = TOK_PERCENT; return token;
        case '=':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                token.kind = TOK_EQEQ;
                token.length = 2;
            } else {
                token.kind = TOK_ASSIGN;
            }
            return token;
        case '!':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                token.kind = TOK_NEQ;
                token.length = 2;
                return token;
            }
            break;
        case '<':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                token.kind = TOK_LE;
                token.length = 2;
            } else {
                token.kind = TOK_LT;
            }
            return token;
        case '>':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                token.kind = TOK_GE;
                token.length = 2;
            } else {
                token.kind = TOK_GT;
            }
            return token;
        case '"': {
            Buffer text;
            buf_init(&text);
            while (lexer_peek(lexer) && lexer_peek(lexer) != '"') {
                char ch = lexer_advance(lexer);
                if (ch == '\\') {
                    char esc = lexer_advance(lexer);
                    switch (esc) {
                        case 'n': buf_append(&text, "\\n"); break;
                        case 't': buf_append(&text, "\\t"); break;
                        case '\\': buf_append(&text, "\\\\"); break;
                        case '"': buf_append(&text, "\\\""); break;
                        default:
                            fatal_at(lexer->line, lexer->col, "unsupported escape sequence");
                    }
                } else {
                    char tmp[2] = { ch, '\0' };
                    buf_append(&text, tmp);
                }
            }
            if (lexer_peek(lexer) != '"') {
                fatal_at(token.line, token.col, "unterminated string");
            }
            lexer_advance(lexer);
            token.kind = TOK_STRING;
            token.length = (size_t) ((lexer->src + lexer->pos) - token.start);
            token.string_value = buf_take(&text);
            return token;
        }
        default:
            break;
    }

    if (isdigit((unsigned char) c)) {
        while (isdigit((unsigned char) lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
        token.kind = TOK_NUMBER;
        token.length = (size_t) ((lexer->src + lexer->pos) - token.start);
        token.number = strtoll(token.start, NULL, 10);
        return token;
    }

    if (is_ident_start(c)) {
        while (is_ident_char(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
        token.length = (size_t) ((lexer->src + lexer->pos) - token.start);
        token.kind = keyword_kind(token.start, token.length);
        return token;
    }

    fatal_at(token.line, token.col, "unexpected character '%c'", c);
    token.kind = TOK_EOF;
    return token;
}

#endif
