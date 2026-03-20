#include <direct.h>
#include <ctype.h>
#include <setjmp.h>
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
    TYPE_TEXTLIST,
    TYPE_MAP,
    TYPE_RESULT
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
    TOK_MAP,
    TOK_RESULT,
    TOK_LET,
    TOK_VAR,
    TOK_CONST,
    TOK_VIEW,
    TOK_FUNCTION,
    TOK_SAY,
    TOK_WRITE,
    TOK_IF,
    TOK_ELSE,
    TOK_FOR,
    TOK_WHILE,
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_EACH,
    TOK_IN,
    TOK_RETURN,
    TOK_IMPORT,
    TOK_MODULE,
    TOK_PACKAGE,
    TOK_TRY,
    TOK_CATCH,
    TOK_THROW,
    TOK_CLASS,
    TOK_NODECALL,
    TOK_PYCALL,
    TOK_JOIN,
    TOK_READ,
    TOK_SIZE,
    TOK_SAME,
    TOK_PACK,
    TOK_PUSH,
    TOK_COUNT,
    TOK_AT,
    TOK_ARGCOUNT,
    TOK_ARGAT,
    TOK_PUT,
    TOK_GET,
    TOK_HAS,
    TOK_DROP,
    TOK_OK,
    TOK_FAIL,
    TOK_ISOK,
    TOK_UNWRAP,
    TOK_ERRORTEXT,
    TOK_GUARD,
    TOK_ESCAPE,
    TOK_MARKUP,
    TOK_PAGE,
    TOK_STACK,
    TOK_INLINE,
    TOK_COLUMN,
    TOK_ROW,
    TOK_BOX,
    TOK_SURFACEBOX,
    TOK_COLUMNBOX,
    TOK_ROWBOX,
    TOK_ACTION,
    TOK_ATTR,
    TOK_ATTRS,
    TOK_CLASSNAME,
    TOK_IDNAME,
    TOK_STYLEATTR,
    TOK_HREFATTR,
    TOK_SRCATTR,
    TOK_ALTATTR,
    TOK_ASK,
    TOK_RUN
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
    bool used;
    bool is_parameter;
    int line;
    int col;
} Symbol;

typedef struct {
    char *code;
    ValueType type;
} Expr;

typedef struct {
    char *name;
    ValueType type;
    int line;
    int col;
} Param;

typedef struct {
    char *name;
    char *package_name;
    char *module_name;
    char *c_name;
    ValueType return_type;
    bool is_entry;
    bool used;
    int line;
    int col;
    Param *params;
    size_t param_count;
} FunctionInfo;

typedef struct {
    char *path;
    char *label;
    int line;
    char **exported_functions;
    size_t exported_count;
    size_t exported_cap;
} ImportInfo;

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
    int loop_depth;
    int active_scope_ids[128];
    int active_scope_loop_depths[128];
    int active_scope_count;
    char *current_package_name;
    char *current_module_name;
} Parser;

typedef struct {
    char **items;
    size_t count;
    size_t cap;
} StringList;

static char g_compiler_lang[8] = "en";
static char g_tool_dir[2048] = "";
static bool g_collect_errors = false;
static int g_error_count = 0;
static jmp_buf *g_recover_env = NULL;

static bool compiler_lang_is(const char *code) {
    return strcmp(g_compiler_lang, code) == 0;
}

static void compiler_load_lang(const char *tool_dir) {
    char path[2048];
    FILE *fp;
    snprintf(path, sizeof(path), "%s\\spear-lang.txt", tool_dir);
    fp = fopen(path, "rb");
    if (!fp) return;
    if (fgets(g_compiler_lang, sizeof(g_compiler_lang), fp)) {
        if ((unsigned char) g_compiler_lang[0] == 0xEF && (unsigned char) g_compiler_lang[1] == 0xBB && (unsigned char) g_compiler_lang[2] == 0xBF) {
            memmove(g_compiler_lang, g_compiler_lang + 3, strlen(g_compiler_lang + 3) + 1);
        }
        for (size_t i = 0; g_compiler_lang[i]; i++) {
            if (g_compiler_lang[i] == '\r' || g_compiler_lang[i] == '\n') {
                g_compiler_lang[i] = '\0';
                break;
            }
        }
        if (strcmp(g_compiler_lang, "ko") != 0 && strcmp(g_compiler_lang, "en") != 0) {
            strcpy(g_compiler_lang, "en");
        }
    }
    fclose(fp);
}

static const char *compiler_text(const char *key) __attribute__((unused));
static const char *compiler_text(const char *key) {
    if (strcmp(key, "error_prefix") == 0) {
        if (compiler_lang_is("ko")) return "spearc 오류";
        if (compiler_lang_is("ja")) return "spearc エラー";
        if (compiler_lang_is("zh")) return "spearc 错误";
        return "spearc error";
    }
    if (strcmp(key, "oom") == 0) {
        if (compiler_lang_is("ko")) return "메모리가 부족합니다";
        if (compiler_lang_is("ja")) return "メモリが不足しています";
        if (compiler_lang_is("zh")) return "内存不足";
        return "out of memory";
    }
    if (strcmp(key, "path_too_long") == 0) {
        if (compiler_lang_is("ko")) return "경로 또는 명령이 너무 깁니다";
        if (compiler_lang_is("ja")) return "パスまたはコマンドが長すぎます";
        if (compiler_lang_is("zh")) return "路径或命令过长";
        return "path or command too long";
    }
    if (strcmp(key, "usage") == 0) {
        if (compiler_lang_is("ko")) return "spearc 오류: 사용법: spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
        if (compiler_lang_is("ja")) return "spearc エラー: 使い方: spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
        if (compiler_lang_is("zh")) return "spearc 错误：用法：spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
        return "spearc error: usage: spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
    }
    if (strcmp(key, "cannot_open") == 0) {
        if (compiler_lang_is("ko")) return "열 수 없습니다";
        if (compiler_lang_is("ja")) return "開けません";
        if (compiler_lang_is("zh")) return "无法打开";
        return "cannot open";
    }
    if (strcmp(key, "failed_read") == 0) {
        if (compiler_lang_is("ko")) return "읽기에 실패했습니다";
        if (compiler_lang_is("ja")) return "読み込みに失敗しました";
        if (compiler_lang_is("zh")) return "读取失败";
        return "failed to read";
    }
    if (strcmp(key, "cannot_write") == 0) {
        if (compiler_lang_is("ko")) return "쓸 수 없습니다";
        if (compiler_lang_is("ja")) return "書き込めません";
        if (compiler_lang_is("zh")) return "无法写入";
        return "cannot write";
    }
    if (strcmp(key, "failed_write") == 0) {
        if (compiler_lang_is("ko")) return "쓰기에 실패했습니다";
        if (compiler_lang_is("ja")) return "書き込みに失敗しました";
        if (compiler_lang_is("zh")) return "写入失败";
        return "failed to write";
    }
    if (strcmp(key, "cannot_resolve") == 0) {
        if (compiler_lang_is("ko")) return "경로를 확인할 수 없습니다";
        if (compiler_lang_is("ja")) return "パスを解決できません";
        if (compiler_lang_is("zh")) return "无法解析路径";
        return "cannot resolve";
    }
    return key;
}

static const char *compiler_message(const char *key) {
    if (strcmp(key, "error_prefix") == 0) {
        if (compiler_lang_is("ko")) return "spearc 오류";
        return "spearc error";
    }
    if (strcmp(key, "warning_prefix") == 0) {
        if (compiler_lang_is("ko")) return "spearc 경고";
        return "spearc warning";
    }
    if (strcmp(key, "unused_variable") == 0) {
        if (compiler_lang_is("ko")) return "사용하지 않는 변수 '%s'";
        return "unused variable '%s'";
    }
    if (strcmp(key, "unused_parameter") == 0) {
        if (compiler_lang_is("ko")) return "사용하지 않는 매개변수 '%s'";
        return "unused parameter '%s'";
    }
    if (strcmp(key, "duplicate_import") == 0) {
        if (compiler_lang_is("ko")) return "중복 import는 무시됩니다: %s";
        return "duplicate import ignored: %s";
    }
    if (strcmp(key, "unused_import") == 0) {
        if (compiler_lang_is("ko")) return "사용하지 않는 import입니다: %s";
        return "unused import: %s";
    }
    if (strcmp(key, "unused_function") == 0) {
        if (compiler_lang_is("ko")) return "사용하지 않는 함수 '%s'";
        return "unused function '%s'";
    }
    if (strcmp(key, "unreachable_code") == 0) {
        if (compiler_lang_is("ko")) return "도달할 수 없는 코드입니다";
        return "unreachable code";
    }
    if (strcmp(key, "condition_always_true") == 0) {
        if (compiler_lang_is("ko")) return "'%s' 조건은 항상 참입니다";
        return "'%s' condition is always true";
    }
    if (strcmp(key, "condition_always_false") == 0) {
        if (compiler_lang_is("ko")) return "'%s' 조건은 항상 거짓입니다";
        return "'%s' condition is always false";
    }
    if (strcmp(key, "oom") == 0) {
        if (compiler_lang_is("ko")) return "메모리가 부족합니다";
        return "out of memory";
    }
    if (strcmp(key, "path_too_long") == 0) {
        if (compiler_lang_is("ko")) return "경로나 명령어가 너무 깁니다";
        return "path or command too long";
    }
    if (strcmp(key, "usage") == 0) {
        if (compiler_lang_is("ko")) return "spearc 오류: 사용법: spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
        return "spearc error: usage: spearc <input.sp> [-o output.c] | spearc --check <input.sp> | spearc --check-stdin <input.sp>\n";
    }
    if (strcmp(key, "cannot_open") == 0) {
        if (compiler_lang_is("ko")) return "열 수 없습니다";
        return "cannot open";
    }
    if (strcmp(key, "failed_read") == 0) {
        if (compiler_lang_is("ko")) return "읽기에 실패했습니다";
        return "failed to read";
    }
    if (strcmp(key, "cannot_write") == 0) {
        if (compiler_lang_is("ko")) return "쓸 수 없습니다";
        return "cannot write";
    }
    if (strcmp(key, "failed_write") == 0) {
        if (compiler_lang_is("ko")) return "쓰기에 실패했습니다";
        return "failed to write";
    }
    if (strcmp(key, "cannot_resolve") == 0) {
        if (compiler_lang_is("ko")) return "경로를 확인할 수 없습니다";
        return "cannot resolve";
    }
    return key;
}

static void fatal_at(int line, int col, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s [line %d:%d] ", compiler_message("error_prefix"), line, col);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    if (g_collect_errors && g_recover_env) {
        g_error_count++;
        longjmp(*g_recover_env, 1);
    }
    exit(1);
}

static void warning_at(int line, int col, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s [line %d:%d] ", compiler_message("warning_prefix"), line, col);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

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

static bool string_list_has(StringList *list, const char *value) {
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], value) == 0) {
            return true;
        }
    }
    return false;
}

static void string_list_add(StringList *list, const char *value) {
    if (string_list_has(list, value)) {
        return;
    }
    if (list->count == list->cap) {
        size_t next = list->cap ? list->cap * 2 : 8;
        list->items = realloc(list->items, next * sizeof(char *));
        if (!list->items) {
            fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("oom"));
            exit(1);
        }
        list->cap = next;
    }
    list->items[list->count++] = xstrdup(value);
}

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
    if (len == 2 && strncmp(text, "fn", len) == 0) return TOK_FUNCTION;
    if (len == 8 && strncmp(text, "function", len) == 0) return TOK_FUNCTION;
    if (len == 3 && strncmp(text, "say", len) == 0) return TOK_SAY;
    if (len == 4 && strncmp(text, "show", len) == 0) return TOK_SAY;
    if (len == 5 && strncmp(text, "print", len) == 0) return TOK_SAY;
    if (len == 5 && strncmp(text, "write", len) == 0) return TOK_WRITE;
    if (len == 3 && strncmp(text, "app", len) == 0) return TOK_RUN;
    if (len == 3 && strncmp(text, "run", len) == 0) return TOK_RUN;
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
    if (len == 3 && strncmp(text, "ask", len) == 0) return TOK_ASK;
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
    parser->loop_depth = 0;
    parser->active_scope_count = 0;
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

static bool parse_statement(Parser *parser, int scope_id);

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

static char *token_text(Token token) {
    return slice_dup(token.start, token.length);
}

static bool token_is_ident_text(Token token, const char *text) {
    size_t len = strlen(text);
    return token.kind == TOK_IDENT && token.length == len && strncmp(token.start, text, len) == 0;
}

static bool match_let_alias(Parser *parser) {
    if (match(parser, TOK_LET)) {
        return true;
    }
    if (token_is_ident_text(parser->lexer.current, "value")) {
        advance(parser);
        return true;
    }
    return false;
}

