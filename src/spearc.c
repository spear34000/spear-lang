#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Buffer;

typedef enum {
    TYPE_NUM,
    TYPE_TEXT,
    TYPE_NUMLIST,
    TYPE_TEXTLIST
} ValueType;

typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_SEMI,
    TOK_COMMA,
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_EQEQ,
    TOK_NEQ,
    TOK_SPEAR,
    TOK_SHARP,
    TOK_NUM,
    TOK_TEXT,
    TOK_NUMLIST,
    TOK_TEXTLIST,
    TOK_LET,
    TOK_CONST,
    TOK_VIEW,
    TOK_SAY,
    TOK_WRITE,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_EACH,
    TOK_IN,
    TOK_RETURN,
    TOK_JOIN,
    TOK_READ,
    TOK_SIZE,
    TOK_SAME,
    TOK_PACK,
    TOK_PUSH,
    TOK_COUNT,
    TOK_AT,
    TOK_GUARD,
    TOK_ESCAPE,
    TOK_MARKUP,
    TOK_PAGE,
    TOK_STACK,
    TOK_INLINE,
    TOK_ACTION
} TokenKind;

typedef struct {
    TokenKind kind;
    const char *start;
    size_t length;
    long long number;
    char *string_value;
    int line;
    int col;
} Token;

typedef struct {
    const char *src;
    size_t len;
    size_t pos;
    int line;
    int col;
    Token current;
} Lexer;

typedef struct {
    char *name;
    ValueType type;
    int depth;
    bool is_const;
} Symbol;

typedef struct {
    char *code;
    ValueType type;
} Expr;

typedef struct {
    char *name;
    ValueType type;
} Param;

typedef struct {
    char *name;
    ValueType return_type;
    bool is_entry;
    Param *params;
    size_t param_count;
} FunctionInfo;

typedef struct {
    Lexer lexer;
    Buffer out;
    Buffer bodies;
    Symbol *symbols;
    size_t symbol_count;
    size_t symbol_cap;
    FunctionInfo *functions;
    size_t function_count;
    size_t function_cap;
    int depth;
    int scope_counter;
    int temp_counter;
    ValueType current_return_type;
    bool current_is_entry;
    int active_scope_ids[128];
    int active_scope_count;
} Parser;

static void fatal_at(int line, int col, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "[line %d:%d] ", line, col);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    exit(1);
}

static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "out of memory\n");
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

static char *slice_dup(const char *start, size_t len) {
    char *copy = xmalloc(len + 1);
    memcpy(copy, start, len);
    copy[len] = '\0';
    return copy;
}

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
        fprintf(stderr, "out of memory\n");
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
        fprintf(stderr, "formatting error\n");
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

static bool is_ident_start(char c) {
    return isalpha((unsigned char) c) || c == '_';
}

static bool is_ident_char(char c) {
    return isalnum((unsigned char) c) || c == '_';
}

static void lexer_init(Lexer *lexer, const char *src) {
    lexer->src = src;
    lexer->len = strlen(src);
    lexer->pos = 0;
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
    if (len == 3 && strncmp(text, "num", len) == 0) return TOK_NUM;
    if (len == 4 && strncmp(text, "text", len) == 0) return TOK_TEXT;
    if (len == 7 && strncmp(text, "numlist", len) == 0) return TOK_NUMLIST;
    if (len == 8 && strncmp(text, "textlist", len) == 0) return TOK_TEXTLIST;
    if (len == 3 && strncmp(text, "let", len) == 0) return TOK_LET;
    if (len == 5 && strncmp(text, "const", len) == 0) return TOK_CONST;
    if (len == 4 && strncmp(text, "view", len) == 0) return TOK_VIEW;
    if (len == 3 && strncmp(text, "say", len) == 0) return TOK_SAY;
    if (len == 5 && strncmp(text, "write", len) == 0) return TOK_WRITE;
    if (len == 2 && strncmp(text, "if", len) == 0) return TOK_IF;
    if (len == 4 && strncmp(text, "else", len) == 0) return TOK_ELSE;
    if (len == 5 && strncmp(text, "while", len) == 0) return TOK_WHILE;
    if (len == 4 && strncmp(text, "each", len) == 0) return TOK_EACH;
    if (len == 2 && strncmp(text, "in", len) == 0) return TOK_IN;
    if (len == 6 && strncmp(text, "return", len) == 0) return TOK_RETURN;
    if (len == 4 && strncmp(text, "join", len) == 0) return TOK_JOIN;
    if (len == 4 && strncmp(text, "read", len) == 0) return TOK_READ;
    if (len == 4 && strncmp(text, "size", len) == 0) return TOK_SIZE;
    if (len == 4 && strncmp(text, "same", len) == 0) return TOK_SAME;
    if (len == 4 && strncmp(text, "pack", len) == 0) return TOK_PACK;
    if (len == 4 && strncmp(text, "push", len) == 0) return TOK_PUSH;
    if (len == 5 && strncmp(text, "count", len) == 0) return TOK_COUNT;
    if (len == 2 && strncmp(text, "at", len) == 0) return TOK_AT;
    if (len == 5 && strncmp(text, "guard", len) == 0) return TOK_GUARD;
    if (len == 6 && strncmp(text, "escape", len) == 0) return TOK_ESCAPE;
    if (len == 6 && strncmp(text, "markup", len) == 0) return TOK_MARKUP;
    if (len == 4 && strncmp(text, "page", len) == 0) return TOK_PAGE;
    if (len == 5 && strncmp(text, "stack", len) == 0) return TOK_STACK;
    if (len == 6 && strncmp(text, "inline", len) == 0) return TOK_INLINE;
    if (len == 6 && strncmp(text, "action", len) == 0) return TOK_ACTION;
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
    switch (c) {
        case '\0': token.kind = TOK_EOF; token.length = 0; return token;
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
            fatal_at(token.line, token.col, "unexpected '!'");
            return token;
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
    parser->active_scope_count = 0;
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

static char *token_text(Token token) {
    return slice_dup(token.start, token.length);
}

static ValueType token_to_type(TokenKind kind) {
    switch (kind) {
        case TOK_NUM: return TYPE_NUM;
        case TOK_TEXT: return TYPE_TEXT;
        case TOK_NUMLIST: return TYPE_NUMLIST;
        case TOK_TEXTLIST: return TYPE_TEXTLIST;
        default:
            fprintf(stderr, "internal error: invalid type token\n");
            exit(1);
    }
}

static const char *ctype_name(ValueType type) {
    switch (type) {
        case TYPE_NUM: return "long long";
        case TYPE_TEXT: return "char *";
        case TYPE_NUMLIST: return "SpearNumList *";
        case TYPE_TEXTLIST: return "SpearTextList *";
        default: return "void *";
    }
}

static bool is_type_token(TokenKind kind) {
    return kind == TOK_NUM || kind == TOK_TEXT || kind == TOK_NUMLIST || kind == TOK_TEXTLIST;
}

static Token peek_token(Parser *parser) {
    Lexer copy = parser->lexer;
    return lexer_next(&copy);
}

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
        fprintf(stderr, "formatting error\n");
        exit(1);
    }
    buf_reserve(&parser->out, (size_t) needed + 1);
    vsnprintf(parser->out.data + parser->out.len, parser->out.cap - parser->out.len, fmt, args);
    va_end(args);
    parser->out.len += (size_t) needed;
    parser->out.data[parser->out.len] = '\0';
    buf_append(&parser->out, "\n");
}