static bool match_var_alias(Parser *parser) {
    if (match(parser, TOK_VAR)) {
        return true;
    }
    if (token_is_ident_text(parser->lexer.current, "variable") ||
        token_is_ident_text(parser->lexer.current, "mutable")) {
        advance(parser);
        return true;
    }
    return false;
}

static ValueType token_to_type(TokenKind kind) {
    switch (kind) {
        case TOK_NUM: return TYPE_NUM;
        case TOK_TEXT: return TYPE_TEXT;
        case TOK_NUMLIST: return TYPE_NUMLIST;
        case TOK_TEXTLIST: return TYPE_TEXTLIST;
        case TOK_MAP: return TYPE_MAP;
        case TOK_RESULT: return TYPE_RESULT;
        default:
            fprintf(stderr, "spearc error: internal error: invalid type token\n");
            exit(1);
    }
}

static const char *ctype_name(ValueType type) {
    switch (type) {
        case TYPE_NUM: return "long long";
        case TYPE_TEXT: return "char *";
        case TYPE_NUMLIST: return "SpearNumList *";
        case TYPE_TEXTLIST: return "SpearTextList *";
        case TYPE_MAP: return "SpearMap *";
        case TYPE_RESULT: return "SpearResult *";
        default: return "void *";
    }
}

static bool is_type_token(TokenKind kind) {
    return kind == TOK_NUM || kind == TOK_TEXT || kind == TOK_NUMLIST || kind == TOK_TEXTLIST || kind == TOK_MAP || kind == TOK_RESULT;
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

static void collect_functions(Parser *parser, const char *source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    char *current_package = NULL;
    char *current_module = NULL;

    for (;;) {
        Token token = lexer_next(&lexer);
        if (token.kind == TOK_EOF) {
            return;
        }

        if (token.kind == TOK_MODULE || token.kind == TOK_PACKAGE) {
            Token next = lexer_next(&lexer);
            if (next.kind != TOK_IDENT) {
                fatal_at(next.line, next.col, "expected name after %s", token_text(token));
            }
            if (token.kind == TOK_PACKAGE) {
                free(current_package);
                current_package = token_text(next);
            } else {
                free(current_module);
                current_module = token_text(next);
            }
            Token semi = lexer_next(&lexer);
            if (semi.kind != TOK_SEMI) {
                fatal_at(semi.line, semi.col, "expected ';' after metadata");
            }
            continue;
        }
        if (token.kind == TOK_CLASS) {
            fatal_at(token.line, token.col, "class is reserved but not implemented yet");
        }

        bool is_entry = false;
        ValueType return_type = TYPE_NUM;
        if (token.kind == TOK_FUNCTION) {
            token = lexer_next(&lexer);
        }
        if (token.kind == TOK_RUN) {
            FunctionInfo info;
            info.name = xstrdup("__spear_run_entry");
            info.package_name = current_package ? xstrdup(current_package) : xstrdup("");
            info.module_name = current_module ? xstrdup(current_module) : xstrdup("");
            info.c_name = make_function_c_name(info.package_name, info.module_name, info.name, true);
            info.return_type = TYPE_NUM;
            info.is_entry = true;
            info.used = true;
            info.line = token.line;
            info.col = token.col;
            info.params = NULL;
            info.param_count = 0;
            add_function_info(parser, info);
            scan_skip_block(&lexer);
            continue;
        }
        if (token.kind == TOK_SPEAR) {
            is_entry = true;
        } else if (token.kind == TOK_NUM || token.kind == TOK_TEXT || token.kind == TOK_NUMLIST || token.kind == TOK_TEXTLIST || token.kind == TOK_MAP || token.kind == TOK_RESULT || token.kind == TOK_VIEW) {
            return_type = token.kind == TOK_VIEW ? TYPE_TEXT : token_to_type(token.kind);
        } else {
            fatal_at(token.line, token.col, "top-level definition must start with function, run, spear, num, text, numlist, textlist, map, result, or view");
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
                        fprintf(stderr, "spearc error: out of memory\n");
                        exit(1);
                    }
                    param_cap = grow;
                }
                params[param_count].name = token_text(param_name);
                params[param_count].type = token_to_type(next.kind);
                params[param_count].line = param_name.line;
                params[param_count].col = param_name.col;
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
        info.package_name = current_package ? xstrdup(current_package) : xstrdup("");
        info.module_name = current_module ? xstrdup(current_module) : xstrdup("");
        info.c_name = make_function_c_name(info.package_name, info.module_name, info.name, is_entry);
        info.return_type = return_type;
        info.is_entry = is_entry;
        info.used = is_entry;
        info.line = name_tok.line;
        info.col = name_tok.col;
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
static Expr parse_map_expr(Parser *parser, int scope_id);
static Expr parse_result_expr(Parser *parser, int scope_id);
static Expr parse_value_expr(Parser *parser, int scope_id, ValueType type);
static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope);
static bool starts_text_expr(Parser *parser);
static bool at_returns_text(Parser *parser);
static char *parse_text_children(Parser *parser, int scope_id);

static ValueType infer_expr_type(Parser *parser) {
    Token token = parser->lexer.current;
    if (token.kind == TOK_STRING ||
        token.kind == TOK_JOIN ||
        token.kind == TOK_READ ||
        token.kind == TOK_ASK ||
        token.kind == TOK_ESCAPE ||
        token.kind == TOK_MARKUP ||
        token.kind == TOK_PAGE ||
        token.kind == TOK_STACK ||
        token.kind == TOK_INLINE ||
        token.kind == TOK_COLUMN ||
        token.kind == TOK_ROW ||
        token.kind == TOK_BOX ||
        token.kind == TOK_SURFACEBOX ||
        token.kind == TOK_COLUMNBOX ||
        token.kind == TOK_ROWBOX ||
        token.kind == TOK_NODECALL ||
        token.kind == TOK_PYCALL ||
        token.kind == TOK_ATTR ||
        token.kind == TOK_ATTRS ||
        token.kind == TOK_CLASSNAME ||
        token.kind == TOK_IDNAME ||
        token.kind == TOK_STYLEATTR ||
        token.kind == TOK_HREFATTR ||
        token.kind == TOK_SRCATTR ||
        token.kind == TOK_ALTATTR ||
        token.kind == TOK_ACTION) {
        return TYPE_TEXT;
    }
    if (token.kind == TOK_TEXT && peek_token(parser).kind == TOK_LPAREN) {
        return TYPE_TEXT;
    }
    if (token.kind == TOK_NUMBER || token.kind == TOK_SIZE || token.kind == TOK_SAME || token.kind == TOK_COUNT || token.kind == TOK_ARGCOUNT) {
        return TYPE_NUM;
    }
    if (token.kind == TOK_AT) {
        return at_returns_text(parser) ? TYPE_TEXT : TYPE_NUM;
    }
    if (token.kind == TOK_ARGAT) {
        return TYPE_TEXT;
    }
    if (token.kind == TOK_MAP && peek_token(parser).kind == TOK_LPAREN) {
        return TYPE_MAP;
    }
    if (token.kind == TOK_OK || token.kind == TOK_FAIL) {
        return TYPE_RESULT;
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
            mark_function_used(fn);
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
                arg = parse_value_expr(parser, scope_id, fn->params[i].type);
                buf_appendf(&args, ", %s", arg.code);
            }
            expect(parser, TOK_RPAREN, "expected ')'");
            Buffer code;
            buf_init(&code);
            buf_appendf(&code, "%s(%s)", fn->c_name, args.data ? args.data : "");
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
        else if (type == TYPE_MAP) buf_appendf(&code, "spear_map_count(%s)", name);
        else fatal_at(list_tok.line, list_tok.col, "count expects a list or map");
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_AT)) {
        Token at_tok = token;
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
        buf_appendf(&code, "spear_numlist_at(%s, %s, %d, %d)", name, index.code, at_tok.line, at_tok.col);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_ARGCOUNT)) {
        expect(parser, TOK_LPAREN, "expected '(' after arg_count");
        expect(parser, TOK_RPAREN, "expected ')'");
        return make_expr(TYPE_NUM, xstrdup("spear_arg_count()"));
    }

    if (match(parser, TOK_HAS)) {
        Token has_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after has");
        Expr map = parse_map_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in has");
        Expr key = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_map_has(%s, %s, %d, %d)", map.code, key.code, has_tok.line, has_tok.col);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    if (match(parser, TOK_ISOK)) {
        Token ok_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after is_ok");
        Expr result = parse_result_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_result_is_ok(%s, %d, %d)", result.code, ok_tok.line, ok_tok.col);
        return make_expr(TYPE_NUM, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected numeric expression");
    return make_expr(TYPE_NUM, xstrdup("0"));
}

static Expr parse_unary_num(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;
    if (match(parser, TOK_MINUS)) {
        Expr inner = parse_unary_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_checked_neg(%s, %d, %d)", inner.code, token.line, token.col);
        return make_expr(TYPE_NUM, buf_take(&code));
    }
    return parse_primary_num(parser, scope_id);
}

static Expr parse_mul_num(Parser *parser, int scope_id) {
    Expr left = parse_unary_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_STAR ||
           parser->lexer.current.kind == TOK_SLASH ||
           parser->lexer.current.kind == TOK_PERCENT) {
        Token op_tok = parser->lexer.current;
        TokenKind op = op_tok.kind;
        advance(parser);
        Expr right = parse_unary_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        if (op == TOK_STAR) {
            buf_appendf(&code, "spear_checked_mul(%s, %s, %d, %d)", left.code, right.code, op_tok.line, op_tok.col);
        } else if (op == TOK_SLASH) {
            buf_appendf(&code, "spear_checked_div(%s, %s, %d, %d)", left.code, right.code, op_tok.line, op_tok.col);
        } else {
            buf_appendf(&code, "spear_checked_mod(%s, %s, %d, %d)", left.code, right.code, op_tok.line, op_tok.col);
        }
        left = make_expr(TYPE_NUM, buf_take(&code));
    }
    return left;
}

static Expr parse_add_num(Parser *parser, int scope_id) {
    Expr left = parse_mul_num(parser, scope_id);
    while (parser->lexer.current.kind == TOK_PLUS ||
           parser->lexer.current.kind == TOK_MINUS) {
        Token op_tok = parser->lexer.current;
        TokenKind op = op_tok.kind;
        advance(parser);
        Expr right = parse_mul_num(parser, scope_id);
        Buffer code;
        buf_init(&code);
        if (op == TOK_PLUS) {
            buf_appendf(&code, "spear_checked_add(%s, %s, %d, %d)", left.code, right.code, op_tok.line, op_tok.col);
        } else {
            buf_appendf(&code, "spear_checked_sub(%s, %s, %d, %d)", left.code, right.code, op_tok.line, op_tok.col);
        }
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

static char *combine_text_parts(int scope_id, Expr *parts, size_t count) {
    if (count == 0) {
        return xstrdup("\"\"");
    }
    char *scope_name = make_scope_name(scope_id);
    char *combined = xstrdup(parts[0].code);
    for (size_t i = 1; i < count; i++) {
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "spear_text_join(&%s, %s, %s)", scope_name, combined, parts[i].code);
        combined = buf_take(&code);
    }
    free(scope_name);
    return combined;
}

static char *parse_text_children(Parser *parser, int scope_id) {
    expect(parser, TOK_LBRACE, "expected '{' for UI content");
    Expr *parts = NULL;
    size_t count = 0;
    size_t cap = 0;
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        if (!starts_text_expr(parser)) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "UI blocks only accept text/view expressions");
        }
        if (count == cap) {
            size_t next = cap ? cap * 2 : 4;
            parts = realloc(parts, next * sizeof(Expr));
            if (!parts) {
                fprintf(stderr, "spearc error: out of memory\n");
                exit(1);
            }
            cap = next;
        }
        parts[count++] = parse_text_expr(parser, scope_id);
        expect(parser, TOK_SEMI, "expected ';' after UI child");
    }
    expect(parser, TOK_RBRACE, "expected '}' after UI content");
    return combine_text_parts(scope_id, parts, count);
}