static void add_symbol(Parser *parser, const char *name, ValueType type, bool is_const) {
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
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        parser->symbol_cap = next;
    }

    parser->symbols[parser->symbol_count].name = xstrdup(name);
    parser->symbols[parser->symbol_count].type = type;
    parser->symbols[parser->symbol_count].depth = parser->depth;
    parser->symbols[parser->symbol_count].is_const = is_const;
    parser->symbol_count++;
}

static ValueType lookup_symbol(Parser *parser, const char *name) {
    for (size_t i = parser->symbol_count; i > 0; i--) {
        Symbol *sym = &parser->symbols[i - 1];
        if (strcmp(sym->name, name) == 0) {
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
        free(parser->symbols[parser->symbol_count - 1].name);
        parser->symbol_count--;
    }
}

static void add_function_info(Parser *parser, FunctionInfo info) {
    for (size_t i = 0; i < parser->function_count; i++) {
        if (strcmp(parser->functions[i].name, info.name) == 0) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "duplicate function '%s'", info.name);
        }
    }

    if (parser->function_count == parser->function_cap) {
        size_t next = parser->function_cap ? parser->function_cap * 2 : 16;
        parser->functions = realloc(parser->functions, next * sizeof(FunctionInfo));
        if (!parser->functions) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        parser->function_cap = next;
    }

    parser->functions[parser->function_count++] = info;
}

static FunctionInfo *find_function(Parser *parser, const char *name) {
    for (size_t i = 0; i < parser->function_count; i++) {
        if (strcmp(parser->functions[i].name, name) == 0) {
            return &parser->functions[i];
        }
    }
    return NULL;
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

static void collect_functions(Parser *parser, const char *source) {
    Lexer lexer;
    lexer_init(&lexer, source);

    for (;;) {
        Token token = lexer_next(&lexer);
        if (token.kind == TOK_EOF) {
            return;
        }

        bool is_entry = false;
        ValueType return_type = TYPE_NUM;
        if (token.kind == TOK_SPEAR) {
            is_entry = true;
        } else if (token.kind == TOK_NUM || token.kind == TOK_TEXT || token.kind == TOK_VIEW) {
            return_type = token.kind == TOK_VIEW ? TYPE_TEXT : token_to_type(token.kind);
        } else {
            fatal_at(token.line, token.col, "top-level definition must start with spear, num, text, or view");
        }

        Token name_tok = lexer_next(&lexer);
        if (name_tok.kind != TOK_IDENT) {
            fatal_at(name_tok.line, name_tok.col, "expected function name");
        }

        Token open = lexer_next(&lexer);
        if (open.kind != TOK_LPAREN) {
            fatal_at(open.line, open.col, "expected '(' after function name");
        }

        Param *params = NULL;
        size_t param_count = 0;
        size_t param_cap = 0;

        Token next = lexer_next(&lexer);
        if (next.kind != TOK_RPAREN) {
            for (;;) {
                if (!is_type_token(next.kind)) {
                    fatal_at(next.line, next.col, "expected parameter type");
                }
                Token param_name = lexer_next(&lexer);
                if (param_name.kind != TOK_IDENT) {
                    fatal_at(param_name.line, param_name.col, "expected parameter name");
                }
                if (param_count == param_cap) {
                    size_t grow = param_cap ? param_cap * 2 : 4;
                    params = realloc(params, grow * sizeof(Param));
                    if (!params) {
                        fprintf(stderr, "out of memory\n");
                        exit(1);
                    }
                    param_cap = grow;
                }
                params[param_count].name = token_text(param_name);
                params[param_count].type = token_to_type(next.kind);
                param_count++;

                Token sep = lexer_next(&lexer);
                if (sep.kind == TOK_RPAREN) {
                    break;
                }
                if (sep.kind != TOK_COMMA) {
                    fatal_at(sep.line, sep.col, "expected ',' or ')'");
                }
                next = lexer_next(&lexer);
            }
        }

        FunctionInfo info;
        info.name = token_text(name_tok);
        info.return_type = return_type;
        info.is_entry = is_entry;
        info.params = params;
        info.param_count = param_count;
        add_function_info(parser, info);

        scan_skip_block(&lexer);
    }
}

static Expr make_expr(ValueType type, char *code) {
    Expr expr;
    expr.type = type;
    expr.code = code;
    return expr;
}

static Expr parse_num_expr(Parser *parser, int scope_id);
static Expr parse_text_expr(Parser *parser, int scope_id);
static Expr parse_list_expr(Parser *parser, int scope_id, ValueType expected_type);
static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope);
static bool starts_text_expr(Parser *parser);
static bool at_returns_text(Parser *parser);

static ValueType infer_expr_type(Parser *parser) {
    Token token = parser->lexer.current;
    if (token.kind == TOK_STRING ||
        token.kind == TOK_JOIN ||
        token.kind == TOK_READ ||
        token.kind == TOK_ESCAPE ||
        token.kind == TOK_MARKUP ||
        token.kind == TOK_PAGE ||
        token.kind == TOK_STACK ||
        token.kind == TOK_INLINE ||
        token.kind == TOK_ACTION) {
        return TYPE_TEXT;
    }
    if (token.kind == TOK_NUMBER || token.kind == TOK_SIZE || token.kind == TOK_SAME || token.kind == TOK_COUNT) {
        return TYPE_NUM;
    }
    if (token.kind == TOK_AT) {
        return at_returns_text(parser) ? TYPE_TEXT : TYPE_NUM;
    }
    if (token.kind == TOK_IDENT) {
        Token next = peek_token(parser);
        char *name = token_text(token);
        if (next.kind == TOK_LPAREN) {
            FunctionInfo *fn = find_function(parser, name);
            free(name);
            if (!fn) {
                fatal_at(token.line, token.col, "unknown function '%s'", token_text(token));
            }
            return fn->return_type;
        }
        ValueType type = lookup_symbol(parser, name);
        free(name);
        return type;
    }
    fatal_at(token.line, token.col, "cannot infer expression type");
    return TYPE_NUM;
}