static Expr parse_text_expr(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;

    if (parser->lexer.current.kind == TOK_EACH) {
        Token each_tok = parser->lexer.current;
        advance(parser);
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
        char *acc_name = new_temp(parser);
        char *scope_name = make_scope_name(scope_id);
        parser->depth++;
        int symbol_depth = parser->depth;
        add_symbol(parser, item_name, list_type == TYPE_NUMLIST ? TYPE_NUM : TYPE_TEXT, false, item_tok.line, item_tok.col, false, false);
        char *body = parse_text_children(parser, scope_id);
        pop_symbols(parser, symbol_depth);
        parser->depth--;

        Buffer code;
        buf_init(&code);
        buf_appendf(
            &code,
            "({ char *%s = spear_text_clone(&%s, \"\"); for (long long %s = 0; %s < %s(%s); %s++) { %s %s = %s(%s, %s, %d, %d); %s = spear_text_join(&%s, %s, %s); } %s; })",
            acc_name,
            scope_name,
            idx_name,
            idx_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_count" : "spear_textlist_count",
            list_name,
            idx_name,
            list_type == TYPE_NUMLIST ? "long long" : "char *",
            item_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_at" : "spear_textlist_at",
            list_name,
            idx_name,
            each_tok.line,
            each_tok.col,
            acc_name,
            scope_name,
            acc_name,
            body,
            acc_name
        );
        free(scope_name);
        free(idx_name);
        free(acc_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_IF)) {
        expect(parser, TOK_LPAREN, "expected '(' after if");
        warn_constant_condition(parser->lexer.current, "if");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        char *when_true = parse_text_children(parser, scope_id);
        char *when_false = NULL;
        if (match(parser, TOK_ELSE)) {
            when_false = parse_text_children(parser, scope_id);
        } else {
            char *scope_name = make_scope_name(scope_id);
            Buffer empty_code;
            buf_init(&empty_code);
            buf_appendf(&empty_code, "spear_text_clone(&%s, \"\")", scope_name);
            free(scope_name);
            when_false = buf_take(&empty_code);
        }
        Buffer code;
        buf_init(&code);
        buf_appendf(&code, "((%s) ? (%s) : (%s))", cond.code, when_true, when_false);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (parser->lexer.current.kind == TOK_TEXT && peek_token(parser).kind == TOK_LPAREN) {
        advance(parser);
        expect(parser, TOK_LPAREN, "expected '(' after text");
        Expr value = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_text_from_num(&%s, %s)", scope_name, value.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

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
            mark_function_used(fn);
            advance(parser);
            expect(parser, TOK_LPAREN, "expected '('");
            Buffer args;
            buf_init(&args);
            char *scope_name = make_scope_name(scope_id);
            buf_appendf(&args, "&%s", scope_name);
            free(scope_name);
            size_t arg_index = 0;
            bool consumed_trailing_block = false;
            while (arg_index < fn->param_count) {
                bool can_use_trailing_block = (
                    fn->params[arg_index].type == TYPE_TEXT &&
                    parser->lexer.current.kind == TOK_RPAREN &&
                    peek_token(parser).kind == TOK_LBRACE
                );
                if (can_use_trailing_block) {
                    break;
                }
                if (arg_index > 0) {
                    expect(parser, TOK_COMMA, "expected ','");
                }
                Expr arg;
                arg = parse_value_expr(parser, scope_id, fn->params[arg_index].type);
                buf_appendf(&args, ", %s", arg.code);
                arg_index++;
            }
            expect(parser, TOK_RPAREN, "expected ')'");
            if (arg_index < fn->param_count) {
                if (arg_index + 1 == fn->param_count &&
                    fn->params[arg_index].type == TYPE_TEXT &&
                    parser->lexer.current.kind == TOK_LBRACE) {
                    char *children = parse_text_children(parser, scope_id);
                    buf_appendf(&args, ", %s", children);
                    arg_index++;
                    consumed_trailing_block = true;
                } else {
                    fatal_at(token.line, token.col, "function '%s' is missing required arguments", fname);
                }
            }
            if (!consumed_trailing_block && parser->lexer.current.kind == TOK_LBRACE) {
                fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unexpected block after function call");
            }
            Buffer code;
            buf_init(&code);
            buf_appendf(&code, "%s(%s)", fn->c_name, args.data ? args.data : "");
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

    if (match(parser, TOK_ASK)) {
        expect(parser, TOK_LPAREN, "expected '(' after ask");
        Expr prompt = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "(printf(\"%%s\\n\", %s), spear_text_read(&%s))", prompt.code, scope_name);
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

    if (match(parser, TOK_ATTR)) {
        expect(parser, TOK_LPAREN, "expected '(' after attr");
        Expr name = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in attr");
        Expr value = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_attr(&%s, %s, %s)", scope_name, name.code, value.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ATTRS)) {
        expect(parser, TOK_LPAREN, "expected '(' after attrs");
        Expr left = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in attrs");
        Expr right = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_attrs(&%s, %s, %s)", scope_name, left.code, right.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_CLASSNAME) || match(parser, TOK_IDNAME) || match(parser, TOK_STYLEATTR) ||
        match(parser, TOK_HREFATTR) || match(parser, TOK_SRCATTR) || match(parser, TOK_ALTATTR)) {
        TokenKind helper_kind = token.kind;
        expect(parser, TOK_LPAREN, "expected '(' after attribute helper");
        Expr value = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        const char *attr_name = "class";
        if (helper_kind == TOK_IDNAME) attr_name = "id";
        else if (helper_kind == TOK_STYLEATTR) attr_name = "style";
        else if (helper_kind == TOK_HREFATTR) attr_name = "href";
        else if (helper_kind == TOK_SRCATTR) attr_name = "src";
        else if (helper_kind == TOK_ALTATTR) attr_name = "alt";
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_attr(&%s, \"%s\", %s)", scope_name, attr_name, value.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_MARKUP)) {
        expect(parser, TOK_LPAREN, "expected '(' after markup");
        Expr tag = parse_text_expr(parser, scope_id);
        Expr attrs = make_expr(TYPE_TEXT, xstrdup("\"\""));
        Expr content;
        if (match(parser, TOK_COMMA)) {
            Expr second = parse_text_expr(parser, scope_id);
            if (match(parser, TOK_COMMA)) {
                attrs = second;
                content = parse_text_expr(parser, scope_id);
                expect(parser, TOK_RPAREN, "expected ')'");
            } else {
                expect(parser, TOK_RPAREN, "expected ')'");
                if (parser->lexer.current.kind == TOK_LBRACE) {
                    attrs = second;
                    content = make_expr(TYPE_TEXT, parse_text_children(parser, scope_id));
                } else {
                    content = second;
                }
            }
        } else {
            expect(parser, TOK_RPAREN, "expected ')' after markup tag");
            content = make_expr(TYPE_TEXT, parse_text_children(parser, scope_id));
        }
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_markup_attrs(&%s, %s, %s, %s)", scope_name, tag.code, attrs.code, content.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_PAGE)) {
        expect(parser, TOK_LPAREN, "expected '(' after page");
        Expr title = parse_text_expr(parser, scope_id);
        Expr body;
        if (match(parser, TOK_COMMA)) {
            body = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
        } else {
            expect(parser, TOK_RPAREN, "expected ')' after page title");
            body = make_expr(TYPE_TEXT, parse_text_children(parser, scope_id));
        }
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_page(&%s, %s, %s)", scope_name, title.code, body.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_STACK)) {
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        if (parser->lexer.current.kind == TOK_LBRACE) {
            char *children = parse_text_children(parser, scope_id);
            buf_init(&code);
            buf_appendf(&code, "spear_column(&%s, %s)", scope_name, children);
        } else {
            expect(parser, TOK_LPAREN, "expected '(' or '{' after stack");
            Expr first = parse_text_expr(parser, scope_id);
            expect(parser, TOK_COMMA, "expected ',' in stack");
            Expr second = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            buf_init(&code);
            buf_appendf(&code, "spear_stack(&%s, %s, %s)", scope_name, first.code, second.code);
        }
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_INLINE)) {
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        if (parser->lexer.current.kind == TOK_LBRACE) {
            char *children = parse_text_children(parser, scope_id);
            buf_init(&code);
            buf_appendf(&code, "spear_markup(&%s, \"span\", %s)", scope_name, children);
        } else {
            expect(parser, TOK_LPAREN, "expected '(' or '{' after inline");
            Expr first = parse_text_expr(parser, scope_id);
            expect(parser, TOK_COMMA, "expected ',' in inline");
            Expr second = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            buf_init(&code);
            buf_appendf(&code, "spear_inline(&%s, %s, %s)", scope_name, first.code, second.code);
        }
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_COLUMN)) {
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = parse_text_children(parser, scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_column(&%s, %s)", scope_name, children);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ROW)) {
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = parse_text_children(parser, scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_row(&%s, %s)", scope_name, children);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_BOX)) {
        expect(parser, TOK_LPAREN, "expected '(' after box");
        Expr mods = parse_text_expr(parser, scope_id);
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = NULL;
        if (match(parser, TOK_COMMA)) {
            Expr body = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            children = body.code;
        } else {
            expect(parser, TOK_RPAREN, "expected ')'");
            children = parse_text_children(parser, scope_id);
        }
        buf_init(&code);
        buf_appendf(&code, "spear_markup_attrs(&%s, \"div\", spear_attr(&%s, \"style\", spear_style_chain(&%s, \"\", %s)), %s)", scope_name, scope_name, scope_name, mods.code, children);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_SURFACEBOX)) {
        expect(parser, TOK_LPAREN, "expected '(' after surface_box");
        Expr mods = parse_text_expr(parser, scope_id);
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = NULL;
        if (match(parser, TOK_COMMA)) {
            Expr body = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            children = body.code;
        } else {
            expect(parser, TOK_RPAREN, "expected ')'");
            children = parse_text_children(parser, scope_id);
        }
        buf_init(&code);
        buf_appendf(&code, "spear_markup_attrs(&%s, \"div\", spear_attr(&%s, \"style\", spear_style_chain(&%s, \"background:#ffffff;border:1px solid #dbe2ea;border-radius:20px;padding:20px;\", %s)), %s)", scope_name, scope_name, scope_name, mods.code, children);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_COLUMNBOX)) {
        expect(parser, TOK_LPAREN, "expected '(' after column_box");
        Expr mods = parse_text_expr(parser, scope_id);
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = NULL;
        if (match(parser, TOK_COMMA)) {
            Expr body = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            children = body.code;
        } else {
            expect(parser, TOK_RPAREN, "expected ')'");
            children = parse_text_children(parser, scope_id);
        }
        buf_init(&code);
        buf_appendf(&code, "spear_markup_attrs(&%s, \"div\", spear_attr(&%s, \"style\", spear_style_chain(&%s, \"display:flex;flex-direction:column;\", %s)), %s)", scope_name, scope_name, scope_name, mods.code, children);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ROWBOX)) {
        expect(parser, TOK_LPAREN, "expected '(' after row_box");
        Expr mods = parse_text_expr(parser, scope_id);
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        char *children = NULL;
        if (match(parser, TOK_COMMA)) {
            Expr body = parse_text_expr(parser, scope_id);
            expect(parser, TOK_RPAREN, "expected ')'");
            children = body.code;
        } else {
            expect(parser, TOK_RPAREN, "expected ')'");
            children = parse_text_children(parser, scope_id);
        }
        buf_init(&code);
        buf_appendf(&code, "spear_markup_attrs(&%s, \"div\", spear_attr(&%s, \"style\", spear_style_chain(&%s, \"display:flex;flex-direction:row;\", %s)), %s)", scope_name, scope_name, scope_name, mods.code, children);
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

    if (match(parser, TOK_NODECALL)) {
        expect(parser, TOK_LPAREN, "expected '(' after nodecall");
        Expr pkg = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in nodecall");
        Expr fn = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in nodecall");
        Expr payload = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_node_call(&%s, %s, %s, %s)", scope_name, pkg.code, fn.code, payload.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_PYCALL)) {
        expect(parser, TOK_LPAREN, "expected '(' after pycall");
        Expr module = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in pycall");
        Expr fn = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in pycall");
        Expr payload = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_py_call(&%s, %s, %s, %s)", scope_name, module.code, fn.code, payload.code);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_AT)) {
        Token at_tok = token;
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
        buf_appendf(&code, "spear_textlist_at(%s, %s, %d, %d)", name, index.code, at_tok.line, at_tok.col);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ARGAT)) {
        Token arg_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after arg_at");
        Expr index = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_arg_at(&%s, %s, %d, %d)", scope_name, index.code, arg_tok.line, arg_tok.col);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_GET)) {
        Token get_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after get");
        Expr map = parse_map_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in get");
        Expr key = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in get");
        Expr fallback = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_map_get(&%s, %s, %s, %s, %d, %d)", scope_name, map.code, key.code, fallback.code, get_tok.line, get_tok.col);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_UNWRAP)) {
        Token unwrap_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after unwrap");
        Expr result = parse_result_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_result_unwrap(&%s, %s, %d, %d)", scope_name, result.code, unwrap_tok.line, unwrap_tok.col);
        free(scope_name);
        return make_expr(TYPE_TEXT, buf_take(&code));
    }

    if (match(parser, TOK_ERRORTEXT)) {
        Token err_tok = token;
        expect(parser, TOK_LPAREN, "expected '(' after error_text");
        Expr result = parse_result_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_result_error_text(&%s, %s, %d, %d)", scope_name, result.code, err_tok.line, err_tok.col);
        free(scope_name);
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

static Expr parse_map_expr(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;
    if (parser->lexer.current.kind == TOK_IDENT) {
        char *name = token_text(token);
        ValueType type = lookup_symbol(parser, name);
        if (type != TYPE_MAP) {
            fatal_at(token.line, token.col, "expected map variable, got non-map '%s'", name);
        }
        advance(parser);
        return make_expr(TYPE_MAP, name);
    }

    if (match(parser, TOK_MAP)) {
        expect(parser, TOK_LPAREN, "expected '(' after map");
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_map_new(&%s)", scope_name);
        free(scope_name);
        return make_expr(TYPE_MAP, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected map expression");
    return make_expr(TYPE_MAP, xstrdup("NULL"));
}

static Expr parse_result_expr(Parser *parser, int scope_id) {
    Token token = parser->lexer.current;
    if (parser->lexer.current.kind == TOK_IDENT) {
        Token next = peek_token(parser);
        if (next.kind == TOK_LPAREN) {
            char *fname = token_text(token);
            FunctionInfo *fn = find_function(parser, fname);
            if (!fn) {
                fatal_at(token.line, token.col, "unknown function '%s'", fname);
            }
            if (fn->return_type != TYPE_RESULT) {
                fatal_at(token.line, token.col, "function '%s' does not return result", fname);
            }
            mark_function_used(fn);
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
                Expr arg = parse_value_expr(parser, scope_id, fn->params[i].type);
                buf_appendf(&args, ", %s", arg.code);
            }
            expect(parser, TOK_RPAREN, "expected ')'");
            Buffer code;
            buf_init(&code);
            buf_appendf(&code, "%s(%s)", fn->c_name, args.data ? args.data : "");
            return make_expr(TYPE_RESULT, buf_take(&code));
        }
    }

    if (parser->lexer.current.kind == TOK_IDENT) {
        char *name = token_text(token);
        ValueType type = lookup_symbol(parser, name);
        if (type != TYPE_RESULT) {
            fatal_at(token.line, token.col, "expected result variable, got non-result '%s'", name);
        }
        advance(parser);
        return make_expr(TYPE_RESULT, name);
    }

    if (match(parser, TOK_OK)) {
        expect(parser, TOK_LPAREN, "expected '(' after ok");
        Expr value = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_result_ok(&%s, %s)", scope_name, value.code);
        free(scope_name);
        return make_expr(TYPE_RESULT, buf_take(&code));
    }

    if (match(parser, TOK_FAIL)) {
        expect(parser, TOK_LPAREN, "expected '(' after fail");
        Expr message = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        Buffer code;
        char *scope_name = make_scope_name(scope_id);
        buf_init(&code);
        buf_appendf(&code, "spear_result_fail(&%s, %s)", scope_name, message.code);
        free(scope_name);
        return make_expr(TYPE_RESULT, buf_take(&code));
    }

    fatal_at(token.line, token.col, "expected result expression");
    return make_expr(TYPE_RESULT, xstrdup("NULL"));
}

static Expr parse_value_expr(Parser *parser, int scope_id, ValueType type) {
    if (type == TYPE_NUM) return parse_num_expr(parser, scope_id);
    if (type == TYPE_TEXT) return parse_text_expr(parser, scope_id);
    if (type == TYPE_NUMLIST || type == TYPE_TEXTLIST) return parse_list_expr(parser, scope_id, type);
    if (type == TYPE_MAP) return parse_map_expr(parser, scope_id);
    if (type == TYPE_RESULT) return parse_result_expr(parser, scope_id);
    fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unsupported value type");
    return make_expr(TYPE_NUM, xstrdup("0"));
}

static bool starts_text_expr(Parser *parser) {
    if (parser->lexer.current.kind == TOK_EACH ||
        parser->lexer.current.kind == TOK_IF ||
        (parser->lexer.current.kind == TOK_TEXT && peek_token(parser).kind == TOK_LPAREN) ||
        parser->lexer.current.kind == TOK_STRING ||
        parser->lexer.current.kind == TOK_JOIN ||
        parser->lexer.current.kind == TOK_READ ||
        parser->lexer.current.kind == TOK_ASK ||
        parser->lexer.current.kind == TOK_ESCAPE ||
        parser->lexer.current.kind == TOK_MARKUP ||
        parser->lexer.current.kind == TOK_PAGE ||
        parser->lexer.current.kind == TOK_STACK ||
        parser->lexer.current.kind == TOK_INLINE ||
        parser->lexer.current.kind == TOK_COLUMN ||
        parser->lexer.current.kind == TOK_ROW ||
        parser->lexer.current.kind == TOK_BOX ||
        parser->lexer.current.kind == TOK_SURFACEBOX ||
        parser->lexer.current.kind == TOK_COLUMNBOX ||
        parser->lexer.current.kind == TOK_ROWBOX ||
        parser->lexer.current.kind == TOK_ATTR ||
        parser->lexer.current.kind == TOK_ATTRS ||
        parser->lexer.current.kind == TOK_CLASSNAME ||
        parser->lexer.current.kind == TOK_IDNAME ||
        parser->lexer.current.kind == TOK_STYLEATTR ||
        parser->lexer.current.kind == TOK_HREFATTR ||
        parser->lexer.current.kind == TOK_SRCATTR ||
        parser->lexer.current.kind == TOK_ALTATTR ||
        parser->lexer.current.kind == TOK_ARGAT ||
        parser->lexer.current.kind == TOK_GET ||
        parser->lexer.current.kind == TOK_UNWRAP ||
        parser->lexer.current.kind == TOK_ERRORTEXT ||
        parser->lexer.current.kind == TOK_NODECALL ||
        parser->lexer.current.kind == TOK_PYCALL ||
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

static bool parse_statement(Parser *parser, int scope_id) {
    if (match(parser, TOK_MODULE) || match(parser, TOK_PACKAGE)) {
        expect(parser, TOK_IDENT, "expected name after metadata");
        expect(parser, TOK_SEMI, "expected ';' after metadata");
        return false;
    }

    if (match(parser, TOK_CLASS)) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "class is reserved but not implemented yet");
    }

    if (match(parser, TOK_CONST)) {
        bool infer = false;
        if (parser->lexer.current.kind == TOK_IDENT) {
            infer = true;
        } else if (match_let_alias(parser)) {
            infer = true;
        }
        ValueType type;
        if (infer) {
            Token name_tok = parser->lexer.current;
            expect(parser, TOK_IDENT, "expected variable name");
            char *name = token_text(name_tok);
            expect(parser, TOK_ASSIGN, "expected '='");
            type = infer_expr_type(parser);
            Expr value;
            value = parse_value_expr(parser, scope_id, type);
            expect(parser, TOK_SEMI, "expected ';'");
            add_symbol(parser, name, type, true, name_tok.line, name_tok.col, false, false);
            emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
            return false;
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
        value = parse_value_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, true, name_tok.line, name_tok.col, false, false);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return false;
    }

    if (match_let_alias(parser) || match_var_alias(parser)) {
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected variable name");
        char *name = token_text(name_tok);
        expect(parser, TOK_ASSIGN, "expected '='");
        ValueType type = infer_expr_type(parser);
        Expr value;
        value = parse_value_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, false, name_tok.line, name_tok.col, false, false);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return false;
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
        value = parse_value_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        add_symbol(parser, name, type, false, name_tok.line, name_tok.col, false, false);
        emit_line(parser, "%s %s = %s;", ctype_name(type), name, value.code);
        return false;
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
        return false;
    }

    if (match(parser, TOK_WRITE)) {
        expect(parser, TOK_LPAREN, "expected '(' after write");
        Expr path = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in write");
        Expr content = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "spear_write_text(%s, %s);", path.code, content.code);
        return false;
    }

    if (match(parser, TOK_PUT)) {
        expect(parser, TOK_LPAREN, "expected '(' after put");
        Expr map = parse_map_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in put");
        Expr key = parse_text_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in put");
        Expr value = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "spear_map_set(%s, %s, %s);", map.code, key.code, value.code);
        return false;
    }

    if (match(parser, TOK_DROP)) {
        expect(parser, TOK_LPAREN, "expected '(' after drop");
        Expr map = parse_map_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in drop");
        Expr key = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "spear_map_remove(%s, %s);", map.code, key.code);
        return false;
    }

    if (parser->lexer.current.kind == TOK_GUARD) {
        Token guard_tok = parser->lexer.current;
        advance(parser);
        expect(parser, TOK_LPAREN, "expected '(' after guard");
        warn_constant_condition(parser->lexer.current, "guard");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_COMMA, "expected ',' in guard");
        Expr message = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "if (!(%s)) { spear_runtime_fail_at(%d, %d, %s); }", cond.code, guard_tok.line, guard_tok.col, message.code);
        return false;
    }

    if (match(parser, TOK_SHARP)) {
        parse_block(parser, scope_id, true);
        return false;
    }

    if (match(parser, TOK_IF)) {
        expect(parser, TOK_LPAREN, "expected '(' after if");
        warn_constant_condition(parser->lexer.current, "if");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        emit_line(parser, "if (%s)", cond.code);
        parse_block(parser, scope_id, false);
        if (match(parser, TOK_ELSE)) {
            emit_line(parser, "else");
            parse_block(parser, scope_id, false);
        }
        return false;
    }

    if (match(parser, TOK_WHILE)) {
        expect(parser, TOK_LPAREN, "expected '(' after while");
        warn_constant_condition(parser->lexer.current, "while");
        Expr cond = parse_num_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        parser->loop_depth++;
        emit_line(parser, "while (%s)", cond.code);
        parse_block(parser, scope_id, false);
        parser->loop_depth--;
        return false;
    }

    if (match(parser, TOK_FOR)) {
        expect(parser, TOK_LPAREN, "expected '(' after for");
        emit_line(parser, "{");
        parser->depth++;
        int symbol_depth = parser->depth;

        Buffer init;
        Buffer cond;
        Buffer step;
        buf_init(&init);
        buf_init(&cond);
        buf_init(&step);

        if (!match(parser, TOK_SEMI)) {
            if (match_let_alias(parser) || match_var_alias(parser)) {
                Token name_tok = parser->lexer.current;
                expect(parser, TOK_IDENT, "expected variable name");
                char *name = token_text(name_tok);
                expect(parser, TOK_ASSIGN, "expected '='");
                ValueType type = infer_expr_type(parser);
                Expr value;
                value = parse_value_expr(parser, scope_id, type);
                add_symbol(parser, name, type, false, name_tok.line, name_tok.col, false, false);
                buf_appendf(&init, "%s %s = %s", ctype_name(type), name, value.code);
            } else if (is_type_token(parser->lexer.current.kind)) {
                ValueType type = token_to_type(parser->lexer.current.kind);
                advance(parser);
                Token name_tok = parser->lexer.current;
                expect(parser, TOK_IDENT, "expected variable name");
                char *name = token_text(name_tok);
                expect(parser, TOK_ASSIGN, "expected '='");
                Expr value;
                value = parse_value_expr(parser, scope_id, type);
                add_symbol(parser, name, type, false, name_tok.line, name_tok.col, false, false);
                buf_appendf(&init, "%s %s = %s", ctype_name(type), name, value.code);
            } else if (parser->lexer.current.kind == TOK_IDENT) {
                Token name_tok = parser->lexer.current;
                advance(parser);
                char *name = token_text(name_tok);
                ValueType type = lookup_symbol(parser, name);
                if (symbol_is_const(parser, name)) {
                    fatal_at(name_tok.line, name_tok.col, "cannot assign to const '%s'", name);
                }
                expect(parser, TOK_ASSIGN, "expected '='");
                Expr value;
                value = parse_value_expr(parser, scope_id, type);
                buf_appendf(&init, "%s = %s", name, value.code);
            } else {
                fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unsupported for initializer");
            }
            expect(parser, TOK_SEMI, "expected ';' after for initializer");
        }

        if (!match(parser, TOK_SEMI)) {
            warn_constant_condition(parser->lexer.current, "for");
            Expr for_cond = parse_num_expr(parser, scope_id);
            buf_appendf(&cond, "%s", for_cond.code);
            expect(parser, TOK_SEMI, "expected ';' after for condition");
        } else {
            buf_append(&cond, "1");
        }

        if (!match(parser, TOK_RPAREN)) {
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
                value = parse_value_expr(parser, scope_id, type);
                buf_appendf(&step, "%s = %s", name, value.code);
            } else {
                fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unsupported for step");
            }
            expect(parser, TOK_RPAREN, "expected ')' after for step");
        }

        parser->loop_depth++;
        emit_line(parser, "for (%s; %s; %s)", init.data ? init.data : "", cond.data ? cond.data : "1", step.data ? step.data : "");
        parse_block(parser, scope_id, false);
        parser->loop_depth--;
        pop_symbols(parser, symbol_depth);
        parser->depth--;
        emit_line(parser, "}");
        return false;
    }

    if (parser->lexer.current.kind == TOK_EACH) {
        Token each_tok = parser->lexer.current;
        advance(parser);
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
        parser->loop_depth++;
        emit_line(parser, "{");
        parser->depth++;
        emit_line(parser, "for (long long %s = 0; %s < %s(%s); %s++)", idx_name, idx_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_count" : "spear_textlist_count",
            list_name, idx_name);
        emit_line(parser, "{");
        parser->depth++;
        emit_line(parser, "%s %s = %s(%s, %s, %d, %d);",
            list_type == TYPE_NUMLIST ? "long long" : "char *",
            item_name,
            list_type == TYPE_NUMLIST ? "spear_numlist_at" : "spear_textlist_at",
            list_name,
            idx_name,
            each_tok.line,
            each_tok.col);
        free(idx_name);
        expect(parser, TOK_LBRACE, "expected '{' after each header");
        int symbol_depth = parser->depth;
        add_symbol(parser, item_name, list_type == TYPE_NUMLIST ? TYPE_NUM : TYPE_TEXT, false, item_tok.line, item_tok.col, false, false);
        bool terminated = false;
        while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
            parse_statement_with_recovery(parser, scope_id, &terminated);
        }
        expect(parser, TOK_RBRACE, "expected '}' after each body");
        pop_symbols(parser, symbol_depth);
        parser->depth--;
        emit_line(parser, "}");
        parser->depth--;
        emit_line(parser, "}");
        parser->loop_depth--;
        return false;
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
        return false;
    }

    if (match(parser, TOK_RETURN)) {
        Expr value;
        if (parser->current_return_type == TYPE_NUM) {
            value = parse_num_expr(parser, scope_id);
            emit_line(parser, "_spear_result_num = %s;", value.code);
        } else if (parser->current_return_type == TYPE_TEXT) {
            value = parse_text_expr(parser, scope_id);
            emit_line(parser, "_spear_result_text = spear_text_clone(ret_scope ? ret_scope : &_scope_0, %s);", value.code);
        } else if (parser->current_return_type == TYPE_RESULT) {
            value = parse_result_expr(parser, scope_id);
            emit_line(parser, "_spear_result_result = spear_result_clone(ret_scope ? ret_scope : &_scope_0, %s);", value.code);
        } else {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "only num/text/result function returns are supported");
        }
        expect(parser, TOK_SEMI, "expected ';'");
        for (int i = parser->active_scope_count - 1; i >= 0; i--) {
            char *scope_name = make_scope_name(parser->active_scope_ids[i]);
            emit_line(parser, "spear_scope_leave(&%s);", scope_name);
            free(scope_name);
        }
        emit_line(parser, "goto _spear_done;");
        return true;
    }

    if (match(parser, TOK_BREAK)) {
        if (parser->loop_depth <= 0) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "break is only valid inside loops");
        }
        expect(parser, TOK_SEMI, "expected ';' after break");
        for (int i = parser->active_scope_count - 1; i >= 0; i--) {
            if (parser->active_scope_loop_depths[i] < parser->loop_depth) {
                break;
            }
            char *scope_name = make_scope_name(parser->active_scope_ids[i]);
            emit_line(parser, "spear_scope_leave(&%s);", scope_name);
            free(scope_name);
        }
        emit_line(parser, "break;");
        return true;
    }

    if (match(parser, TOK_CONTINUE)) {
        if (parser->loop_depth <= 0) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "continue is only valid inside loops");
        }
        expect(parser, TOK_SEMI, "expected ';' after continue");
        for (int i = parser->active_scope_count - 1; i >= 0; i--) {
            if (parser->active_scope_loop_depths[i] < parser->loop_depth) {
                break;
            }
            char *scope_name = make_scope_name(parser->active_scope_ids[i]);
            emit_line(parser, "spear_scope_leave(&%s);", scope_name);
            free(scope_name);
        }
        emit_line(parser, "continue;");
        return true;
    }

    if (parser->lexer.current.kind == TOK_THROW) {
        Token throw_tok = parser->lexer.current;
        advance(parser);
        expect(parser, TOK_LPAREN, "expected '(' after throw");
        Expr message = parse_text_expr(parser, scope_id);
        expect(parser, TOK_RPAREN, "expected ')'");
        expect(parser, TOK_SEMI, "expected ';' after throw");
        emit_line(parser, "spear_runtime_fail_at(%d, %d, %s);", throw_tok.line, throw_tok.col, message.code);
        return true;
    }

    if (match(parser, TOK_TRY)) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "try/catch is reserved but not implemented yet");
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
        value = parse_value_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "%s = %s;", name, value.code);
        return false;
    }

    fatal_at(parser->lexer.current.line, parser->lexer.current.col, "unexpected statement");
    return false;
}