static Expr parse_primary_num(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;
    if (match(parser, TOK_NUMBER)) {
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "%lld", token.number);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (parser->lexer.current.kind == TOK_IDENT) {
        Token next = peek_token(parser);
        if (next.kind == TOK_LPAREN) {
            char *fname = token_text(token);
            FunctionInfo *fn = find_function(parser, fname);
            if (!fn) {
                fatal_at(token.line, token.col, "unknown function '%s'", fname);
            }
            if (fn->return_type != TYPE_NUM) {
                fatal_at(token.line, token.col, "function '%s' does not return num", fname);
            }
            advance(parser);
            expect(parser, TOK_LPAREN, "expected '('");
            Buffer args;
            buf_init(&args);
            char *scope_name = make_scope_name(scope_id);
            buf_appendf(&args, "&%s", scope_name);
            free(scope_name);
            for (size_t i = 0; i < fn->param_count; i++) {
                if (i > 0) {
                    expect(parser, TOK_COMMA, "expected ','");
                }
                Expr arg;
                if (fn->params[i].type == TYPE_NUM) arg = parse_num_expr(parser, scope_id);
                else if (fn->params[i].type == TYPE_TEXT) arg = parse_text_expr(parser, scope_id);
                else arg = parse_list_expr(parser, scope_id, fn->params[i].type);
                buf_appendf(&args, ", %s", arg.code);
            }
            expect(parser, TOK_RPAREN, "expected ')'");
            Buffer code;
            buf_init(&code);
            buf_appendf(&code, "%s(%s)", fname, args.data ? args.data : "");
            return make_expr(TYPE_NUM, buf_take(&code));
        } else {
            match(parser, TOK_IDENT);
            char *name = token_text(token);
            ValueType type = lookup_symbol(parser, name);
            if (type != TYPE_NUM) {
                fatal_at(token.line, token.col, "expected num variable, got non-num '%s'", name);
            }
            return make_expr(TYPE_NUM, name);
        }
    }

    if (match(parser, TOK_LPAREN)) {
        Expr inner = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "(%s)", inner.code);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_SIZE)) {
        expect(parser, TOK_LPAREN, "expected '(' after size");
        Expr inner = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_text_size(%s)", inner.code);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_SAME)) {
        expect(parser, TOK_LPAREN, "expected '(' after same");
        Expr left = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in same");
        Expr right = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_text_same(%s, %s)", left.code, right.code);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_COUNT)) {
        expect(parser, TOK_LPAREN, "expected '(' after count");
        Token list_tok = parser->lexer.current;
        if (list_tok.kind != TOK_IDENT) {
            fatal_at(list_tok.line, list_tok.col, "expected list variable");
        }
        advance(parser);
        char *name = token_text(list_tok);
        ValueType type = lookup_symbol(parser, name);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        if (type == TYPE_NUMLIST) buf_appendf(&code, "spear_numlist_count(%s)", name);
        else if (type == TYPE_TEXTLIST) buf_appendf(&code, "spear_textlist_count(%s)", name);
        else fatal_at(list_tok.line, list_tok.col, "count expects a list");
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_AT)) {
        Token list_tok;
        expect(parser, TOK_LPAREN, "expected '(' after at");
        list_tok = parser->lexer.current;
        if (list_tok.kind != TOK_IDENT) {
            fatal_at(list_tok.line, list_tok.col, "expected list variable");
        }
        advance(parser);
        char *name = token_text(list_tok);
        ValueType type = lookup_symbol(parser, name);
        expect(parser, TOK_COMMA, "expected ',' in at");
        Expr index = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        if (type != TYPE_NUMLIST) {
            fatal_at(list_tok.line, list_tok.col, "at(list, idx) in numeric context requires numlist");
        }
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_numlist_at(%s, %s)", name, index.code);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected numeric expression");
    return make_expr(TYPE_NUM, xstrdup("0"));
}

static Expr parse_unary_num(Parser *parser, int scope_id) {
    if (match(parser, TOK_MINUS)) {
        Expr inner = parse_unary_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "(-%s)", inner.code);
        return make_expr(TYPE_NUM, buf_take(&code));
    }
    return parse_primary_num(parser, scope_id);
}

static Expr parse_mul_num(Parser *parser, int scope_id) {
    Expr left = parse_unary_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_STAR ||
           parser->lexer.current.kind == TOK_SLASH ||
           parser->lexer.current.kind == TOK_PERCENT) {
        TokenKind op = parser->lexer.current.kind;
        advance(parser);
        Expr right = parse_unary_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        const char *sym = op == TOK_STAR ? "*" : op == TOK_SLASH ? "/" : "%";
        buf_appendf(&code, "(%s %s %s)", left.code, sym, right.code);
        left = make_expr(TYPE_NUM, buf_take(&code));
    }
    return left;
}

static Expr parse_add_num(Parser *parser, int scope_id) {
    Expr left = parse_mul_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_PLUS ||
           parser->lexer.current.kind == TOK_MINUS) {
        TokenKind op = parser->lexer.current.kind;
        advance(parser);
        Expr right = parse_mul_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "(%s %c %s)", left.code, op == TOK_PLUS ? '+' : '-', right.code);
        left = make_expr(TYPE_NUM, buf_take(&code));
    }
    return left;
}

static Expr parse_cmp_num(Parser *parser, int scope_id) {
    Expr left = parse_add_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_LT ||
           parser->lexer.current.kind == TOK_GT ||
           parser->lexer.current.kind == TOK_LE ||
           parser->lexer.current.kind == TOK_GE) {
        TokenKind op = parser->lexer.current.kind;
        advance(parser);
        Expr right = parse_add_num(parser, scope_id);
        const char *sym = "<";
        if (op == TOK_GT) sym = ">";
        if (op == TOK_LE) sym = "<=";
        if (op == TOK_GE) sym = ">=";
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "(%s %s %s)", left.code, sym, right.code);
        left = make_expr(TYPE_NUM, buf_take(&code));
    }
    return left;
}

static Expr parse_eq_num(Parser *parser, int scope_id) {
    Expr left = parse_cmp_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_EQEQ ||
           parser->lexer.current.kind == TOK_NEQ) {
        TokenKind op = parser->lexer.current.kind;
        advance(parser);
        Expr right = parse_cmp_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "(%s %s %s)", left.code, op == TOK_EQEQ ? "==" : "!=", right.code);
        left = make_expr(TYPE_NUM, buf_take(&code));
    }
    return left;
}

static Expr parse_num_expr(Parser *parser, int scope_id) {
    return parse_eq_num(parser, scope_id);
}

static Expr parse_text_expr(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;

    if (match(parser, TOK_STRING)) {
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_text_clone(&%s, \"%s\")", scope_name, token.string_value);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (parser->lexer.current.kind == TOK_IDENT) {
        Token next = peek_token(parser);
        if (next.kind == TOK_LPAREN) {
            char *fname = token_text(token);
            FunctionInfo *fn = find_function(parser, fname);
            if (!fn) {
                fatal_at(token.line, token.col, "unknown function '%s'", fname);
            }
            if (fn->return_type != TYPE_TEXT) {
                fatal_at(token.line, token.col, "function '%s' does not return text", fname);
            }
            advance(parser);
            expect(parser, TOK_LPAREN, "expected '('");
            Buffer args;
            buf_init(&args);
            char *scope_name = make_scope_name(scope_id);
            buf_appendf(&args, "&%s", scope_name);
            free(scope_name);
            for (size_t i = 0; i < fn->param_count; i++) {
                if (i > 0) {
                    expect(parser, TOK_COMMA, "expected ','");
                }
                Expr arg;
                if (fn->params[i].type == TYPE_NUM) arg = parse_num_expr(parser, scope_id);
                else if (fn->params[i].type == TYPE_TEXT) arg = parse_text_expr(parser, scope_id);
                else arg = parse_list_expr(parser, scope_id, fn->params[i].type);
                buf_appendf(&args, ", %s", arg.code);
            }
            expect(parser, TOK_RPAREN, "expected ')'");
            Buffer code;
            buf_init(&code);
            buf_appendf(&code, "%s(%s)", fname, args.data ? args.data : "");
            return make_expr(TYPE_TEXT, buf_take(&code));
        } else {
            match(parser, TOK_IDENT);
            char *name = token_text(token);
            ValueType type = lookup_symbol(parser, name);
            if (type != TYPE_TEXT) {
                fatal_at(token.line, token.col, "expected text variable, got non-text '%s'", name);
            }
            return make_expr(TYPE_TEXT, name);
        }
    }

    if (match(parser, TOK_JOIN)) {
        expect(parser, TOK_LPAREN, "expected '(' after join");
        Expr left = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in join");
        Expr right = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')' after join");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_text_join(&%s, %s, %s)", scope_name, left.code, right.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_READ)) {
        expect(parser, TOK_LPAREN, "expected '(' after read");
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_text_read(&%s)", scope_name);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ESCAPE)) {
        expect(parser, TOK_LPAREN, "expected '(' after escape");
        Expr value = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_html_escape(&%s, %s)", scope_name, value.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_MARKUP)) {
        expect(parser, TOK_LPAREN, "expected '(' after markup");
        Expr tag = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in markup");
        Expr content = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_markup(&%s, %s, %s)", scope_name, tag.code, content.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_PAGE)) {
        expect(parser, TOK_LPAREN, "expected '(' after page");
        Expr title = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in page");
        Expr body = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_page(&%s, %s, %s)", scope_name, title.code, body.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_STACK)) {
        expect(parser, TOK_LPAREN, "expected '(' after stack");
        Expr first = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in stack");
        Expr second = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_stack(&%s, %s, %s)", scope_name, first.code, second.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_INLINE)) {
        expect(parser, TOK_LPAREN, "expected '(' after inline");
        Expr first = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in inline");
        Expr second = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_inline(&%s, %s, %s)", scope_name, first.code, second.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ACTION)) {
        expect(parser, TOK_LPAREN, "expected '(' after action");
        Expr href = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in action");
        Expr label = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_action(&%s, %s, %s)", scope_name, href.code, label.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_AT)) {
        Token list_tok;
        expect(parser, TOK_LPAREN, "expected '(' after at");
        list_tok = parser->lexer.current;
        if (list_tok.kind != TOK_IDENT) {
            fatal_at(list_tok.line, list_tok.col, "expected list variable");
        }
        advance(parser);
        char *name = token_text(list_tok);
        ValueType type = lookup_symbol(parser, name);
        expect(parser, TOK_COMMA, "expected ',' in at");
        Expr index = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        if (type != TYPE_TEXTLIST) {
            fatal_at(list_tok.line, list_tok.col, "at(list, idx) in text context requires textlist");
        }
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_textlist_at(%s, %s)", name, index.code);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected text expression");
    return make_expr(TYPE_TEXT, xstrdup("\"\""));
}