static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope) {
    expect(parser, TOK_LBRACE, "expected '{'");

    int scope_id = parent_scope_id;
    emit_line(parser, "{");
    parser->depth++;
    if (creates_scope) {
        scope_id = ++parser->scope_counter;
        parser->active_scope_ids[parser->active_scope_count++] = scope_id;
        parser->active_scope_loop_depths[parser->active_scope_count - 1] = parser->loop_depth;
        char *scope_name = make_scope_name(scope_id);
        char *parent_name = make_scope_name(parent_scope_id);
        emit_line(parser, "SpearScope %s = spear_scope_enter(&%s);", scope_name, parent_name);
        free(scope_name);
        free(parent_name);
    }

    int symbol_depth = parser->depth;
    bool terminated = false;
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        parse_statement_with_recovery(parser, scope_id, &terminated);
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
"#include <limits.h>\n"
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <string.h>\n"
"#include <direct.h>\n"
"#include <windows.h>\n"
"\n"
"static const char *SPEAR_TOOL_DIR;\n"
"static int SPEAR_ARGC;\n"
"static char **SPEAR_ARGV;\n"
"\n"
"static int spear_eq(const char *a, const char *b) {\n"
"    return strcmp(a, b) == 0;\n"
"}\n"
"\n"
"static const char *spear_lang_code(void) {\n"
"    static char code[8] = \"en\";\n"
"    static int loaded = 0;\n"
"    char path[2048];\n"
"    FILE *fp;\n"
"    if (loaded) return code;\n"
"    loaded = 1;\n"
"    snprintf(path, sizeof(path), \"%s\\\\spear-lang.txt\", SPEAR_TOOL_DIR);\n"
"    fp = fopen(path, \"rb\");\n"
"    if (!fp) return code;\n"
"    if (fgets(code, sizeof(code), fp)) {\n"
"        for (size_t i = 0; code[i]; i++) {\n"
"            if (code[i] == '\\r' || code[i] == '\\n') { code[i] = '\\0'; break; }\n"
"        }\n"
"        if (!(spear_eq(code, \"en\") || spear_eq(code, \"ko\"))) {\n"
"            strcpy(code, \"en\");\n"
"        }\n"
"    }\n"
"    fclose(fp);\n"
"    return code;\n"
"}\n"
"\n"
"static const char *spear_runtime_prefix(void) {\n"
"    const char *lang = spear_lang_code();\n"
"    if (spear_eq(lang, \"ko\")) return \"spear 런타임 오류\";\n"
"    if (spear_eq(lang, \"ja\")) return \"spear ランタイムエラー\";\n"
"    if (spear_eq(lang, \"zh\")) return \"spear 运行时错误\";\n"
"    return \"spear runtime error\";\n"
"}\n"
"\n"
"static const char *spear_localize_message(const char *message) {\n"
"    const char *lang = spear_lang_code();\n"
"    if (spear_eq(lang, \"en\")) return message;\n"
"    if (spear_eq(message, \"out of memory\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"메모리가 부족합니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"メモリが不足しています\";\n"
"        if (spear_eq(lang, \"zh\")) return \"内存不足\";\n"
"    }\n"
"    if (spear_eq(message, \"cannot resolve temp directory\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"임시 디렉터리를 찾을 수 없습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"一時ディレクトリを解決できません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"无法解析临时目录\";\n"
"    }\n"
"    if (spear_eq(message, \"cannot create temp file\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"임시 파일을 만들 수 없습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"一時ファイルを作成できません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"无法创建临时文件\";\n"
"    }\n"
"    if (spear_eq(message, \"unsafe bridge target name\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"브리지 대상 이름이 안전하지 않습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"ブリッジ対象名が安全ではありません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"桥接目标名称不安全\";\n"
"    }\n"
"    if (spear_eq(message, \"unsafe bridge function name\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"브리지 함수 이름이 안전하지 않습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"ブリッジ関数名が安全ではありません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"桥接函数名称不安全\";\n"
"    }\n"
"    if (spear_eq(message, \"unsafe tag name\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"태그 이름이 안전하지 않습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"タグ名が安全ではありません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"标签名称不安全\";\n"
"    }\n"
"    if (spear_eq(message, \"unsafe action href\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"action 링크가 안전하지 않습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"action リンクが安全ではありません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"action 链接不安全\";\n"
"    }\n"
"    if (spear_eq(message, \"division by zero\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"0으로 나눌 수 없습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"0 では割れません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"不能除以 0\";\n"
"    }\n"
"    if (spear_eq(message, \"modulo by zero\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"0으로 나머지를 계산할 수 없습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"0 で剰余を計算できません\";\n"
"        if (spear_eq(lang, \"zh\")) return \"不能对 0 取模\";\n"
"    }\n"
"    if (spear_eq(message, \"numeric overflow in unary '-'\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"단항 '-' 계산 중 숫자 오버플로가 발생했습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"単項 '-' の計算で数値オーバーフローが発生しました\";\n"
"        if (spear_eq(lang, \"zh\")) return \"一元 '-' 计算时发生数值溢出\";\n"
"    }\n"
"    if (spear_eq(message, \"numeric overflow in '+'\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"'+' 계산 중 숫자 오버플로가 발생했습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"'+' の計算で数値オーバーフローが発生しました\";\n"
"        if (spear_eq(lang, \"zh\")) return \"'+' 计算时发生数值溢出\";\n"
"    }\n"
"    if (spear_eq(message, \"numeric overflow in '-'\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"'-' 계산 중 숫자 오버플로가 발생했습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"'-' の計算で数値オーバーフローが発生しました\";\n"
"        if (spear_eq(lang, \"zh\")) return \"'-' 计算时发生数值溢出\";\n"
"    }\n"
"    if (spear_eq(message, \"numeric overflow in '*'\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"'*' 계산 중 숫자 오버플로가 발생했습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"'*' の計算で数値オーバーフローが発生しました\";\n"
"        if (spear_eq(lang, \"zh\")) return \"'*' 计算时发生数值溢出\";\n"
"    }\n"
"    if (spear_eq(message, \"numeric overflow in '/'\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"'/' 계산 중 숫자 오버플로가 발생했습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"'/' の計算で数値オーバーフローが発生しました\";\n"
"        if (spear_eq(lang, \"zh\")) return \"'/' 计算时发生数值溢出\";\n"
"    }\n"
"    if (spear_eq(message, \"numlist index out of bounds\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"numlist 인덱스가 범위를 벗어났습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"numlist のインデックスが範囲外です\";\n"
"        if (spear_eq(lang, \"zh\")) return \"numlist 索引越界\";\n"
"    }\n"
"    if (spear_eq(message, \"textlist index out of bounds\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"textlist 인덱스가 범위를 벗어났습니다\";\n"
"        if (spear_eq(lang, \"ja\")) return \"textlist のインデックスが範囲外です\";\n"
"        if (spear_eq(lang, \"zh\")) return \"textlist 索引越界\";\n"
"    }\n"
"    return message;\n"
"}\n"
"\n"
"static const char *spear_localize_path_prefix(const char *prefix) {\n"
"    const char *lang = spear_lang_code();\n"
"    if (spear_eq(lang, \"en\")) return prefix;\n"
"    if (spear_eq(prefix, \"cannot read\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"읽을 수 없습니다:\";\n"
"        if (spear_eq(lang, \"ja\")) return \"読み込めません:\";\n"
"        if (spear_eq(lang, \"zh\")) return \"无法读取:\";\n"
"    }\n"
"    if (spear_eq(prefix, \"failed to read\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"읽기에 실패했습니다:\";\n"
"        if (spear_eq(lang, \"ja\")) return \"読み込みに失敗しました:\";\n"
"        if (spear_eq(lang, \"zh\")) return \"读取失败:\";\n"
"    }\n"
"    if (spear_eq(prefix, \"cannot write\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"쓸 수 없습니다:\";\n"
"        if (spear_eq(lang, \"ja\")) return \"書き込めません:\";\n"
"        if (spear_eq(lang, \"zh\")) return \"无法写入:\";\n"
"    }\n"
"    if (spear_eq(prefix, \"failed to write\")) {\n"
"        if (spear_eq(lang, \"ko\")) return \"쓰기에 실패했습니다:\";\n"
"        if (spear_eq(lang, \"ja\")) return \"書き込みに失敗しました:\";\n"
"        if (spear_eq(lang, \"zh\")) return \"写入失败:\";\n"
"    }\n"
"    return prefix;\n"
"}\n"
"\n"
"static void spear_runtime_fail(const char *message) {\n"
"    fprintf(stderr, \"%s: %s\\n\", spear_runtime_prefix(), spear_localize_message(message));\n"
"    exit(1);\n"
"}\n"
"\n"
"static void spear_runtime_fail_at(int line, int col, const char *message) {\n"
"    fprintf(stderr, \"%s [%d:%d]: %s\\n\", spear_runtime_prefix(), line, col, spear_localize_message(message));\n"
"    exit(1);\n"
"}\n"
"\n"
"static void spear_runtime_fail_path(const char *prefix, const char *path) {\n"
"    fprintf(stderr, \"%s: %s %s\\n\", spear_runtime_prefix(), spear_localize_path_prefix(prefix), path);\n"
"    exit(1);\n"
"}\n"
"\n"
"typedef struct SpearScope SpearScope;\n"
"static void *spear_alloc(SpearScope *scope, size_t size);\n"
"static void spear_write_text(const char *path, const char *content);\n"
"\n"
"static int spear_bridge_name_ok(const char *value) {\n"
"    if (!value || !value[0]) return 0;\n"
"    for (size_t i = 0; value[i]; i++) {\n"
"        char c = value[i];\n"
"        if (!(isalnum((unsigned char) c) || c == '_' || c == '-' || c == '/' || c == '.' || c == '@')) return 0;\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_read_text_file(SpearScope *scope, const char *path) {\n"
"    FILE *fp = fopen(path, \"rb\");\n"
"    if (!fp) {\n"
"        spear_runtime_fail_path(\"cannot read\", path);\n"
"    }\n"
"    fseek(fp, 0, SEEK_END);\n"
"    long size = ftell(fp);\n"
"    rewind(fp);\n"
"    char *dst = (char *) spear_alloc(scope, (size_t) size + 1);\n"
"    if (fread(dst, 1, (size_t) size, fp) != (size_t) size) {\n"
"        fclose(fp);\n"
"        spear_runtime_fail_path(\"failed to read\", path);\n"
"    }\n"
"    fclose(fp);\n"
"    dst[size] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static void spear_temp_file(char *path, size_t cap, const char *prefix) {\n"
"    char temp_root[MAX_PATH];\n"
"    char name[MAX_PATH];\n"
"    if (!GetTempPathA(sizeof(temp_root), temp_root)) {\n"
"        spear_runtime_fail(\"cannot resolve temp directory\");\n"
"    }\n"
"    if (!GetTempFileNameA(temp_root, prefix, 0, name)) {\n"
"        spear_runtime_fail(\"cannot create temp file\");\n"
"    }\n"
"    snprintf(path, cap, \"%s\", name);\n"
"}\n"
"\n"
"static char *spear_bridge_call(SpearScope *scope, const char *runner, const char *script_leaf, const char *target, const char *fn, const char *payload) {\n"
"    char req_path[MAX_PATH];\n"
"    char res_path[MAX_PATH];\n"
"    char script_path[2048];\n"
"    char command[8192];\n"
"    int exit_code;\n"
"    if (!spear_bridge_name_ok(target)) {\n"
"        spear_runtime_fail(\"unsafe bridge target name\");\n"
"    }\n"
"    if (!spear_bridge_name_ok(fn)) {\n"
"        spear_runtime_fail(\"unsafe bridge function name\");\n"
"    }\n"
"    spear_temp_file(req_path, sizeof(req_path), \"spr\");\n"
"    spear_temp_file(res_path, sizeof(res_path), \"spr\");\n"
"    spear_write_text(req_path, payload);\n"
"    snprintf(script_path, sizeof(script_path), \"%s\\\\runtime\\\\%s\", SPEAR_TOOL_DIR, script_leaf);\n"
"    snprintf(command, sizeof(command), \"%s \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\"\", runner, script_path, target, fn, req_path, res_path);\n"
"    exit_code = system(command);\n"
"    char *response = spear_read_text_file(scope, res_path);\n"
"    DeleteFileA(req_path);\n"
"    DeleteFileA(res_path);\n"
"    if (exit_code != 0) {\n"
"        spear_runtime_fail_at(1, 1, response);\n"
"    }\n"
"    return response;\n"
"}\n"
"\n"
"static char *spear_node_call(SpearScope *scope, const char *target, const char *fn, const char *payload) {\n"
"    return spear_bridge_call(scope, \"node\", \"bridge_node.mjs\", target, fn, payload);\n"
"}\n"
"\n"
"static char *spear_py_call(SpearScope *scope, const char *target, const char *fn, const char *payload) {\n"
"    return spear_bridge_call(scope, \"python\", \"bridge_python.py\", target, fn, payload);\n"
"}\n"
"\n"
"static long long spear_checked_neg(long long value, int line, int col) {\n"
"    if (value == LLONG_MIN) {\n"
"        spear_runtime_fail_at(line, col, \"numeric overflow in unary '-'\");\n"
"    }\n"
"    return -value;\n"
"}\n"
"\n"
"static long long spear_checked_add(long long left, long long right, int line, int col) {\n"
"    __int128 result = (__int128) left + (__int128) right;\n"
"    if (result > LLONG_MAX || result < LLONG_MIN) {\n"
"        spear_runtime_fail_at(line, col, \"numeric overflow in '+'\");\n"
"    }\n"
"    return (long long) result;\n"
"}\n"
"\n"
"static long long spear_checked_sub(long long left, long long right, int line, int col) {\n"
"    __int128 result = (__int128) left - (__int128) right;\n"
"    if (result > LLONG_MAX || result < LLONG_MIN) {\n"
"        spear_runtime_fail_at(line, col, \"numeric overflow in '-'\");\n"
"    }\n"
"    return (long long) result;\n"
"}\n"
"\n"
"static long long spear_checked_mul(long long left, long long right, int line, int col) {\n"
"    __int128 result = (__int128) left * (__int128) right;\n"
"    if (result > LLONG_MAX || result < LLONG_MIN) {\n"
"        spear_runtime_fail_at(line, col, \"numeric overflow in '*'\");\n"
"    }\n"
"    return (long long) result;\n"
"}\n"
"\n"
"static long long spear_checked_div(long long left, long long right, int line, int col) {\n"
"    if (right == 0) {\n"
"        spear_runtime_fail_at(line, col, \"division by zero\");\n"
"    }\n"
"    if (left == LLONG_MIN && right == -1) {\n"
"        spear_runtime_fail_at(line, col, \"numeric overflow in '/'\");\n"
"    }\n"
"    return left / right;\n"
"}\n"
"\n"
"static long long spear_checked_mod(long long left, long long right, int line, int col) {\n"
"    if (right == 0) {\n"
"        spear_runtime_fail_at(line, col, \"modulo by zero\");\n"
"    }\n"
"    return left % right;\n"
"}\n"
"\n"
"typedef struct SpearChunk {\n"
"    struct SpearChunk *next;\n"
"    size_t used;\n"
"    size_t cap;\n"
"    char data[];\n"
"} SpearChunk;\n"
"\n"
"struct SpearScope {\n"
"    struct SpearScope *parent;\n"
"    SpearChunk *chunks;\n"
"    struct SpearCleanup *cleanups;\n"
"};\n"
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
"typedef struct SpearMap {\n"
"    SpearScope *owner;\n"
"    char **keys;\n"
"    char **values;\n"
"    size_t len;\n"
"    size_t cap;\n"
"} SpearMap;\n"
"\n"
"typedef struct SpearResult {\n"
"    SpearScope *owner;\n"
"    int ok;\n"
"    char *value;\n"
"    char *error;\n"
"} SpearResult;\n"
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
"            spear_runtime_fail(\"out of memory\");\n"
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
"static char *spear_text_from_num(SpearScope *scope, long long value) {\n"
"    char temp[32];\n"
"    sprintf(temp, \"%lld\", value);\n"
"    return spear_text_clone(scope, temp);\n"
"}\n"
"\n"
"static char *spear_style_chain(SpearScope *scope, const char *base, const char *mods) {\n"
"    if (!base) base = \"\";\n"
"    if (!mods) mods = \"\";\n"
"    if (!base[0]) {\n"
"        return spear_text_clone(scope, mods);\n"
"    }\n"
"    if (!mods[0]) {\n"
"        return spear_text_clone(scope, base);\n"
"    }\n"
"    return spear_text_join(scope, base, mods);\n"
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
"static char *spear_markup_attrs(SpearScope *scope, const char *tag, const char *attrs, const char *content) {\n"
"    if (!spear_tag_is_safe(tag)) {\n"
"        spear_runtime_fail(\"unsafe tag name\");\n"
"    }\n"
"    size_t tag_len = strlen(tag);\n"
"    size_t attrs_len = (attrs && attrs[0]) ? strlen(attrs) + 1 : 0;\n"
"    size_t body_len = strlen(content);\n"
"    char *dst = (char *) spear_alloc(scope, tag_len * 2 + attrs_len + body_len + 8);\n"
"    if (attrs_len) {\n"
"        sprintf(dst, \"<%s %s>%s</%s>\", tag, attrs, content, tag);\n"
"    } else {\n"
"        sprintf(dst, \"<%s>%s</%s>\", tag, content, tag);\n"
"    }\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_markup(SpearScope *scope, const char *tag, const char *content) {\n"
"    return spear_markup_attrs(scope, tag, \"\", content);\n"
"}\n"
"\n"
"static int spear_attr_name_is_safe(const char *name) {\n"
"    if (!name || !name[0]) return 0;\n"
"    for (size_t i = 0; name[i]; i++) {\n"
"        char c = name[i];\n"
"        if (!(isalnum((unsigned char) c) || c == '-' || c == '_')) return 0;\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_attr(SpearScope *scope, const char *name, const char *value) {\n"
"    if (!spear_attr_name_is_safe(name)) {\n"
"        spear_runtime_fail(\"unsafe attribute name\");\n"
"    }\n"
"    char *safe_value = spear_html_escape(scope, value);\n"
"    size_t len = strlen(name) + strlen(safe_value) + 4;\n"
"    char *dst = (char *) spear_alloc(scope, len + 1);\n"
"    sprintf(dst, \"%s=\\\"%s\\\"\", name, safe_value);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_attrs(SpearScope *scope, const char *left, const char *right) {\n"
"    if (!left[0]) return spear_text_clone(scope, right);\n"
"    if (!right[0]) return spear_text_clone(scope, left);\n"
"    size_t len = strlen(left) + strlen(right) + 2;\n"
"    char *dst = (char *) spear_alloc(scope, len + 1);\n"
"    sprintf(dst, \"%s %s\", left, right);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_column(SpearScope *scope, const char *content) {\n"
"    return spear_markup_attrs(scope, \"div\", \"style=\\\"display:flex;flex-direction:column;gap:12px\\\"\", content);\n"
"}\n"
"\n"
"static char *spear_row(SpearScope *scope, const char *content) {\n"
"    return spear_markup_attrs(scope, \"div\", \"style=\\\"display:flex;align-items:center;gap:12px;flex-wrap:wrap\\\"\", content);\n"
"}\n"
"\n"
"static char *spear_page(SpearScope *scope, const char *title, const char *body) {\n"
"    char *safe_title = spear_html_escape(scope, title);\n"
"    const char *prefix = \"<!doctype html><html><head><meta charset=\\\"utf-8\\\"><meta name=\\\"viewport\\\" content=\\\"width=device-width,initial-scale=1\\\"><title>\";\n"
"    const char *middle = \"</title><style>*{box-sizing:border-box}html{-webkit-text-size-adjust:100%%;-ms-text-size-adjust:100%%}html,body{margin:0;padding:0}body{font-family:Georgia,\\\"Times New Roman\\\",serif;line-height:1.5;background:#ffffff;color:#111827}h1,h2,h3,h4,h5,h6,p,blockquote,figure,pre,ul,ol,dl{margin:0;padding:0}ul,ol{padding-left:1.25em}img,svg,video,canvas{display:block;max-width:100%%;height:auto;border:0}a{text-decoration:none;color:inherit}button,input,textarea,select{font:inherit;color:inherit}table{border-collapse:collapse;border-spacing:0}</style></head><body>\";\n"
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
"        spear_runtime_fail(\"unsafe action href\");\n"
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
"        spear_runtime_fail_path(\"cannot write\", path);\n"
"    }\n"
"    size_t len = strlen(content);\n"
"    if (fwrite(content, 1, len, fp) != len) {\n"
"        spear_runtime_fail_path(\"failed to write\", path);\n"
"        fclose(fp);\n"
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
"        spear_runtime_fail(\"out of memory\");\n"
"    }\n"
"    while ((ch = getchar()) != EOF && ch != '\\n') {\n"
"        if (len + 1 >= cap) {\n"
"            cap *= 2;\n"
"            temp = (char *) realloc(temp, cap);\n"
"            if (!temp) {\n"
"                spear_runtime_fail(\"out of memory\");\n"
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
"    if (!list) { spear_runtime_fail(\"out of memory\"); }\n"
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
"    if (!list) { spear_runtime_fail(\"out of memory\"); }\n"
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
"        if (!items) { spear_runtime_fail(\"out of memory\"); }\n"
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
"        if (!items) { spear_runtime_fail(\"out of memory\"); }\n"
"        list->items = items;\n"
"        list->cap = next;\n"
"    }\n"
"    list->items[list->len++] = spear_text_clone(list->owner, value);\n"
"}\n"
"\n"
"static void spear_map_drop(void *ptr) {\n"
"    SpearMap *map = (SpearMap *) ptr;\n"
"    free(map->keys);\n"
"    free(map->values);\n"
"    free(map);\n"
"}\n"
"\n"
"static SpearMap *spear_map_new(SpearScope *scope) {\n"
"    SpearMap *map = (SpearMap *) calloc(1, sizeof(SpearMap));\n"
"    if (!map) { spear_runtime_fail(\"out of memory\"); }\n"
"    map->owner = scope;\n"
"    spear_scope_add_cleanup(scope, spear_map_drop, map);\n"
"    return map;\n"
"}\n"
"\n"
"static size_t spear_map_find(SpearMap *map, const char *key) {\n"
"    for (size_t i = 0; i < map->len; i++) {\n"
"        if (strcmp(map->keys[i], key) == 0) return i;\n"
"    }\n"
"    return (size_t) -1;\n"
"}\n"
"\n"
"static void spear_map_set(SpearMap *map, const char *key, const char *value) {\n"
"    size_t index = spear_map_find(map, key);\n"
"    if (index != (size_t) -1) {\n"
"        map->values[index] = spear_text_clone(map->owner, value);\n"
"        return;\n"
"    }\n"
"    if (map->len == map->cap) {\n"
"        size_t next = map->cap ? map->cap * 2 : 8;\n"
"        char **keys = (char **) realloc(map->keys, next * sizeof(char *));\n"
"        char **values = (char **) realloc(map->values, next * sizeof(char *));\n"
"        if (!keys || !values) { spear_runtime_fail(\"out of memory\"); }\n"
"        map->keys = keys;\n"
"        map->values = values;\n"
"        map->cap = next;\n"
"    }\n"
"    map->keys[map->len] = spear_text_clone(map->owner, key);\n"
"    map->values[map->len] = spear_text_clone(map->owner, value);\n"
"    map->len++;\n"
"}\n"
"\n"
"static long long spear_map_has(SpearMap *map, const char *key, int line, int col) {\n"
"    (void) line;\n"
"    (void) col;\n"
"    return spear_map_find(map, key) != (size_t) -1;\n"
"}\n"
"\n"
"static char *spear_map_get(SpearScope *scope, SpearMap *map, const char *key, const char *fallback, int line, int col) {\n"
"    (void) line;\n"
"    (void) col;\n"
"    size_t index = spear_map_find(map, key);\n"
"    if (index == (size_t) -1) return spear_text_clone(scope, fallback);\n"
"    return spear_text_clone(scope, map->values[index]);\n"
"}\n"
"\n"
"static void spear_map_remove(SpearMap *map, const char *key) {\n"
"    size_t index = spear_map_find(map, key);\n"
"    if (index == (size_t) -1) return;\n"
"    for (size_t i = index + 1; i < map->len; i++) {\n"
"        map->keys[i - 1] = map->keys[i];\n"
"        map->values[i - 1] = map->values[i];\n"
"    }\n"
"    map->len--;\n"
"}\n"
"\n"
"static long long spear_map_count(SpearMap *map) { return (long long) map->len; }\n"
"\n"
"static SpearResult *spear_result_make(SpearScope *scope, int ok, const char *value, const char *error) {\n"
"    SpearResult *result = (SpearResult *) spear_alloc(scope, sizeof(SpearResult));\n"
"    result->owner = scope;\n"
"    result->ok = ok;\n"
"    result->value = spear_text_clone(scope, value ? value : \"\");\n"
"    result->error = spear_text_clone(scope, error ? error : \"\");\n"
"    return result;\n"
"}\n"
"\n"
"static SpearResult *spear_result_ok(SpearScope *scope, const char *value) {\n"
"    return spear_result_make(scope, 1, value, \"\");\n"
"}\n"
"\n"
"static SpearResult *spear_result_fail(SpearScope *scope, const char *error) {\n"
"    return spear_result_make(scope, 0, \"\", error);\n"
"}\n"
"\n"
"static SpearResult *spear_result_clone(SpearScope *scope, SpearResult *result) {\n"
"    return spear_result_make(scope, result->ok, result->value, result->error);\n"
"}\n"
"\n"
"static long long spear_result_is_ok(SpearResult *result, int line, int col) {\n"
"    (void) line;\n"
"    (void) col;\n"
"    return result->ok ? 1 : 0;\n"
"}\n"
"\n"
"static char *spear_result_unwrap(SpearScope *scope, SpearResult *result, int line, int col) {\n"
"    if (!result->ok) {\n"
"        spear_runtime_fail_at(line, col, result->error[0] ? result->error : \"result is an error\");\n"
"    }\n"
"    return spear_text_clone(scope, result->value);\n"
"}\n"
"\n"
"static char *spear_result_error_text(SpearScope *scope, SpearResult *result, int line, int col) {\n"
"    (void) line;\n"
"    (void) col;\n"
"    return spear_text_clone(scope, result->error);\n"
"}\n"
"\n"
"static long long spear_numlist_count(SpearNumList *list) { return (long long) list->len; }\n"
"static long long spear_textlist_count(SpearTextList *list) { return (long long) list->len; }\n"
"\n"
"static long long spear_arg_count(void) {\n"
"    return SPEAR_ARGC > 0 ? (long long) (SPEAR_ARGC - 1) : 0;\n"
"}\n"
"\n"
"static char *spear_arg_at(SpearScope *scope, long long index, int line, int col) {\n"
"    if (index < 0 || index >= spear_arg_count()) { spear_runtime_fail_at(line, col, \"argument index out of bounds\"); }\n"
"    return spear_text_clone(scope, SPEAR_ARGV[index + 1]);\n"
"}\n"
"\n"
"static long long spear_numlist_at(SpearNumList *list, long long index, int line, int col) {\n"
"    if (index < 0 || (size_t) index >= list->len) { spear_runtime_fail_at(line, col, \"numlist index out of bounds\"); }\n"
"    return list->items[index];\n"
"}\n"
"\n"
"static char *spear_textlist_at(SpearTextList *list, long long index, int line, int col) {\n"
"    if (index < 0 || (size_t) index >= list->len) { spear_runtime_fail_at(line, col, \"textlist index out of bounds\"); }\n"
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
    normalize_import_leaf(clean_leaf, sizeof(clean_leaf), leaf);
    join_fs_path(candidate, sizeof(candidate), base, clean_leaf);
    if (file_exists(candidate)) {
        checked_snprintf(out, cap, "%s", candidate);
        return;
    }

    if (g_tool_dir[0] &&
        (strncmp(clean_leaf, "std/", 4) == 0 || strncmp(clean_leaf, "std\\", 4) == 0)) {
        checked_snprintf(candidate, sizeof(candidate), "%s\\%s", g_tool_dir, clean_leaf);
        if (file_exists(candidate)) {
            checked_snprintf(out, cap, "%s", candidate);
            return;
        }
    }

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
    return data;
}