static Expr parse_list_expr(Parser *parser, int scope_id, ValueType expected_type) {
    Token token = parser->lexer.current;
    if (parser->lexer.current.kind == TOK_IDENT) {
        match(parser, TOK_IDENT);
        char *name = token_text(token);
        ValueType type = lookup_symbol(parser, name);
        if (type != expected_type) {
            fatal_at(token.line, token.col, "list type mismatch on '%s'", name);
        }
        return make_expr(type, name);
    }

    if (match(parser, TOK_PACK)) {
        expect(parser, TOK_LPAREN, "expected '(' after pack");
        Buffer elems;
        buf_init(&elems);
        size_t count = 0;
        if (!match(parser, TOK_RPAREN)) {
            for (;;) {
                if (count > 0) {
                    expect(parser, TOK_COMMA, "expected ',' in pack");
                }
                Expr item;
                if (expected_type == TYPE_NUMLIST) {
                    item = parse_num_expr(parser, scope_id);
                } else if (expected_type == TYPE_TEXTLIST) {
                    item = parse_text_expr(parser, scope_id);
                } else {
                    fatal_at(token.line, token.col, "pack only creates lists");
                }
                if (count > 0) {
                    buf_append(&elems, ", ");
                }
                buf_append(&elems, item.code);
                count++;
                if (parser->lexer.current.kind == TOK_RPAREN) {
                    break;
                }
            }
            expect(parser, TOK_RPAREN, "expected ')'");
        }
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        if (expected_type == TYPE_NUMLIST) {
            buf_appendf(&code, "spear_numlist_make(&%s, (long long[]){%s}, %zu)", scope_name, elems.data ? elems.data : "", count);
        } else if (expected_type == TYPE_TEXTLIST) {
            buf_appendf(&code, "spear_textlist_make(&%s, (const char *[]){%s}, %zu)", scope_name, elems.data ? elems.data : "", count);
        } else {
            fatal_at(token.line, token.col, "pack only creates lists");
        }
        free(scope_name);
        return make_expr(expected_type, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected list expression");
    return make_expr(expected_type, xstrdup("NULL"));
}

static bool starts_text_expr(Parser *parser) {
    if (parser->lexer.current.kind == TOK_STRING ||
        parser->lexer.current.kind == TOK_JOIN ||
        parser->lexer.current.kind == TOK_READ ||
        parser->lexer.current.kind == TOK_ESCAPE ||
        parser->lexer.current.kind == TOK_MARKUP ||
        parser->lexer.current.kind == TOK_PAGE ||
        parser->lexer.current.kind == TOK_STACK ||
        parser->lexer.current.kind == TOK_INLINE ||
        parser->lexer.current.kind == TOK_ACTION) {
        return true;
    }
    if (parser->lexer.current.kind == TOK_IDENT) {
        Token next = peek_token(parser);
        char *name = token_text(parser->lexer.current);
        if (next.kind == TOK_LPAREN) {
            FunctionInfo *fn = find_function(parser, name);
            free(name);
            return fn && fn->return_type == TYPE_TEXT;
        }
        ValueType type = lookup_symbol(parser, name);
        free(name);
        return type == TYPE_TEXT;
    }
    return false;
}

static bool at_returns_text(Parser *parser) {
    if (parser->lexer.current.kind != TOK_AT) {
        return false;
    }
    Lexer copy = parser->lexer;
    Token open = lexer_next(&copy);
    Token name_tok = lexer_next(&copy);
    (void) open;
    if (name_tok.kind != TOK_IDENT) {
        return false;
    }
    char *name = token_text(name_tok);
    ValueType type = lookup_symbol(parser, name);
    free(name);
    return type == TYPE_TEXTLIST;
}

static void parse_statement(Parser *parser, int scope_id) {
    if (match(parser, TOK_CONST)) {
        bool infer = match(parser, TOK_LET);
        ValueType type;
        if (infer) {
            Token name_tok = parser->lexer.current;
            expect(parser, TOK_IDENT, "expected variable name");
            char *name = token_text(name_tok);
            expect(parser, TOK_ASSIGN, "expected '='");
            type = infer_expr_type(parser);
            Expr value;
            if (type == TYPE_NUM) value = parse_num_expr(parser, scope_id);
            else if (type == TYPE_TEXT) value = parse_text_expr(parser, scope_id);
            else value = parse_list_expr(parser, scope_id, type);
            expect(parser, TOK_SEMI, "expected ';'");
            add_symbol(parser, name, type, true);
            emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
            return;
        }

        if (!is_type_token(parser->lexer.current.kind)) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "expected type or let after const");
        }
        type = token_to_type(parser->lexer.current.kind);
        advance(parser);
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected variable name");
        char *name = token_text(name_tok);
        expect(parser, TOK_ASSIGN, "expected '='");
        Expr value;
        if (type == TYPE_NUM) value = parse_num_expr(parser, scope_id);
        else if (type == TYPE_TEXT) value = parse_text_expr(parser, scope_id);
        else value = parse_list_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, true);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return;
    }

    if (match(parser, TOK_LET)) {
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected variable name");
        char *name = token_text(name_tok);
        expect(parser, TOK_ASSIGN, "expected '='");
        ValueType type = infer_expr_type(parser);
        Expr value;
        if (type == TYPE_NUM) value = parse_num_expr(parser, scope_id);
        else if (type == TYPE_TEXT) value = parse_text_expr(parser, scope_id);
        else value = parse_list_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, false);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return;
    }

    if (is_type_token(parser->lexer.current.kind)) {
        TokenKind decl_kind = parser->lexer.current.kind;
        ValueType type = token_to_type(decl_kind);
        advance(parser);
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected variable name");
        char *name = token_text(name_tok);
        expect(parser, TOK_ASSIGN, "expected '='");
        Expr value;
        if (type == TYPE_NUM) value = parse_num_expr(parser, scope_id);
        else if (type == TYPE_TEXT) value = parse_text_expr(parser, scope_id);
        else value = parse_list_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, false);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return;
    }

    if (match(parser, TOK_SAY)) {
        expect(parser, TOK_LPAREN, "expected '(' after say");
        if (at_returns_text(parser) || starts_text_expr(parser)) {
            Expr value = parse_text_expr(parser, scope_id);
            emit_line(parser, "printf(\"%%s\\n\", %s);", value.code);
        } else {
            Expr value = parse_num_expr(parser, scope_id);
            emit_line(parser, "printf(\"%%lld\\n\", (long long) (%s));", value.code);
        }
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        return;
    }

    if (match(parser, TOK_WRITE)) {
        expect(parser, TOK_LPAREN, "expected '(' after write");
        Expr path = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in write");
        Expr content = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "spear_write_text(%s, %s);", path.code, content.code);
        return;
    }

    if (match(parser, TOK_GUARD)) {
        expect(parser, TOK_LPAREN, "expected '(' after guard");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in guard");
        Expr message = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "if (!(%s)) { fprintf(stderr, \"%%s\\n\", %s); exit(1); }", cond.code, message.code);
        return;
    }

    if (match(parser, TOK_SHARP)) {
        parse_block(parser, scope_id, true);
        return;
    }

    if (match(parser, TOK_IF)) {
        expect(parser, TOK_LPAREN, "expected '(' after if");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        emit_line(parser, "if (%s)", cond.code);
        parse_block(parser, scope_id, false);
        if (match(parser, TOK_ELSE)) {
            emit_line(parser, "else");
            parse_block(parser, scope_id, false);
        }
        return;
    }

    if (match(parser, TOK_WHILE)) {
        expect(parser, TOK_LPAREN, "expected '(' after while");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        emit_line(parser, "while (%s)", cond.code);
        parse_block(parser, scope_id, false);
        return;
    }

    if (match(parser, TOK_EACH)) {
        Token item_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected loop variable name");
        char *item_name = token_text(item_tok);
        expect(parser, TOK_IN, "expected 'in' after loop variable");
        Token list_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected list name");
        char *list_name = token_text(list_tok);
        ValueType list_type = lookup_symbol(parser, list_name);
        if (list_type != TYPE_NUMLIST && list_type != TYPE_TEXTLIST) {
            fatal_at(list_tok.line, list_tok.col, "each expects a list");
        }

        char *idx_name = new_temp(parser);
        emit_line(parser, "{");
        parser->depth++;
        emit_line(parser, "for (long long %s = 0; %s < %s(%s); %s++)", idx_name, idx_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_count" : "spear_textlist_count",
            list_name, idx_name);
        emit_line(parser, "{");
        parser->depth++;
        emit_line(parser, "%s %s = %s(%s, %s);",
            list_type == TYPE_NUMLIST ? "long long" : "char *",
            item_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_at" : "spear_textlist_at",
            list_name,
            idx_name);
        free(idx_name);
        expect(parser, TOK_LBRACE, "expected '{' after each header");
        int symbol_depth = parser->depth;
        add_symbol(parser, item_name, list_type == TYPE_NUMLIST ? TYPE_NUM : TYPE_TEXT, false);
        while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
            parse_statement(parser, scope_id);
        }
        expect(parser, TOK_RBRACE, "expected '}' after each body");
        pop_symbols(parser, symbol_depth);
        parser->depth--;
        emit_line(parser, "}");
        parser->depth--;
        emit_line(parser, "}");
        return;
    }

    if (match(parser, TOK_PUSH)) {
        Token list_tok;
        expect(parser, TOK_LPAREN, "expected '(' after push");
        list_tok = parser->lexer.current;
        if (list_tok.kind != TOK_IDENT) {
            fatal_at(list_tok.line, list_tok.col, "expected list variable");
        }
        advance(parser);
        char *name = token_text(list_tok);
        ValueType type = lookup_symbol(parser, name);
        expect(parser, TOK_COMMA, "expected ',' in push");
        if (type == TYPE_NUMLIST) {
            Expr value = parse_num_expr(parser, scope_id);
            emit_line(parser, "spear_numlist_push(%s, %s);", name, value.code);
        } else if (type == TYPE_TEXTLIST) {
            Expr value = parse_text_expr(parser, scope_id);
            emit_line(parser, "spear_textlist_push(%s, %s);", name, value.code);
        } else {
            fatal_at(list_tok.line, list_tok.col, "push expects a list");
        }
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        return;
    }

    if (match(parser, TOK_RETURN)) {
        Expr value;
        if (parser->current_return_type == TYPE_NUM) {
            value = parse_num_expr(parser, scope_id);
            emit_line(parser, "_spear_result_num = %s;", value.code);
        } else if (parser->current_return_type == TYPE_TEXT) {
            value = parse_text_expr(parser, scope_id);
            emit_line(parser, "_spear_result_text = spear_text_clone(ret_scope ? ret_scope : &_scope_0, %s);", value.code);
        } else {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "only num/text function returns are supported");
        }
        expect(parser, TOK_SEMI, "expected ';'");
        for (int i = parser->active_scope_count - 1; i >= 0; i--) {
            char *scope_name = make_scope_name(parser->active_scope_ids[i]);
            emit_line(parser, "spear_scope_leave(&%s);", scope_name);
            free(scope_name);
        }
        emit_line(parser, "goto _spear_done;");
        return;
    }

    if (parser->lexer.current.kind == TOK_IDENT) {
        Token name_tok = parser->lexer.current;
        advance(parser);
        char *name = token_text(name_tok);
        ValueType type = lookup_symbol(parser, name);
        if (symbol_is_const(parser, name)) {
            fatal_at(name_tok.line, name_tok.col, "cannot assign to const '%s'", name);
        }
        expect(parser, TOK_ASSIGN, "expected '='");
        Expr value;
        if (type == TYPE_NUM) value = parse_num_expr(parser, scope_id);
        else if (type == TYPE_TEXT) value = parse_text_expr(parser, scope_id);
        else value = parse_list_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "%s = %s;", name, value.code);
        return;
    }

    fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unexpected statement");
}