static void collect_exported_functions_recursive(const char *path, StringList *seen_files, StringList *exports) {
    char full_path[2048];
    char clean_path[2048];
    char base_dir[2048];
    normalize_import_leaf(clean_path, sizeof(clean_path), path);
    if (!_fullpath(full_path, clean_path, sizeof(full_path))) {
        return;
    }
    if (string_list_has(seen_files, full_path)) {
        return;
    }
    string_list_add(seen_files, full_path);
    char *source = read_file(full_path);
    parent_dir_of(full_path, base_dir, sizeof(base_dir));

    Lexer lexer;
    lexer_init(&lexer, source);
    for (;;) {
        Token token = lexer_next(&lexer);
        if (token.kind == TOK_EOF) {
            break;
        }
        if (token.kind == TOK_IMPORT) {
            Token path_tok = lexer_next(&lexer);
            if (path_tok.kind != TOK_STRING) {
                break;
            }
            Token semi = lexer_next(&lexer);
            if (semi.kind != TOK_SEMI) {
                break;
            }
            char *rel = token_text(path_tok);
            char next_path[2048];
            resolve_import_path(next_path, sizeof(next_path), base_dir, rel);
            collect_exported_functions_recursive(next_path, seen_files, exports);
            free(rel);
            continue;
        }
        if (token.kind == TOK_MODULE || token.kind == TOK_PACKAGE) {
            Token next = lexer_next(&lexer);
            Token semi = lexer_next(&lexer);
            if (next.kind != TOK_IDENT || semi.kind != TOK_SEMI) {
                break;
            }
            continue;
        }
        if (token.kind == TOK_FUNCTION) {
            token = lexer_next(&lexer);
        }
        if (token.kind == TOK_RUN) {
            scan_skip_block(&lexer);
            continue;
        }
        if (token.kind == TOK_SPEAR || token.kind == TOK_NUM || token.kind == TOK_TEXT || token.kind == TOK_NUMLIST || token.kind == TOK_TEXTLIST || token.kind == TOK_VIEW) {
            Token name_tok = lexer_next(&lexer);
            if (name_tok.kind == TOK_IDENT) {
                char *name = token_text(name_tok);
                string_list_add(exports, name);
                free(name);
            }
            scan_skip_definition_body(&lexer);
        }
    }

    free(source);
}