static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope) {
    expect(parser, TOK_LBRACE, "expected '{'");

    int scope_id = parent_scope_id;
    emit_line(parser, "{");
    parser->depth++;
    if (creates_scope) {
        scope_id = ++parser->scope_counter;
        parser->active_scope_ids[parser->active_scope_count++] = scope_id;
        char *scope_name = make_scope_name(scope_id);
        char *parent_name = make_scope_name(parent_scope_id);
        emit_line(parser, "SpearScope %s = spear_scope_enter(&%s);", scope_name, parent_name);
        free(scope_name);
        free(parent_name);
    }

    int symbol_depth = parser->depth;
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        parse_statement(parser, scope_id);
    }
    expect(parser, TOK_RBRACE, "expected '}'");

    if (creates_scope) {
        char *scope_name = make_scope_name(scope_id);
        emit_line(parser, "spear_scope_leave(&%s);", scope_name);
        free(scope_name);
        parser->active_scope_count--;
    }
    pop_symbols(parser, symbol_depth);
    parser->depth--;
    emit_line(parser, "}");
}

static const char *runtime_prelude =
"#include <ctype.h>\n"
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <string.h>\n"
"\n"
"typedef struct SpearChunk {\n"
"    struct SpearChunk *next;\n"
"    size_t used;\n"
"    size_t cap;\n"
"    char data[];\n"
"} SpearChunk;\n"
"\n"
"typedef struct SpearScope {\n"
"    struct SpearScope *parent;\n"
"    SpearChunk *chunks;\n"
"    struct SpearCleanup *cleanups;\n"
"} SpearScope;\n"
"\n"
"typedef void (*SpearCleanupFn)(void *);\n"
"\n"
"typedef struct SpearCleanup {\n"
"    SpearCleanupFn fn;\n"
"    void *ptr;\n"
"    struct SpearCleanup *next;\n"
"} SpearCleanup;\n"
"\n"
"typedef struct SpearNumList {\n"
"    SpearScope *owner;\n"
"    long long *items;\n"
"    size_t len;\n"
"    size_t cap;\n"
"} SpearNumList;\n"
"\n"
"typedef struct SpearTextList {\n"
"    SpearScope *owner;\n"
"    char **items;\n"
"    size_t len;\n"
"    size_t cap;\n"
"} SpearTextList;\n"
"\n"
"static void spear_numlist_push(SpearNumList *list, long long value);\n"
"static void spear_textlist_push(SpearTextList *list, const char *value);\n"
"\n"
"static SpearScope spear_scope_enter(SpearScope *parent) {\n"
"    SpearScope scope;\n"
"    scope.parent = parent;\n"
"    scope.chunks = NULL;\n"
"    scope.cleanups = NULL;\n"
"    return scope;\n"
"}\n"
"\n"
"static void *spear_alloc(SpearScope *scope, size_t size) {\n"
"    const size_t align = sizeof(void *);\n"
"    size = (size + align - 1) & ~(align - 1);\n"
"    if (!scope->chunks || scope->chunks->used + size > scope->chunks->cap) {\n"
"        size_t cap = size > 4096 ? size : 4096;\n"
"        SpearChunk *chunk = (SpearChunk *) malloc(sizeof(SpearChunk) + cap);\n"
"        if (!chunk) {\n"
"            fprintf(stderr, \"out of memory\\n\");\n"
"            exit(1);\n"
"        }\n"
"        chunk->next = scope->chunks;\n"
"        chunk->used = 0;\n"
"        chunk->cap = cap;\n"
"        scope->chunks = chunk;\n"
"    }\n"
"    void *ptr = scope->chunks->data + scope->chunks->used;\n"
"    scope->chunks->used += size;\n"
"    return ptr;\n"
"}\n"
"\n"
"static void spear_scope_add_cleanup(SpearScope *scope, SpearCleanupFn fn, void *ptr) {\n"
"    SpearCleanup *cleanup = (SpearCleanup *) spear_alloc(scope, sizeof(SpearCleanup));\n"
"    cleanup->fn = fn;\n"
"    cleanup->ptr = ptr;\n"
"    cleanup->next = scope->cleanups;\n"
"    scope->cleanups = cleanup;\n"
"}\n"
"\n"
"static char *spear_text_clone(SpearScope *scope, const char *src) {\n"
"    size_t len = strlen(src) + 1;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    memcpy(dst, src, len);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_text_join(SpearScope *scope, const char *a, const char *b) {\n"
"    size_t len_a = strlen(a);\n"
"    size_t len_b = strlen(b);\n"
"    char *dst = (char *) spear_alloc(scope, len_a + len_b + 1);\n"
"    memcpy(dst, a, len_a);\n"
"    memcpy(dst + len_a, b, len_b + 1);\n"
"    return dst;\n"
"}\n"
"\n"
"static int spear_tag_is_safe(const char *tag) {\n"
"    if (!tag[0]) return 0;\n"
"    for (size_t i = 0; tag[i]; i++) {\n"
"        char c = tag[i];\n"
"        if (!(isalnum((unsigned char) c) || c == '-')) return 0;\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_html_escape(SpearScope *scope, const char *src) {\n"
"    size_t extra = 0;\n"
"    for (size_t i = 0; src[i]; i++) {\n"
"        switch (src[i]) {\n"
"            case '&': extra += 4; break;\n"
"            case '<': extra += 3; break;\n"
"            case '>': extra += 3; break;\n"
"            case '\"': extra += 5; break;\n"
"            case '\\'': extra += 4; break;\n"
"            default: break;\n"
"        }\n"
"    }\n"
"    size_t len = strlen(src);\n"
"    char *dst = (char *) spear_alloc(scope, len + extra + 1);\n"
"    size_t j = 0;\n"
"    for (size_t i = 0; src[i]; i++) {\n"
"        const char *rep = NULL;\n"
"        switch (src[i]) {\n"
"            case '&': rep = \"&amp;\"; break;\n"
"            case '<': rep = \"&lt;\"; break;\n"
"            case '>': rep = \"&gt;\"; break;\n"
"            case '\"': rep = \"&quot;\"; break;\n"
"            case '\\'': rep = \"&#39;\"; break;\n"
"            default: dst[j++] = src[i]; continue;\n"
"        }\n"
"        size_t rlen = strlen(rep);\n"
"        memcpy(dst + j, rep, rlen);\n"
"        j += rlen;\n"
"    }\n"
"    dst[j] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_markup(SpearScope *scope, const char *tag, const char *content) {\n"
"    if (!spear_tag_is_safe(tag)) {\n"
"        fprintf(stderr, \"unsafe tag name\\n\");\n"
"        exit(1);\n"
"    }\n"
"    size_t tag_len = strlen(tag);\n"
"    size_t body_len = strlen(content);\n"
"    char *dst = (char *) spear_alloc(scope, tag_len * 2 + body_len + 6);\n"
"    sprintf(dst, \"<%s>%s</%s>\", tag, content, tag);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_page(SpearScope *scope, const char *title, const char *body) {\n"
"    char *safe_title = spear_html_escape(scope, title);\n"
"    const char *prefix = \"<!doctype html><html><head><meta charset=\\\"utf-8\\\"><meta name=\\\"viewport\\\" content=\\\"width=device-width,initial-scale=1\\\"><title>\";\n"
"    const char *middle = \"</title></head><body>\";\n"
"    const char *suffix = \"</body></html>\";\n"
"    size_t len = strlen(prefix) + strlen(safe_title) + strlen(middle) + strlen(body) + strlen(suffix) + 1;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    sprintf(dst, \"%s%s%s%s%s\", prefix, safe_title, middle, body, suffix);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_stack(SpearScope *scope, const char *a, const char *b) {\n"
"    char *body = spear_text_join(scope, a, b);\n"
"    return spear_markup(scope, \"div\", body);\n"
"}\n"
"\n"
"static char *spear_inline(SpearScope *scope, const char *a, const char *b) {\n"
"    char *body = spear_text_join(scope, a, b);\n"
"    return spear_markup(scope, \"span\", body);\n"
"}\n"
"\n"
"static int spear_href_is_safe(const char *href) {\n"
"    return strncmp(href, \"https://\", 8) == 0 || strncmp(href, \"http://\", 7) == 0 || href[0] == '/' || href[0] == '#';\n"
"}\n"
"\n"
"static char *spear_action(SpearScope *scope, const char *href, const char *label) {\n"
"    if (!spear_href_is_safe(href)) {\n"
"        fprintf(stderr, \"unsafe action href\\n\");\n"
"        exit(1);\n"
"    }\n"
"    char *safe_href = spear_html_escape(scope, href);\n"
"    char *safe_label = spear_html_escape(scope, label);\n"
"    size_t len = strlen(safe_href) + strlen(safe_label) + 32;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    sprintf(dst, \"<a href=\\\"%s\\\">%s</a>\", safe_href, safe_label);\n"
"    return dst;\n"
"}\n"
"\n"
"static void spear_write_text(const char *path, const char *content) {\n"
"    FILE *fp = fopen(path, \"wb\");\n"
"    if (!fp) {\n"
"        fprintf(stderr, \"cannot write %s\\n\", path);\n"
"        exit(1);\n"
"    }\n"
"    size_t len = strlen(content);\n"
"    if (fwrite(content, 1, len, fp) != len) {\n"
"        fprintf(stderr, \"failed to write %s\\n\", path);\n"
"        fclose(fp);\n"
"        exit(1);\n"
"    }\n"
"    fclose(fp);\n"
"}\n"
"\n"
"static long long spear_text_size(const char *src) {\n"
"    return (long long) strlen(src);\n"
"}\n"
"\n"
"static long long spear_text_same(const char *a, const char *b) {\n"
"    return strcmp(a, b) == 0;\n"
"}\n"
"\n"
"static char *spear_text_read(SpearScope *scope) {\n"
"    size_t cap = 64;\n"
"    size_t len = 0;\n"
"    char *temp = (char *) malloc(cap);\n"
"    int ch;\n"
"    if (!temp) {\n"
"        fprintf(stderr, \"out of memory\\n\");\n"
"        exit(1);\n"
"    }\n"
"    while ((ch = getchar()) != EOF && ch != '\\n') {\n"
"        if (len + 1 >= cap) {\n"
"            cap *= 2;\n"
"            temp = (char *) realloc(temp, cap);\n"
"            if (!temp) {\n"
"                fprintf(stderr, \"out of memory\\n\");\n"
"                exit(1);\n"
"            }\n"
"        }\n"
"        temp[len++] = (char) ch;\n"
"    }\n"
"    temp[len] = '\\0';\n"
"    char *dst = spear_text_clone(scope, temp);\n"
"    free(temp);\n"
"    return dst;\n"
"}\n"
"\n"
"static void spear_numlist_drop(void *ptr) {\n"
"    SpearNumList *list = (SpearNumList *) ptr;\n"
"    free(list->items);\n"
"    free(list);\n"
"}\n"
"\n"
"static void spear_textlist_drop(void *ptr) {\n"
"    SpearTextList *list = (SpearTextList *) ptr;\n"
"    free(list->items);\n"
"    free(list);\n"
"}\n"
"\n"
"static SpearNumList *spear_numlist_new(SpearScope *scope) {\n"
"    SpearNumList *list = (SpearNumList *) calloc(1, sizeof(SpearNumList));\n"
"    if (!list) { fprintf(stderr, \"out of memory\\n\"); exit(1); }\n"
"    list->owner = scope;\n"
"    spear_scope_add_cleanup(scope, spear_numlist_drop, list);\n"
"    return list;\n"
"}\n"
"\n"
"static SpearNumList *spear_numlist_make(SpearScope *scope, const long long *items, size_t count) {\n"
"    SpearNumList *list = spear_numlist_new(scope);\n"
"    for (size_t i = 0; i < count; i++) spear_numlist_push(list, items[i]);\n"
"    return list;\n"
"}\n"
"\n"
"static SpearTextList *spear_textlist_new(SpearScope *scope) {\n"
"    SpearTextList *list = (SpearTextList *) calloc(1, sizeof(SpearTextList));\n"
"    if (!list) { fprintf(stderr, \"out of memory\\n\"); exit(1); }\n"
"    list->owner = scope;\n"
"    spear_scope_add_cleanup(scope, spear_textlist_drop, list);\n"
"    return list;\n"
"}\n"
"\n"
"static SpearTextList *spear_textlist_make(SpearScope *scope, const char **items, size_t count) {\n"
"    SpearTextList *list = spear_textlist_new(scope);\n"
"    for (size_t i = 0; i < count; i++) spear_textlist_push(list, items[i]);\n"
"    return list;\n"
"}\n"
"\n"
"static void spear_numlist_push(SpearNumList *list, long long value) {\n"
"    if (list->len == list->cap) {\n"
"        size_t next = list->cap ? list->cap * 2 : 8;\n"
"        long long *items = (long long *) realloc(list->items, next * sizeof(long long));\n"
"        if (!items) { fprintf(stderr, \"out of memory\\n\"); exit(1); }\n"
"        list->items = items;\n"
"        list->cap = next;\n"
"    }\n"
"    list->items[list->len++] = value;\n"
"}\n"
"\n"
"static void spear_textlist_push(SpearTextList *list, const char *value) {\n"
"    if (list->len == list->cap) {\n"
"        size_t next = list->cap ? list->cap * 2 : 8;\n"
"        char **items = (char **) realloc(list->items, next * sizeof(char *));\n"
"        if (!items) { fprintf(stderr, \"out of memory\\n\"); exit(1); }\n"
"        list->items = items;\n"
"        list->cap = next;\n"
"    }\n"
"    list->items[list->len++] = spear_text_clone(list->owner, value);\n"
"}\n"
"\n"
"static long long spear_numlist_count(SpearNumList *list) { return (long long) list->len; }\n"
"static long long spear_textlist_count(SpearTextList *list) { return (long long) list->len; }\n"
"\n"
"static long long spear_numlist_at(SpearNumList *list, long long index) {\n"
"    if (index < 0 || (size_t) index >= list->len) { fprintf(stderr, \"numlist index out of bounds\\n\"); exit(1); }\n"
"    return list->items[index];\n"
"}\n"
"\n"
"static char *spear_textlist_at(SpearTextList *list, long long index) {\n"
"    if (index < 0 || (size_t) index >= list->len) { fprintf(stderr, \"textlist index out of bounds\\n\"); exit(1); }\n"
"    return list->items[index];\n"
"}\n"
"\n"
"static void spear_scope_leave(SpearScope *scope) {\n"
"    SpearCleanup *cleanup = scope->cleanups;\n"
"    while (cleanup) {\n"
"        cleanup->fn(cleanup->ptr);\n"
"        cleanup = cleanup->next;\n"
"    }\n"
"    SpearChunk *chunk = scope->chunks;\n"
"    while (chunk) {\n"
"        SpearChunk *next = chunk->next;\n"
"        free(chunk);\n"
"        chunk = next;\n"
"    }\n"
"    scope->chunks = NULL;\n"
"}\n"
"\n";