static ImportInfo *collect_direct_imports(const char *path, const char *root_source, size_t *out_count) {
    char full_path[2048];
    char clean_path[2048];
    char base_dir[2048];
    char *source;
    ImportInfo *items = NULL;
    size_t count = 0;
    size_t cap = 0;

    *out_count = 0;
    normalize_import_leaf(clean_path, sizeof(clean_path), path);
    if (!_fullpath(full_path, clean_path, sizeof(full_path))) {
        return NULL;
    }
    source = root_source ? xstrdup(root_source) : read_file(full_path);
    parent_dir_of(full_path, base_dir, sizeof(base_dir));

    const char *cursor = source;
    int current_line = 1;
    while (*cursor) {
        const char *line_end = strchr(cursor, '\n');
        size_t line_len = line_end ? (size_t) (line_end - cursor) : strlen(cursor);
        const char *trim = cursor;
        while (*trim == ' ' || *trim == '\t') trim++;
        if (strncmp(trim, "import \"", 8) == 0) {
            const char *start = strchr(trim, '"');
            if (start) start++;
            const char *end = start ? strchr(start, '"') : NULL;
            if (start && end && end[1] == ';') {
                char rel[1024];
                char next_path[2048];
                size_t rel_len = (size_t) (end - start);
                memcpy(rel, start, rel_len);
                rel[rel_len] = '\0';
                resolve_import_path(next_path, sizeof(next_path), base_dir, rel);
                import_list_add(&items, &count, &cap, next_path, rel, current_line);
                StringList seen_files = {0};
                StringList exports = {0};
                collect_exported_functions_recursive(next_path, &seen_files, &exports);
                for (size_t i = 0; i < exports.count; i++) {
                    import_export_add(&items[count - 1], exports.items[i]);
                }
            }
        }
        current_line++;
        cursor += line_len;
        if (*cursor == '\n') {
            cursor++;
        }
    }

    free(source);
    *out_count = count;
    return items;
}

static char *load_source_tree(const char *path, const char *root_source, PathList *seen, PathList *active) {
    char full_path[2048];
    char clean_path[2048];
    char base_dir[2048];
    normalize_import_leaf(clean_path, sizeof(clean_path), path);
    if (!_fullpath(full_path, clean_path, sizeof(full_path))) {
        fprintf(stderr, "%s: %s %s\n", compiler_message("error_prefix"), compiler_message("cannot_resolve"), clean_path);
        exit(1);
    }
    if (path_list_has(active, full_path)) {
        fprintf(stderr, "spearc error: circular import detected at %s\n", full_path);
        exit(1);
    }
    if (path_list_has(seen, full_path)) {
        return xstrdup("");
    }
    path_list_add(seen, full_path);
    path_list_add(active, full_path);
    char *source = root_source ? xstrdup(root_source) : read_file(full_path);
    parent_dir_of(full_path, base_dir, sizeof(base_dir));

    Buffer out;
    buf_init(&out);

    const char *cursor = source;
    int current_line = 1;
    while (*cursor) {
        const char *line_end = strchr(cursor, '\n');
        size_t line_len = line_end ? (size_t) (line_end - cursor) : strlen(cursor);
        const char *trim = cursor;
        while (*trim == ' ' || *trim == '\t') trim++;

        if (strncmp(trim, "import \"", 8) == 0) {
            const char *start = strchr(trim, '"');
            if (start) start++;
            const char *end = start ? strchr(start, '"') : NULL;
            if (!start || !end || end[1] != ';') {
                fprintf(stderr, "spearc error: malformed import in %s\n", full_path);
                exit(1);
            }
            char rel[1024];
            char next_path[2048];
            char full_child[2048];
            size_t rel_len = (size_t) (end - start);
            memcpy(rel, start, rel_len);
            rel[rel_len] = '\0';
            resolve_import_path(next_path, sizeof(next_path), base_dir, rel);
            if (_fullpath(full_child, next_path, sizeof(full_child)) && path_list_has(seen, full_child)) {
                fprintf(stderr, "%s [line %d:%d] ", compiler_message("warning_prefix"), current_line, 1);
                fprintf(stderr, compiler_message("duplicate_import"), rel);
                fputc('\n', stderr);
            }
            char *child = load_source_tree(next_path, NULL, seen, active);
            append_line_directive(&out, 1);
            buf_append(&out, child);
            if (out.len && out.data[out.len - 1] != '\n') {
                buf_append(&out, "\n");
            }
            append_line_directive(&out, current_line + 1);
            free(child);
        } else {
            buf_reserve(&out, line_len + 1);
            memcpy(out.data + out.len, cursor, line_len);
            out.len += line_len;
            out.data[out.len] = '\0';
            if (line_end) {
                buf_append(&out, "\n");
            }
        }

        if (!line_end) break;
        cursor = line_end + 1;
        current_line++;
    }

    free(source);
    path_list_remove(active, full_path);
    return buf_take(&out);
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
    if (len > 3 && strcmp(input_path + len - 3, ".sp") == 0) {
        char *base = slice_dup(input_path, len - 3);
        buf_appendf(&out, "%s.c", base);
        free(base);
    } else {
        buf_appendf(&out, "%s.c", input_path);
    }
    return buf_take(&out);
}