static char *read_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "cannot open %s\n", path);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *data = xmalloc((size_t) size + 1);
    if (fread(data, 1, (size_t) size, fp) != (size_t) size) {
        fprintf(stderr, "failed to read %s\n", path);
        fclose(fp);
        exit(1);
    }
    data[size] = '\0';
    fclose(fp);
    return data;
}

static void write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "cannot write %s\n", path);
        exit(1);
    }
    size_t len = strlen(content);
    if (fwrite(content, 1, len, fp) != len) {
        fprintf(stderr, "failed to write %s\n", path);
        fclose(fp);
        exit(1);
    }
    fclose(fp);
}

static char *default_output_path(const char *input_path) {
    size_t len = strlen(input_path);
    Buffer out;
    buf_init(&out);
    if (len > 3 && strcmp(input_path + len - 3, ".sp") == 0) {
        char *base = slice_dup(input_path, len - 3);
        buf_appendf(&out, "%s.c", base);
        free(base);
    } else {
        buf_appendf(&out, "%s.c", input_path);
    }
    return buf_take(&out);
}

static void emit_function_prototype(Buffer *out, FunctionInfo *fn) {
    if (fn->is_entry) {
        buf_appendf(out, "int %s(void);\n", fn->name);
        return;
    }
    buf_appendf(out, "%s %s(SpearScope *ret_scope", ctype_name(fn->return_type), fn->name);
    for (size_t i = 0; i < fn->param_count; i++) {
        buf_appendf(out, ", %s %s", ctype_name(fn->params[i].type), fn->params[i].name);
    }
    buf_append(out, ");\n");
}