static bool parser_has_used_function(Parser *parser, const char *name) {
    for (size_t i = 0; i < parser->function_count; i++) {
        if (strcmp(parser->functions[i].name, name) == 0) {
            return parser->functions[i].used;
        }
    }
    return false;
}

static void warn_unused_imports(Parser *parser, ImportInfo *imports, size_t import_count) {
    for (size_t i = 0; i < import_count; i++) {
        bool used = false;
        for (size_t j = 0; j < imports[i].exported_count; j++) {
            if (parser_has_used_function(parser, imports[i].exported_functions[j])) {
                used = true;
                break;
            }
        }
        if (!used) {
            fprintf(stderr, "%s [line %d:%d] ", compiler_message("warning_prefix"), imports[i].line, 1);
            fprintf(stderr, compiler_message("unused_import"), imports[i].label);
            fputc('\n', stderr);
        }
    }
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

static void parse_function_definition(Parser *parser) {
    bool is_entry = false;
    ValueType return_type = TYPE_NUM;
    if (match(parser, TOK_MODULE)) {
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected name after metadata");
        free(parser->current_module_name);
        parser->current_module_name = token_text(name_tok);
        expect(parser, TOK_SEMI, "expected ';' after metadata");
        return;
    }
    if (match(parser, TOK_PACKAGE)) {
        Token name_tok = parser->lexer.current;
        expect(parser, TOK_IDENT, "expected name after metadata");
        free(parser->current_package_name);
        parser->current_package_name = token_text(name_tok);
        expect(parser, TOK_SEMI, "expected ';' after metadata");
        return;
    }
    if (match(parser, TOK_CLASS)) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "class is reserved but not implemented yet");
    }
    if (match(parser, TOK_RUN)) {
        FunctionInfo *fn = find_function_exact(parser, "__spear_run_entry", parser->current_package_name, parser->current_module_name);
        if (!fn) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "internal error: missing run entry");
        }

        parser->depth = 0;
        parser->scope_counter = 0;
        parser->current_return_type = TYPE_NUM;
        parser->current_is_entry = true;
        parser->active_scope_count = 0;
        parser->symbol_count = 0;

        buf_appendf(&parser->out, "\nint %s(void)\n", fn->c_name);
        emit_line(parser, "{");
        parser->depth++;
        emit_line(parser, "SpearScope _scope_0 = spear_scope_enter(NULL);");
        emit_line(parser, "long long _spear_result_num = 0;");
        expect(parser, TOK_LBRACE, "expected '{' after run");
        bool terminated = false;
        while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
            parse_statement_with_recovery(parser, 0, &terminated);
        }
        expect(parser, TOK_RBRACE, "expected '}' after run block");
        emit_line(parser, "goto _spear_done;");
        emit_line(parser, "_spear_done:");
        pop_symbols(parser, parser->depth);
        emit_line(parser, "spear_scope_leave(&_scope_0);");
        emit_line(parser, "return _spear_result_num;");
        parser->depth--;
        emit_line(parser, "}");
        (void) fn;
        return;
    }
    match(parser, TOK_FUNCTION);
    if (match(parser, TOK_SPEAR)) {
        is_entry = true;
    } else if (is_type_token(parser->lexer.current.kind) || parser->lexer.current.kind == TOK_VIEW) {
        return_type = parser->lexer.current.kind == TOK_VIEW ? TYPE_TEXT : token_to_type(parser->lexer.current.kind);
        if (return_type != TYPE_NUM && return_type != TYPE_TEXT && return_type != TYPE_RESULT) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "top-level functions may return only num, text, or result");
        }
        advance(parser);
    } else {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "expected top-level function");
    }

    Token name_tok = parser->lexer.current;
    expect(parser, TOK_IDENT, "expected function name");
    char *name = token_text(name_tok);
    FunctionInfo *fn = find_function_exact(parser, name, parser->current_package_name, parser->current_module_name);
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
        buf_appendf(&parser->out, "\nint %s(void)\n", fn->c_name);
    } else {
        buf_appendf(&parser->out, "\n%s %s(SpearScope *ret_scope", ctype_name(return_type), fn->c_name);
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
    } else if (return_type == TYPE_TEXT) {
        emit_line(parser, "char *_spear_result_text = spear_text_clone(&_scope_0, \"\");");
    } else {
        emit_line(parser, "SpearResult *_spear_result_result = spear_result_fail(ret_scope ? ret_scope : &_scope_0, \"result was not set\");");
    }

    for (size_t i = 0; i < fn->param_count; i++) {
        add_symbol(parser, fn->params[i].name, fn->params[i].type, true, fn->params[i].line, fn->params[i].col, false, true);
    }

    expect(parser, TOK_LBRACE, "expected '{' after function header");
    bool terminated = false;
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        parse_statement_with_recovery(parser, 0, &terminated);
    }
    expect(parser, TOK_RBRACE, "expected '}' after function body");
    emit_line(parser, "goto _spear_done;");
    emit_line(parser, "_spear_done:");
    pop_symbols(parser, parser->depth);
    emit_line(parser, "spear_scope_leave(&_scope_0);");
    if (is_entry || return_type == TYPE_NUM) {
        emit_line(parser, "return _spear_result_num;");
    } else if (return_type == TYPE_TEXT) {
        emit_line(parser, "return _spear_result_text;");
    } else {
        emit_line(parser, "return _spear_result_result;");
    }
    parser->depth--;
    emit_line(parser, "}");
}

static char *compile_source(const char *source, const char *tool_dir, ImportInfo *imports, size_t import_count) {
    Parser parser;
    g_error_count = 0;
    parser_init(&parser, source);
    collect_functions(&parser, source);
    lexer_init(&parser.lexer, source);
    parser.lexer.current = lexer_next(&parser.lexer);
    buf_append(&parser.out, runtime_prelude);
    char *escaped_tool_dir = escape_c_string(tool_dir);
    buf_appendf(&parser.out, "static const char *SPEAR_TOOL_DIR = \"%s\";\n", escaped_tool_dir);
    free(escaped_tool_dir);

    char *entry = NULL;
    for (size_t i = 0; i < parser.function_count; i++) {
        emit_function_prototype(&parser.out, &parser.functions[i]);
        if (parser.functions[i].is_entry) {
            entry = parser.functions[i].c_name;
        }
    }
    if (!entry && !g_collect_errors) {
        fatal_at(1, 1, "program requires one spear entry function");
    }

    while (parser.lexer.current.kind != TOK_EOF) {
        if (g_collect_errors) {
            jmp_buf env;
            jmp_buf *previous = g_recover_env;
            g_recover_env = &env;
            if (setjmp(env) == 0) {
                parse_function_definition(&parser);
                g_recover_env = previous;
            } else {
                g_recover_env = previous;
                recover_top_level(&parser);
            }
        } else {
            parse_function_definition(&parser);
        }
    }

    if (g_error_count > 0) {
        return NULL;
    }

    warn_unused_imports(&parser, imports, import_count);

    if (entry) {
        buf_appendf(&parser.out, "\nint main(int argc, char **argv) { SPEAR_ARGC = argc; SPEAR_ARGV = argv; return %s(); }\n", entry);
    }

    return buf_take(&parser.out);
}

int main(int argc, char **argv) {
    char tool_dir[2048];
    if (argc < 2 || argc > 4) {
        if (argc > 0) {
            executable_dir(argv[0], tool_dir, sizeof(tool_dir));
            compiler_load_lang(tool_dir);
            checked_snprintf(g_tool_dir, sizeof(g_tool_dir), "%s", tool_dir);
        }
        fprintf(stderr, "%s", compiler_message("usage"));
        return 1;
    }

    bool check_only = false;
    bool check_stdin = false;
    const char *input = argv[1];
    char *output = NULL;
    char *stdin_source = NULL;
    executable_dir(argv[0], tool_dir, sizeof(tool_dir));
    compiler_load_lang(tool_dir);
    checked_snprintf(g_tool_dir, sizeof(g_tool_dir), "%s", tool_dir);

    if (argc == 3 && strcmp(argv[1], "--check") == 0) {
        check_only = true;
        input = argv[2];
    } else if (argc == 3 && strcmp(argv[1], "--check-stdin") == 0) {
        check_only = true;
        check_stdin = true;
        input = argv[2];
    } else if (argc == 4) {
        if (strcmp(argv[2], "-o") != 0) {
                fprintf(stderr, "%s: expected -o before output path\n", compiler_message("error_prefix"));
            return 1;
        }
        output = xstrdup(argv[3]);
    } else if (argc == 2) {
        output = default_output_path(input);
    } else {
        fprintf(stderr, "%s", compiler_message("usage"));
        return 1;
    }
    g_collect_errors = check_only;

    PathList seen = {0};
    PathList active = {0};
    if (check_stdin) {
        stdin_source = read_stream(stdin, "stdin");
    }
    size_t direct_import_count = 0;
    ImportInfo *direct_imports = collect_direct_imports(input, stdin_source, &direct_import_count);
    char prelude_path[2048];
    char repo_prelude_path[2048];
    checked_snprintf(prelude_path, sizeof(prelude_path), "%s\\std\\prelude.sp", tool_dir);
    checked_snprintf(repo_prelude_path, sizeof(repo_prelude_path), "%s\\..\\std\\prelude.sp", tool_dir);
    char *source = load_source_tree(input, stdin_source, &seen, &active);
    FILE *prelude_fp = fopen(prelude_path, "rb");
    if (!prelude_fp) {
        prelude_fp = fopen(repo_prelude_path, "rb");
        if (prelude_fp) {
            fclose(prelude_fp);
            checked_snprintf(prelude_path, sizeof(prelude_path), "%s", repo_prelude_path);
            prelude_fp = fopen(prelude_path, "rb");
        }
    }
    if (prelude_fp) {
        fclose(prelude_fp);
        char *prelude = load_source_tree(prelude_path, NULL, &seen, &active);
        Buffer combined;
        buf_init(&combined);
        buf_append(&combined, prelude);
        if (combined.len && combined.data[combined.len - 1] != '\n') {
            buf_append(&combined, "\n");
        }
        append_line_directive(&combined, 1);
        buf_append(&combined, source);
        free(prelude);
        free(source);
        source = buf_take(&combined);
    }
    char *generated = compile_source(source, tool_dir, direct_imports, direct_import_count);
    if (!generated) {
        free(stdin_source);
        free(source);
        return 1;
    }
    if (!check_only) {
        write_file(output, generated);
        printf("generated %s\n", output);
    }

    free(stdin_source);
    free(source);
    free(generated);
    if (output) free(output);
    return 0;
}