static void parse_function_definition(Parser *parser) {
    bool is_entry = false;
    ValueType return_type = TYPE_NUM;
    if (match(parser, TOK_SPEAR)) {
        is_entry = true;
    } else if (is_type_token(parser->lexer.current.kind) || parser->lexer.current.kind == TOK_VIEW) {
        return_type = parser->lexer.current.kind == TOK_VIEW ? TYPE_TEXT : token_to_type(parser->lexer.current.kind);
        if (return_type != TYPE_NUM && return_type != TYPE_TEXT) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "top-level functions may return only num or text");
        }
        advance(parser);
    } else {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "expected top-level function");
    }

    Token name_tok = parser->lexer.current;
    expect(parser, TOK_IDENT, "expected function name");
    char *name = token_text(name_tok);
    FunctionInfo *fn = find_function(parser, name);
    if (!fn) {
        fatal_at(name_tok.line, name_tok.col, "internal error: missing function info");
    }

    parser->depth = 0;
    parser->scope_counter = 0;
    parser->current_return_type = return_type;
    parser->current_is_entry = is_entry;
    parser->active_scope_count = 0;
    parser->symbol_count = 0;

    expect(parser, TOK_LPAREN, "expected '(' after function name");

    if (is_entry) {
        expect(parser, TOK_RPAREN, "entry function does not take parameters");
        buf_appendf(&parser->out, "\nint %s(void)\n", name);
    } else {
        buf_appendf(&parser->out, "\n%s %s(SpearScope *ret_scope", ctype_name(return_type), name);
        if (parser->lexer.current.kind != TOK_RPAREN) {
            for (size_t i = 0; i < fn->param_count; i++) {
                Token type_tok = parser->lexer.current;
                if (!is_type_token(type_tok.kind)) {
                    fatal_at(type_tok.line, type_tok.col, "expected parameter type");
                }
                advance(parser);
                Token param_tok = parser->lexer.current;
                expect(parser, TOK_IDENT, "expected parameter name");
                buf_appendf(&parser->out, ", %s %s", ctype_name(token_to_type(type_tok.kind)), fn->params[i].name);
                (void) param_tok;
                if (i + 1 < fn->param_count) {
                    expect(parser, TOK_COMMA, "expected ','");
                }
            }
        }
        expect(parser, TOK_RPAREN, "expected ')'");
        buf_append(&parser->out, ")\n");
    }

    emit_line(parser, "{");
    parser->depth++;
    emit_line(parser, "SpearScope _scope_0 = spear_scope_enter(NULL);");
    if (!is_entry) {
        emit_line(parser, "(void) ret_scope;");
    }
    if (return_type == TYPE_NUM || is_entry) {
        emit_line(parser, "long long _spear_result_num = 0;");
    } else {
        emit_line(parser, "char *_spear_result_text = spear_text_clone(&_scope_0, \"\");");
    }

    for (size_t i = 0; i < fn->param_count; i++) {
        add_symbol(parser, fn->params[i].name, fn->params[i].type, true);
    }

    expect(parser, TOK_LBRACE, "expected '{' after function header");
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        parse_statement(parser, 0);
    }
    expect(parser, TOK_RBRACE, "expected '}' after function body");
    emit_line(parser, "goto _spear_done;");
    emit_line(parser, "_spear_done:");
    emit_line(parser, "spear_scope_leave(&_scope_0);");
    if (is_entry || return_type == TYPE_NUM) {
        emit_line(parser, "return _spear_result_num;");
    } else {
        emit_line(parser, "return _spear_result_text;");
    }
    parser->depth--;
    emit_line(parser, "}");
}

static char *compile_source(const char *source) {
    Parser parser;
    parser_init(&parser, source);
    collect_functions(&parser, source);
    lexer_init(&parser.lexer, source);
    parser.lexer.current = lexer_next(&parser.lexer);
    buf_append(&parser.out, runtime_prelude);

    char *entry = NULL;
    for (size_t i = 0; i < parser.function_count; i++) {
        emit_function_prototype(&parser.out, &parser.functions[i]);
        if (parser.functions[i].is_entry) {
            entry = parser.functions[i].name;
        }
    }
    if (!entry) {
        fatal_at(1, 1, "program requires one spear entry function");
    }

    while (parser.lexer.current.kind != TOK_EOF) {
        parse_function_definition(&parser);
    }

    buf_appendf(&parser.out, "\nint main(void) { return %s(); }\n", entry);

    return buf_take(&parser.out);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 4) {
        fprintf(stderr, "usage: spearc <input.sp> [-o output.c]\n");
        return 1;
    }

    const char *input = argv[1];
    char *output = NULL;

    if (argc == 4) {
        if (strcmp(argv[2], "-o") != 0) {
            fprintf(stderr, "expected -o before output path\n");
            return 1;
        }
        output = xstrdup(argv[3]);
    } else {
        output = default_output_path(input);
    }

    char *source = read_file(input);
    char *generated = compile_source(source);
    write_file(output, generated);

    printf("generated %s\n", output);
    free(source);
    free(generated);
    free(output);
    return 0;
}
