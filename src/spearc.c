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

static const char *compiler_message(const char *key);
#include "spearc_diag.h"

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

#include "spearc_util.h"
#include "spearc_buffer.h"
#include "spearc_string_list.h"
#include "spearc_names.h"
#include "spearc_lexer.h"

static bool parse_statement(Parser *parser, int scope_id);
#include "spearc_parser_utils.h"
#include "spearc_scan.h"
#include "spearc_symbols.h"
#include "spearc_types.h"

#include "spearc_imports.h"
#include "spearc_emit.h"
#include "spearc_frontend_scan.h"

static Expr parse_num_expr(Parser *parser, int scope_id);
static Expr parse_text_expr(Parser *parser, int scope_id);
static Expr parse_list_expr(Parser *parser, int scope_id, ValueType expected_type);
static Expr parse_map_expr(Parser *parser, int scope_id);
static Expr parse_result_expr(Parser *parser, int scope_id);
static Expr parse_value_expr(Parser *parser, int scope_id, ValueType type);
static ValueType infer_expr_type(Parser *parser);
static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope);
static bool starts_text_expr(Parser *parser);
static bool at_returns_text(Parser *parser);
static char *parse_text_children(Parser *parser, int scope_id);
#include "spearc_statement_core.h"
#include "spearc_statement_control.h"
#include "spearc_statement_tail.h"
#include "spearc_expr_num.h"
#include "spearc_expr_text.h"
#include "spearc_expr_value.h"
#include "spearc_expr_infer.h"

static bool parse_statement(Parser *parser, int scope_id) {
    if (match(parser, TOK_CLASS)) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "class is reserved but not implemented yet");
    }

    {
        bool handled = false;
        bool terminated = parse_core_statement(parser, scope_id, &handled);
        if (handled) {
            return terminated;
        }
    }

    {
        bool handled = false;
        bool terminated = parse_control_statement(parser, scope_id, &handled);
        if (handled) {
            return terminated;
        }
    }

    {
        bool handled = false;
        bool terminated = parse_tail_statement(parser, scope_id, &handled);
        if (handled) {
            return terminated;
        }
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
"#include <time.h>\n"
"#include <direct.h>\n"
"#include <winsock2.h>\n"
"#include <ws2tcpip.h>\n"
"#include <windows.h>\n"
"#include <winhttp.h>\n"
"\n"
"#define SPEAR_BCRYPT_ALG_HANDLE_HMAC_FLAG 0x00000008UL\n"
"\n"
"static const char *SPEAR_TOOL_DIR;\n"
"static int SPEAR_ARGC;\n"
"static char **SPEAR_ARGV;\n"
"\n"
"typedef struct {\n"
"    HMODULE mod;\n"
"    int started;\n"
"    int (WSAAPI *startup)(WORD, LPWSADATA);\n"
"    int (WSAAPI *cleanup)(void);\n"
"    SOCKET (WSAAPI *socket_fn)(int, int, int);\n"
"    int (WSAAPI *connect_fn)(SOCKET, const struct sockaddr *, int);\n"
"    int (WSAAPI *shutdown_fn)(SOCKET, int);\n"
"    int (WSAAPI *closesocket_fn)(SOCKET);\n"
"    int (WSAAPI *send_fn)(SOCKET, const char *, int, int);\n"
"    int (WSAAPI *recv_fn)(SOCKET, char *, int, int);\n"
"    int (WSAAPI *bind_fn)(SOCKET, const struct sockaddr *, int);\n"
"    int (WSAAPI *listen_fn)(SOCKET, int);\n"
"    SOCKET (WSAAPI *accept_fn)(SOCKET, struct sockaddr *, int *);\n"
"    int (WSAAPI *setsockopt_fn)(SOCKET, int, int, const char *, int);\n"
"    int (WSAAPI *getaddrinfo_fn)(PCSTR, PCSTR, const ADDRINFOA *, PADDRINFOA *);\n"
"    void (WSAAPI *freeaddrinfo_fn)(PADDRINFOA);\n"
"    int (WSAAPI *sendto_fn)(SOCKET, const char *, int, int, const struct sockaddr *, int);\n"
"    int (WSAAPI *recvfrom_fn)(SOCKET, char *, int, int, struct sockaddr *, int *);\n"
"} SpearSocketApi;\n"
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
"static char *spear_read_text_file(SpearScope *scope, const char *path);\n"
"static char *spear_text_clone(SpearScope *scope, const char *src);\n"
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
"static int spear_command_exists(const char *name) {\n"
"    char resolved[MAX_PATH];\n"
"    char with_ext[MAX_PATH];\n"
"    if (SearchPathA(NULL, name, NULL, (DWORD) sizeof(resolved), resolved, NULL) > 0) return 1;\n"
"    snprintf(with_ext, sizeof(with_ext), \"%s.exe\", name);\n"
"    return SearchPathA(NULL, with_ext, NULL, (DWORD) sizeof(resolved), resolved, NULL) > 0;\n"
"}\n"
"\n"
"static int spear_json_seek_key(const char *json, const char *key, const char **value_start) {\n"
"    char pattern[128];\n"
"    snprintf(pattern, sizeof(pattern), \"\\\"%s\\\"\", key);\n"
"    const char *hit = strstr(json, pattern);\n"
"    if (!hit) return 0;\n"
"    hit = strchr(hit + strlen(pattern), ':');\n"
"    if (!hit) return 0;\n"
"    hit++;\n"
"    while (*hit == ' ' || *hit == '\\t' || *hit == '\\r' || *hit == '\\n') hit++;\n"
"    *value_start = hit;\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_json_get_string(const char *json, const char *key, char *out, size_t cap) {\n"
"    const char *value = NULL;\n"
"    size_t len = 0;\n"
"    if (!spear_json_seek_key(json, key, &value) || *value != '\"') return 0;\n"
"    value++;\n"
"    while (*value && *value != '\"') {\n"
"        char ch = *value++;\n"
"        if (ch == '\\\\' && *value) {\n"
"            char esc = *value++;\n"
"            if (esc == 'n') ch = '\\n';\n"
"            else if (esc == 'r') ch = '\\r';\n"
"            else if (esc == 't') ch = '\\t';\n"
"            else ch = esc;\n"
"        }\n"
"        if (len + 1 < cap) out[len++] = ch;\n"
"    }\n"
"    if (cap > 0) out[len < cap ? len : cap - 1] = '\\0';\n"
"    return 1;\n"
"}\n"
"\n"
"static long long spear_json_get_int(const char *json, const char *key, long long fallback) {\n"
"    const char *value = NULL;\n"
"    if (!spear_json_seek_key(json, key, &value)) return fallback;\n"
"    return _strtoi64(value, NULL, 10);\n"
"}\n"
"\n"
"static int spear_json_get_raw(const char *json, const char *key, char *out, size_t cap) {\n"
"    const char *value = NULL;\n"
"    const char *end;\n"
"    int depth = 0;\n"
"    if (!spear_json_seek_key(json, key, &value)) return 0;\n"
"    end = value;\n"
"    if (*value == '\"') {\n"
"        end++;\n"
"        while (*end) {\n"
"            if (*end == '\\\\' && end[1]) { end += 2; continue; }\n"
"            if (*end == '\"') { end++; break; }\n"
"            end++;\n"
"        }\n"
"    } else if (*value == '{' || *value == '[') {\n"
"        char open = *value;\n"
"        char close = open == '{' ? '}' : ']';\n"
"        while (*end) {\n"
"            if (*end == '\"') {\n"
"                end++;\n"
"                while (*end) {\n"
"                    if (*end == '\\\\' && end[1]) { end += 2; continue; }\n"
"                    if (*end == '\"') { end++; break; }\n"
"                    end++;\n"
"                }\n"
"                continue;\n"
"            }\n"
"            if (*end == open) depth++;\n"
"            if (*end == close) {\n"
"                depth--;\n"
"                if (depth == 0) { end++; break; }\n"
"            }\n"
"            end++;\n"
"        }\n"
"    } else {\n"
"        while (*end && *end != ',' && *end != '}') end++;\n"
"    }\n"
"    while (end > value && (end[-1] == ' ' || end[-1] == '\\t' || end[-1] == '\\r' || end[-1] == '\\n')) end--;\n"
"    size_t len = (size_t) (end - value);\n"
"    if (cap == 0) return 1;\n"
"    if (len >= cap) len = cap - 1;\n"
"    memcpy(out, value, len);\n"
"    out[len] = '\\0';\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_json_escape(SpearScope *scope, const char *text) {\n"
"    size_t extra = 0;\n"
"    size_t len = strlen(text);\n"
"    for (size_t i = 0; i < len; i++) {\n"
"        char ch = text[i];\n"
"        if (ch == '\\\\' || ch == '\"' || ch == '\\n' || ch == '\\r' || ch == '\\t') extra++;\n"
"    }\n"
"    char *dst = (char *) spear_alloc(scope, len + extra + 1);\n"
"    size_t out = 0;\n"
"    for (size_t i = 0; i < len; i++) {\n"
"        char ch = text[i];\n"
"        if (ch == '\\\\' || ch == '\"') {\n"
"            dst[out++] = '\\\\';\n"
"            dst[out++] = ch;\n"
"        } else if (ch == '\\n') {\n"
"            dst[out++] = '\\\\';\n"
"            dst[out++] = 'n';\n"
"        } else if (ch == '\\r') {\n"
"            dst[out++] = '\\\\';\n"
"            dst[out++] = 'r';\n"
"        } else if (ch == '\\t') {\n"
"            dst[out++] = '\\\\';\n"
"            dst[out++] = 't';\n"
"        } else {\n"
"            dst[out++] = ch;\n"
"        }\n"
"    }\n"
"    dst[out] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_error(SpearScope *scope, const char *message) {\n"
"    char *escaped = spear_json_escape(scope, message);\n"
"    size_t len = strlen(escaped) + 32;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":false,\\\"error\\\":\\\"%s\\\"}\", escaped);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_path(SpearScope *scope, const char *path) {\n"
"    char *escaped = spear_json_escape(scope, path);\n"
"    size_t len = strlen(escaped) + 32;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":true,\\\"path\\\":\\\"%s\\\"}\", escaped);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_text_path(SpearScope *scope, const char *text, const char *path) {\n"
"    char *escaped_text = spear_json_escape(scope, text);\n"
"    char *escaped_path = spear_json_escape(scope, path);\n"
"    size_t len = strlen(escaped_text) + strlen(escaped_path) + 48;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":true,\\\"text\\\":\\\"%s\\\",\\\"path\\\":\\\"%s\\\"}\", escaped_text, escaped_path);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_claims(SpearScope *scope, const char *claims_json) {\n"
"    size_t len = strlen(claims_json) + 32;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":true,\\\"claims\\\":%s}\", claims_json);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_http_ok(SpearScope *scope, long long status, const char *body, const char *url) {\n"
"    char status_buf[32];\n"
"    char *escaped_body = spear_json_escape(scope, body ? body : \"\");\n"
"    char *escaped_url = spear_json_escape(scope, url ? url : \"\");\n"
"    _snprintf(status_buf, sizeof(status_buf), \"%lld\", status);\n"
"    size_t len = strlen(escaped_body) + strlen(escaped_url) + strlen(status_buf) + 64;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":true,\\\"status\\\":%s,\\\"body\\\":\\\"%s\\\",\\\"url\\\":\\\"%s\\\"}\", status_buf, escaped_body, escaped_url);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_result_http_error(SpearScope *scope, long long status, const char *body, const char *error, const char *url) {\n"
"    char status_buf[32];\n"
"    char *escaped_body = spear_json_escape(scope, body ? body : \"\");\n"
"    char *escaped_error = spear_json_escape(scope, error ? error : \"\");\n"
"    char *escaped_url = spear_json_escape(scope, url ? url : \"\");\n"
"    _snprintf(status_buf, sizeof(status_buf), \"%lld\", status);\n"
"    size_t len = strlen(escaped_body) + strlen(escaped_error) + strlen(escaped_url) + strlen(status_buf) + 96;\n"
"    char *dst = (char *) spear_alloc(scope, len);\n"
"    snprintf(dst, len, \"{\\\"ok\\\":false,\\\"status\\\":%s,\\\"body\\\":\\\"%s\\\",\\\"error\\\":\\\"%s\\\",\\\"url\\\":\\\"%s\\\"}\", status_buf, escaped_body, escaped_error, escaped_url);\n"
"    return dst;\n"
"}\n"
"\n"
"typedef struct SpearStrBuf {\n"
"    char *data;\n"
"    size_t len;\n"
"    size_t cap;\n"
"} SpearStrBuf;\n"
"\n"
"static void spear_sb_init(SpearStrBuf *buf) {\n"
"    buf->cap = 256;\n"
"    buf->len = 0;\n"
"    buf->data = (char *) malloc(buf->cap);\n"
"    if (!buf->data) spear_runtime_fail(\"out of memory\");\n"
"    buf->data[0] = '\\0';\n"
"}\n"
"\n"
"static void spear_sb_reserve(SpearStrBuf *buf, size_t extra) {\n"
"    size_t need = buf->len + extra + 1;\n"
"    if (need <= buf->cap) return;\n"
"    while (buf->cap < need) buf->cap *= 2;\n"
"    buf->data = (char *) realloc(buf->data, buf->cap);\n"
"    if (!buf->data) spear_runtime_fail(\"out of memory\");\n"
"}\n"
"\n"
"static void spear_sb_append(SpearStrBuf *buf, const char *text) {\n"
"    size_t add = strlen(text);\n"
"    spear_sb_reserve(buf, add);\n"
"    memcpy(buf->data + buf->len, text, add + 1);\n"
"    buf->len += add;\n"
"}\n"
"\n"
"static void spear_sb_append_len(SpearStrBuf *buf, const char *text, size_t add) {\n"
"    spear_sb_reserve(buf, add);\n"
"    memcpy(buf->data + buf->len, text, add);\n"
"    buf->len += add;\n"
"    buf->data[buf->len] = '\\0';\n"
"}\n"
"\n"
"static void spear_sb_appendf(SpearStrBuf *buf, const char *fmt, ...) {\n"
"    va_list args;\n"
"    int need;\n"
"    va_start(args, fmt);\n"
"    need = _vscprintf(fmt, args);\n"
"    va_end(args);\n"
"    if (need < 0) spear_runtime_fail(\"out of memory\");\n"
"    spear_sb_reserve(buf, (size_t) need);\n"
"    va_start(args, fmt);\n"
"    vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, args);\n"
"    va_end(args);\n"
"    buf->len += (size_t) need;\n"
"}\n"
"\n"
"static char *spear_sb_take(SpearScope *scope, SpearStrBuf *buf) {\n"
"    char *out = spear_text_clone(scope, buf->data ? buf->data : \"\");\n"
"    free(buf->data);\n"
"    buf->data = NULL;\n"
"    buf->len = 0;\n"
"    buf->cap = 0;\n"
"    return out;\n"
"}\n"
"\n"
"static char *spear_html_escape_text(SpearScope *scope, const char *text) {\n"
"    size_t extra = 0;\n"
"    size_t len = strlen(text);\n"
"    char *dst;\n"
"    size_t out = 0;\n"
"    for (size_t i = 0; i < len; i++) {\n"
"        if (text[i] == '&') extra += 4;\n"
"        else if (text[i] == '<' || text[i] == '>') extra += 3;\n"
"        else if (text[i] == '\"') extra += 5;\n"
"    }\n"
"    dst = (char *) spear_alloc(scope, len + extra + 1);\n"
"    for (size_t i = 0; i < len; i++) {\n"
"        if (text[i] == '&') { memcpy(dst + out, \"&amp;\", 5); out += 5; }\n"
"        else if (text[i] == '<') { memcpy(dst + out, \"&lt;\", 4); out += 4; }\n"
"        else if (text[i] == '>') { memcpy(dst + out, \"&gt;\", 4); out += 4; }\n"
"        else if (text[i] == '\"') { memcpy(dst + out, \"&quot;\", 6); out += 6; }\n"
"        else dst[out++] = text[i];\n"
"    }\n"
"    dst[out] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static int spear_json_get_bool(const char *json, const char *key, int fallback) {\n"
"    char raw[64];\n"
"    if (!spear_json_get_raw(json, key, raw, sizeof(raw))) return fallback;\n"
"    if (_stricmp(raw, \"true\") == 0 || strcmp(raw, \"1\") == 0) return 1;\n"
"    if (_stricmp(raw, \"false\") == 0 || strcmp(raw, \"0\") == 0) return 0;\n"
"    return fallback;\n"
"}\n"
"\n"
"static int spear_utf8_to_wide(const char *src, wchar_t *dst, int cap) {\n"
"    if (cap <= 0) return 0;\n"
"    if (!src) src = \"\";\n"
"    return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, cap) > 0;\n"
"}\n"
"\n"
"static int spear_json_array_get(const char *json, int index, char *out, size_t cap) {\n"
"    const char *p = json;\n"
"    int current = 0;\n"
"    if (!p) return 0;\n"
"    while (*p && *p != '[') p++;\n"
"    if (*p != '[') return 0;\n"
"    p++;\n"
"    for (;;) {\n"
"        const char *start;\n"
"        const char *end;\n"
"        int depth = 0;\n"
"        while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;\n"
"        if (*p == ']' || *p == '\\0') return 0;\n"
"        start = p;\n"
"        if (*p == '\"') {\n"
"            p++;\n"
"            while (*p) {\n"
"                if (*p == '\\\\' && p[1]) { p += 2; continue; }\n"
"                if (*p == '\"') { p++; break; }\n"
"                p++;\n"
"            }\n"
"            end = p;\n"
"        } else if (*p == '{' || *p == '[') {\n"
"            char open = *p;\n"
"            char close = open == '{' ? '}' : ']';\n"
"            while (*p) {\n"
"                if (*p == '\"') {\n"
"                    p++;\n"
"                    while (*p) {\n"
"                        if (*p == '\\\\' && p[1]) { p += 2; continue; }\n"
"                        if (*p == '\"') { p++; break; }\n"
"                        p++;\n"
"                    }\n"
"                    continue;\n"
"                }\n"
"                if (*p == open) depth++;\n"
"                if (*p == close) {\n"
"                    depth--;\n"
"                    if (depth == 0) { p++; break; }\n"
"                }\n"
"                p++;\n"
"            }\n"
"            end = p;\n"
"        } else {\n"
"            while (*p && *p != ',' && *p != ']') p++;\n"
"            end = p;\n"
"        }\n"
"        while (end > start && (end[-1] == ' ' || end[-1] == '\\t' || end[-1] == '\\r' || end[-1] == '\\n')) end--;\n"
"        if (current == index) {\n"
"            size_t len = (size_t) (end - start);\n"
"            if (cap == 0) return 1;\n"
"            if (len >= cap) len = cap - 1;\n"
"            memcpy(out, start, len);\n"
"            out[len] = '\\0';\n"
"            return 1;\n"
"        }\n"
"        current++;\n"
"        while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;\n"
"        if (*p == ',') { p++; continue; }\n"
"        if (*p == ']' || *p == '\\0') return 0;\n"
"    }\n"
"}\n"
"\n"
"static int spear_json_object_next(const char *json, int *offset, char *key, size_t key_cap, char *value, size_t value_cap) {\n"
"    const char *p = json + *offset;\n"
"    int key_len = 0;\n"
"    if (!json || !offset) return 0;\n"
"    if (*offset == 0) {\n"
"        p = json;\n"
"        while (*p && *p != '{') p++;\n"
"        if (*p != '{') return 0;\n"
"        p++;\n"
"    }\n"
"    while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n' || *p == ',') p++;\n"
"    if (*p == '}' || *p == '\\0') { *offset = (int) (p - json); return 0; }\n"
"    if (*p != '\"') return 0;\n"
"    p++;\n"
"    while (*p && *p != '\"') {\n"
"        if (*p == '\\\\' && p[1]) {\n"
"            p++;\n"
"            if ((size_t) key_len + 1 < key_cap) key[key_len++] = *p;\n"
"            p++;\n"
"            continue;\n"
"        }\n"
"        if ((size_t) key_len + 1 < key_cap) key[key_len++] = *p;\n"
"        p++;\n"
"    }\n"
"    key[key_len] = '\\0';\n"
"    if (*p != '\"') return 0;\n"
"    p++;\n"
"    while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;\n"
"    if (*p != ':') return 0;\n"
"    p++;\n"
"    while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;\n"
"    const char *start = p;\n"
"    const char *end = p;\n"
"    if (*p == '\"') {\n"
"        p++;\n"
"        while (*p) {\n"
"            if (*p == '\\\\' && p[1]) { p += 2; continue; }\n"
"            if (*p == '\"') { p++; break; }\n"
"            p++;\n"
"        }\n"
"        end = p;\n"
"    } else if (*p == '{' || *p == '[') {\n"
"        char stack[64];\n"
"        int top = 0;\n"
"        stack[top++] = (*p == '{') ? '}' : ']';\n"
"        p++;\n"
"        while (*p && top > 0) {\n"
"            if (*p == '\"') {\n"
"                p++;\n"
"                while (*p) {\n"
"                    if (*p == '\\\\' && p[1]) { p += 2; continue; }\n"
"                    if (*p == '\"') { p++; break; }\n"
"                    p++;\n"
"                }\n"
"                continue;\n"
"            }\n"
"            if ((*p == '{' || *p == '[') && top < 63) stack[top++] = (*p == '{') ? '}' : ']';\n"
"            else if (top > 0 && *p == stack[top - 1]) top--;\n"
"            p++;\n"
"        }\n"
"        end = p;\n"
"    } else {\n"
"        while (*p && *p != ',' && *p != '}') p++;\n"
"        end = p;\n"
"    }\n"
"    while (end > start && (end[-1] == ' ' || end[-1] == '\\t' || end[-1] == '\\r' || end[-1] == '\\n')) end--;\n"
"    size_t copy = (size_t) (end - start);\n"
"    if (copy >= value_cap) copy = value_cap - 1;\n"
"    memcpy(value, start, copy);\n"
"    value[copy] = '\\0';\n"
"    while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;\n"
"    if (*p == ',') p++;\n"
"    *offset = (int) (p - json);\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_json_value_to_text(SpearScope *scope, const char *raw) {\n"
"    char wrapped[16384];\n"
"    char text[8192];\n"
"    if (!raw) return spear_text_clone(scope, \"\");\n"
"    snprintf(wrapped, sizeof(wrapped), \"{\\\"x\\\":%s}\", raw);\n"
"    if (spear_json_get_string(wrapped, \"x\", text, sizeof(text))) return spear_text_clone(scope, text);\n"
"    return spear_text_clone(scope, raw);\n"
"}\n"
"\n"
"static void spear_ui_get_state_value(const char *states_json, const char *name, char *out, size_t cap) {\n"
"    int index = 0;\n"
"    char item[4096];\n"
"    out[0] = '\\0';\n"
"    while (spear_json_array_get(states_json, index++, item, sizeof(item))) {\n"
"        char state_name[256];\n"
"        if (!spear_json_get_string(item, \"name\", state_name, sizeof(state_name))) continue;\n"
"        if (strcmp(state_name, name) != 0) continue;\n"
"        if (spear_json_get_string(item, \"value\", out, cap)) return;\n"
"        if (spear_json_get_raw(item, \"value\", out, cap)) return;\n"
"        return;\n"
"    }\n"
"}\n"
"\n"
"static void spear_ui_append_action_attrs(SpearScope *scope, SpearStrBuf *buf, const char *action_json) {\n"
"    char value[512];\n"
"    if (!action_json || action_json[0] != '{') return;\n"
"    if (spear_json_get_string(action_json, \"type\", value, sizeof(value))) spear_sb_appendf(buf, \" data-action=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"    if (spear_json_get_string(action_json, \"name\", value, sizeof(value))) spear_sb_appendf(buf, \" data-action-name=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"    if (spear_json_get_string(action_json, \"route\", value, sizeof(value))) spear_sb_appendf(buf, \" data-route=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"    if (spear_json_get_string(action_json, \"dialog\", value, sizeof(value))) spear_sb_appendf(buf, \" data-dialog=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"    if (spear_json_get_string(action_json, \"state_target\", value, sizeof(value))) spear_sb_appendf(buf, \" data-state-target=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"    if (spear_json_get_string(action_json, \"state_value\", value, sizeof(value))) spear_sb_appendf(buf, \" data-state-value=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"}\n"
"\n"
"static const char *spear_ui_style(const char *name) {\n"
"    if (strcmp(name, \"body\") == 0) return \"font-family:Segoe UI,Arial,sans-serif;background:#f5f3ee;color:#111827\";\n"
"    if (strcmp(name, \"main\") == 0) return \"max-width:1080px;margin:0 auto;padding:28px 22px 72px 22px;display:flex;flex-direction:column;gap:20px\";\n"
"    if (strcmp(name, \"column\") == 0) return \"display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"row\") == 0) return \"display:flex;gap:16px;flex-wrap:wrap;align-items:flex-start\";\n"
"    if (strcmp(name, \"nav\") == 0) return \"display:flex;gap:12px;flex-wrap:wrap;align-items:center\";\n"
"    if (strcmp(name, \"toolbar\") == 0) return \"display:flex;justify-content:space-between;gap:16px;align-items:center;padding-bottom:8px;border-bottom:1px solid #e5e7eb\";\n"
"    if (strcmp(name, \"tabs\") == 0) return \"display:flex;gap:10px;flex-wrap:wrap;align-items:center\";\n"
"    if (strcmp(name, \"tab\") == 0) return \"padding:10px 14px;border:1px solid #d1d5db;border-radius:999px;background:#ffffff;color:#111827;text-decoration:none\";\n"
"    if (strcmp(name, \"list\") == 0) return \"display:flex;flex-direction:column;gap:12px;list-style:none;margin:0;padding:0\";\n"
"    if (strcmp(name, \"item\") == 0) return \"padding:16px 18px;border:1px solid #e5e7eb;border-radius:18px;background:#ffffff;display:flex;flex-direction:column;gap:8px\";\n"
"    if (strcmp(name, \"form\") == 0) return \"display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"title\") == 0) return \"font-size:40px;line-height:1;letter-spacing:-0.04em;margin:0\";\n"
"    if (strcmp(name, \"text\") == 0) return \"font-size:18px;line-height:1.7;color:#4b5563;margin:0\";\n"
"    if (strcmp(name, \"divider\") == 0) return \"border:0;border-top:1px solid #e5e7eb;margin:0\";\n"
"    if (strcmp(name, \"button\") == 0) return \"padding:12px 18px;border:0;border-radius:14px;background:#111827;color:#ffffff;font:inherit;cursor:pointer\";\n"
"    if (strcmp(name, \"button_secondary\") == 0) return \"display:inline-block;padding:12px 18px;border-radius:14px;border:1px solid #d1d5db;color:#111827;text-decoration:none;background:#ffffff\";\n"
"    if (strcmp(name, \"field\") == 0) return \"padding:12px 14px;border-radius:12px;border:1px solid #d1d5db;background:#ffffff;color:#111827;font:inherit;width:100%\";\n"
"    if (strcmp(name, \"field_error\") == 0) return \"padding:12px 14px;border-radius:12px;border:1px solid #dc2626;background:#ffffff;color:#111827;font:inherit;width:100%\";\n"
"    if (strcmp(name, \"card\") == 0) return \"padding:24px;border:1px solid #e5e7eb;border-radius:22px;background:#ffffff;display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"section\") == 0) return \"display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"dialog\") == 0) return \"padding:22px;border:1px solid #cbd5e1;border-radius:20px;background:#ffffff;box-shadow:0 24px 60px rgba(15,23,42,0.14);display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"sheet\") == 0) return \"padding:22px;border:1px solid #e5e7eb;border-radius:24px 24px 0 0;background:#ffffff;display:flex;flex-direction:column;gap:16px\";\n"
"    if (strcmp(name, \"stat\") == 0) return \"padding:18px;border:1px solid #e5e7eb;border-radius:18px;background:#ffffff;min-width:180px;display:flex;flex-direction:column;gap:8px\";\n"
"    return \"\";\n"
"}\n"
"\n"
"static char *spear_hex_encode(SpearScope *scope, const unsigned char *data, size_t len) {\n"
"    char *dst = (char *) spear_alloc(scope, len * 2 + 1);\n"
"    for (size_t i = 0; i < len; i++) {\n"
"        snprintf(dst + i * 2, 3, \"%02x\", data[i]);\n"
"    }\n"
"    dst[len * 2] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_base64url_encode(SpearScope *scope, const unsigned char *data, size_t len) {\n"
"    static const char table[] = \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_\";\n"
"    size_t out_cap = ((len + 2) / 3) * 4 + 1;\n"
"    char *dst = (char *) spear_alloc(scope, out_cap);\n"
"    size_t i = 0;\n"
"    size_t out = 0;\n"
"    while (i + 2 < len) {\n"
"        unsigned int block = ((unsigned int) data[i] << 16) | ((unsigned int) data[i + 1] << 8) | (unsigned int) data[i + 2];\n"
"        dst[out++] = table[(block >> 18) & 63];\n"
"        dst[out++] = table[(block >> 12) & 63];\n"
"        dst[out++] = table[(block >> 6) & 63];\n"
"        dst[out++] = table[block & 63];\n"
"        i += 3;\n"
"    }\n"
"    if (i < len) {\n"
"        unsigned int block = (unsigned int) data[i] << 16;\n"
"        dst[out++] = table[(block >> 18) & 63];\n"
"        if (i + 1 < len) {\n"
"            block |= (unsigned int) data[i + 1] << 8;\n"
"            dst[out++] = table[(block >> 12) & 63];\n"
"            dst[out++] = table[(block >> 6) & 63];\n"
"        } else {\n"
"            dst[out++] = table[(block >> 12) & 63];\n"
"        }\n"
"    }\n"
"    dst[out] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static int spear_base64url_decode(const char *text, unsigned char *out, size_t cap, size_t *out_len) {\n"
"    int vals[4];\n"
"    int val_count = 0;\n"
"    size_t out_pos = 0;\n"
"    for (size_t i = 0; text[i]; i++) {\n"
"        unsigned char ch = (unsigned char) text[i];\n"
"        int value = -1;\n"
"        if (ch >= 'A' && ch <= 'Z') value = ch - 'A';\n"
"        else if (ch >= 'a' && ch <= 'z') value = ch - 'a' + 26;\n"
"        else if (ch >= '0' && ch <= '9') value = ch - '0' + 52;\n"
"        else if (ch == '-' || ch == '+') value = 62;\n"
"        else if (ch == '_' || ch == '/') value = 63;\n"
"        else continue;\n"
"        vals[val_count++] = value;\n"
"        if (val_count == 4) {\n"
"            unsigned int block = ((unsigned int) vals[0] << 18) | ((unsigned int) vals[1] << 12) | ((unsigned int) vals[2] << 6) | (unsigned int) vals[3];\n"
"            if (out_pos + 3 > cap) return 0;\n"
"            out[out_pos++] = (unsigned char) ((block >> 16) & 255);\n"
"            out[out_pos++] = (unsigned char) ((block >> 8) & 255);\n"
"            out[out_pos++] = (unsigned char) (block & 255);\n"
"            val_count = 0;\n"
"        }\n"
"    }\n"
"    if (val_count == 2) {\n"
"        unsigned int block = ((unsigned int) vals[0] << 18) | ((unsigned int) vals[1] << 12);\n"
"        if (out_pos + 1 > cap) return 0;\n"
"        out[out_pos++] = (unsigned char) ((block >> 16) & 255);\n"
"    } else if (val_count == 3) {\n"
"        unsigned int block = ((unsigned int) vals[0] << 18) | ((unsigned int) vals[1] << 12) | ((unsigned int) vals[2] << 6);\n"
"        if (out_pos + 2 > cap) return 0;\n"
"        out[out_pos++] = (unsigned char) ((block >> 16) & 255);\n"
"        out[out_pos++] = (unsigned char) ((block >> 8) & 255);\n"
"    } else if (val_count != 0) {\n"
"        return 0;\n"
"    }\n"
"    if (out_len) *out_len = out_pos;\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_utf8_from_bytes(unsigned char *bytes, size_t len, char *out, size_t cap) {\n"
"    if (cap == 0) return 0;\n"
"    if (len >= cap) len = cap - 1;\n"
"    memcpy(out, bytes, len);\n"
"    out[len] = '\\0';\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_resolve_under_root(char *out, size_t cap, const char *root_text, const char *relative_text) {\n"
"    char root_full[MAX_PATH];\n"
"    char combined[MAX_PATH];\n"
"    char target_full[MAX_PATH];\n"
"    size_t root_len;\n"
"    if (!_fullpath(root_full, root_text, sizeof(root_full))) return 0;\n"
"    snprintf(combined, sizeof(combined), \"%s\\\\%s\", root_full, relative_text);\n"
"    if (!_fullpath(target_full, combined, sizeof(target_full))) return 0;\n"
"    root_len = strlen(root_full);\n"
"    while (root_len > 3 && (root_full[root_len - 1] == '\\\\' || root_full[root_len - 1] == '/')) {\n"
"        root_full[--root_len] = '\\0';\n"
"    }\n"
"    if (_strnicmp(root_full, target_full, root_len) != 0) return -1;\n"
"    if (target_full[root_len] != '\\0' && target_full[root_len] != '\\\\' && target_full[root_len] != '/') return -1;\n"
"    snprintf(out, cap, \"%s\", target_full);\n"
"    return 1;\n"
"}\n"
"\n"
"static void spear_ensure_dir_recursive(const char *path) {\n"
"    char temp[MAX_PATH];\n"
"    snprintf(temp, sizeof(temp), \"%s\", path);\n"
"    for (char *cursor = temp + 3; *cursor; cursor++) {\n"
"        if (*cursor == '\\\\' || *cursor == '/') {\n"
"            char saved = *cursor;\n"
"            *cursor = '\\0';\n"
"            CreateDirectoryA(temp, NULL);\n"
"            *cursor = saved;\n"
"        }\n"
"    }\n"
"    CreateDirectoryA(temp, NULL);\n"
"}\n"
"\n"
"static int spear_write_text_try(const char *path, const char *content) {\n"
"    FILE *fp = fopen(path, \"wb\");\n"
"    if (!fp) return 0;\n"
"    size_t len = strlen(content);\n"
"    int ok = fwrite(content, 1, len, fp) == len;\n"
"    fclose(fp);\n"
"    return ok;\n"
"}\n"
"\n"
"static int spear_bcrypt_hash_bytes(int hmac_mode, const unsigned char *key, size_t key_len, const unsigned char *data, size_t data_len, unsigned char *out, unsigned long out_cap) {\n"
"    typedef LONG (WINAPI *SpearBCryptOpenAlgorithmProvider)(void **, LPCWSTR, LPCWSTR, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptCloseAlgorithmProvider)(void *, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptGetProperty)(void *, LPCWSTR, unsigned char *, ULONG, ULONG *, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptCreateHash)(void *, void **, unsigned char *, ULONG, unsigned char *, ULONG, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptHashData)(void *, unsigned char *, ULONG, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptFinishHash)(void *, unsigned char *, ULONG, ULONG);\n"
"    typedef LONG (WINAPI *SpearBCryptDestroyHash)(void *);\n"
"    HMODULE bcrypt_mod = LoadLibraryA(\"bcrypt.dll\");\n"
"    void *alg = NULL;\n"
"    void *hash = NULL;\n"
"    unsigned char *obj = NULL;\n"
"    ULONG obj_len = 0;\n"
"    ULONG hash_len = 0;\n"
"    ULONG cb = 0;\n"
"    LONG status;\n"
"    SpearBCryptOpenAlgorithmProvider open_alg;\n"
"    SpearBCryptCloseAlgorithmProvider close_alg;\n"
"    SpearBCryptGetProperty get_prop;\n"
"    SpearBCryptCreateHash create_hash;\n"
"    SpearBCryptHashData hash_data;\n"
"    SpearBCryptFinishHash finish_hash;\n"
"    SpearBCryptDestroyHash destroy_hash;\n"
"    if (!bcrypt_mod) return 0;\n"
"    open_alg = (SpearBCryptOpenAlgorithmProvider) GetProcAddress(bcrypt_mod, \"BCryptOpenAlgorithmProvider\");\n"
"    close_alg = (SpearBCryptCloseAlgorithmProvider) GetProcAddress(bcrypt_mod, \"BCryptCloseAlgorithmProvider\");\n"
"    get_prop = (SpearBCryptGetProperty) GetProcAddress(bcrypt_mod, \"BCryptGetProperty\");\n"
"    create_hash = (SpearBCryptCreateHash) GetProcAddress(bcrypt_mod, \"BCryptCreateHash\");\n"
"    hash_data = (SpearBCryptHashData) GetProcAddress(bcrypt_mod, \"BCryptHashData\");\n"
"    finish_hash = (SpearBCryptFinishHash) GetProcAddress(bcrypt_mod, \"BCryptFinishHash\");\n"
"    destroy_hash = (SpearBCryptDestroyHash) GetProcAddress(bcrypt_mod, \"BCryptDestroyHash\");\n"
"    if (!open_alg || !close_alg || !get_prop || !create_hash || !hash_data || !finish_hash || !destroy_hash) { FreeLibrary(bcrypt_mod); return 0; }\n"
"    status = open_alg(&alg, L\"SHA256\", NULL, hmac_mode ? SPEAR_BCRYPT_ALG_HANDLE_HMAC_FLAG : 0);\n"
"    if (status < 0) { FreeLibrary(bcrypt_mod); return 0; }\n"
"    status = get_prop(alg, L\"ObjectLength\", (unsigned char *) &obj_len, sizeof(obj_len), &cb, 0);\n"
"    if (status < 0 || obj_len == 0) { close_alg(alg, 0); FreeLibrary(bcrypt_mod); return 0; }\n"
"    status = get_prop(alg, L\"HashDigestLength\", (unsigned char *) &hash_len, sizeof(hash_len), &cb, 0);\n"
"    if (status < 0 || hash_len == 0 || hash_len > out_cap) { close_alg(alg, 0); FreeLibrary(bcrypt_mod); return 0; }\n"
"    obj = (unsigned char *) malloc(obj_len);\n"
"    if (!obj) { close_alg(alg, 0); FreeLibrary(bcrypt_mod); return 0; }\n"
"    status = create_hash(alg, &hash, obj, obj_len, hmac_mode ? (unsigned char *) key : NULL, hmac_mode ? (ULONG) key_len : 0, 0);\n"
"    if (status >= 0) status = hash_data(hash, (unsigned char *) data, (ULONG) data_len, 0);\n"
"    if (status >= 0) status = finish_hash(hash, out, hash_len, 0);\n"
"    if (hash) destroy_hash(hash);\n"
"    free(obj);\n"
"    close_alg(alg, 0);\n"
"    FreeLibrary(bcrypt_mod);\n"
"    return status >= 0;\n"
"}\n"
"\n"
"static int spear_bcrypt_pbkdf2_sha256(const unsigned char *password, size_t password_len, const unsigned char *salt, size_t salt_len, unsigned long long rounds, unsigned char *out, unsigned long out_len) {\n"
"    unsigned char u[32];\n"
"    unsigned char t[32];\n"
"    unsigned char *salt_block;\n"
"    unsigned long block_index = 1;\n"
"    unsigned long out_pos = 0;\n"
"    if (rounds == 0) return 0;\n"
"    salt_block = (unsigned char *) malloc(salt_len + 4);\n"
"    if (!salt_block) return 0;\n"
"    memcpy(salt_block, salt, salt_len);\n"
"    while (out_pos < out_len) {\n"
"        salt_block[salt_len + 0] = (unsigned char) ((block_index >> 24) & 255);\n"
"        salt_block[salt_len + 1] = (unsigned char) ((block_index >> 16) & 255);\n"
"        salt_block[salt_len + 2] = (unsigned char) ((block_index >> 8) & 255);\n"
"        salt_block[salt_len + 3] = (unsigned char) (block_index & 255);\n"
"        if (!spear_bcrypt_hash_bytes(1, password, password_len, salt_block, salt_len + 4, u, sizeof(u))) {\n"
"            free(salt_block);\n"
"            return 0;\n"
"        }\n"
"        memcpy(t, u, sizeof(t));\n"
"        for (unsigned long long i = 1; i < rounds; i++) {\n"
"            if (!spear_bcrypt_hash_bytes(1, password, password_len, u, sizeof(u), u, sizeof(u))) {\n"
"                free(salt_block);\n"
"                return 0;\n"
"            }\n"
"            for (size_t j = 0; j < sizeof(t); j++) t[j] ^= u[j];\n"
"        }\n"
"        unsigned long remaining = out_len - out_pos;\n"
"        unsigned long copy_len = remaining < sizeof(t) ? remaining : (unsigned long) sizeof(t);\n"
"        memcpy(out + out_pos, t, copy_len);\n"
"        out_pos += copy_len;\n"
"        block_index++;\n"
"    }\n"
"    free(salt_block);\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_native_secure_token(SpearScope *scope, const char *payload) {\n"
"    typedef long (WINAPI *SpearBCryptGenRandom)(void *, unsigned char *, unsigned long, unsigned long);\n"
"    long long bytes = spear_json_get_int(payload, \"bytes\", 32);\n"
"    HMODULE bcrypt_mod;\n"
"    SpearBCryptGenRandom bcrypt_random;\n"
"    unsigned char random_bytes[64];\n"
"    if (bytes < 16) bytes = 16;\n"
"    if (bytes > 64) bytes = 64;\n"
"    bcrypt_mod = LoadLibraryA(\"bcrypt.dll\");\n"
"    if (!bcrypt_mod) return spear_native_result_error(scope, \"cannot load system rng\");\n"
"    bcrypt_random = (SpearBCryptGenRandom) GetProcAddress(bcrypt_mod, \"BCryptGenRandom\");\n"
"    if (!bcrypt_random) {\n"
"        FreeLibrary(bcrypt_mod);\n"
"        return spear_native_result_error(scope, \"cannot load system rng\");\n"
"    }\n"
"    if (bcrypt_random(NULL, random_bytes, (unsigned long) bytes, 0x00000002UL) < 0) {\n"
"        FreeLibrary(bcrypt_mod);\n"
"        return spear_native_result_error(scope, \"system rng failed\");\n"
"    }\n"
"    FreeLibrary(bcrypt_mod);\n"
"    size_t out_len = (size_t) bytes * 2;\n"
"    char *dst = (char *) spear_alloc(scope, out_len + 1);\n"
"    for (long long i = 0; i < bytes; i++) {\n"
"        snprintf(dst + (size_t) i * 2, 3, \"%02x\", random_bytes[i]);\n"
"    }\n"
"    dst[out_len] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_secure_compare(SpearScope *scope, const char *payload) {\n"
"    char left[4096];\n"
"    char right[4096];\n"
"    size_t left_len;\n"
"    size_t right_len;\n"
"    unsigned int diff = 0;\n"
"    (void) scope;\n"
"    if (!spear_json_get_string(payload, \"left\", left, sizeof(left))) left[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"right\", right, sizeof(right))) right[0] = '\\0';\n"
"    left_len = strlen(left);\n"
"    right_len = strlen(right);\n"
"    diff = (unsigned int) (left_len ^ right_len);\n"
"    for (size_t i = 0; i < left_len && i < right_len; i++) diff |= (unsigned int) (left[i] ^ right[i]);\n"
"    return diff == 0 ? \"1\" : \"0\";\n"
"}\n"
"\n"
"static char *spear_native_sha256_hex(SpearScope *scope, const char *payload) {\n"
"    char text[8192];\n"
"    unsigned char digest[32];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    if (!spear_bcrypt_hash_bytes(0, NULL, 0, (const unsigned char *) text, strlen(text), digest, sizeof(digest))) {\n"
"        return spear_native_result_error(scope, \"sha256 failed\");\n"
"    }\n"
"    return spear_hex_encode(scope, digest, sizeof(digest));\n"
"}\n"
"\n"
"static char *spear_native_hmac_sha256(SpearScope *scope, const char *payload) {\n"
"    char key[4096];\n"
"    char text[8192];\n"
"    unsigned char digest[32];\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    if (!spear_bcrypt_hash_bytes(1, (const unsigned char *) key, strlen(key), (const unsigned char *) text, strlen(text), digest, sizeof(digest))) {\n"
"        return spear_native_result_error(scope, \"hmac failed\");\n"
"    }\n"
"    return spear_hex_encode(scope, digest, sizeof(digest));\n"
"}\n"
"\n"
"static char *spear_native_pbkdf2_sha256(SpearScope *scope, const char *payload) {\n"
"    char password[4096];\n"
"    char salt[4096];\n"
"    unsigned char digest[32];\n"
"    long long rounds = spear_json_get_int(payload, \"rounds\", 120000);\n"
"    if (!spear_json_get_string(payload, \"password\", password, sizeof(password))) password[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"salt\", salt, sizeof(salt))) salt[0] = '\\0';\n"
"    if (rounds < 100000) rounds = 100000;\n"
"    if (rounds > 1000000) rounds = 1000000;\n"
"    if (!spear_bcrypt_pbkdf2_sha256((const unsigned char *) password, strlen(password), (const unsigned char *) salt, strlen(salt), (unsigned long long) rounds, digest, sizeof(digest))) {\n"
"        return spear_native_result_error(scope, \"pbkdf2 failed\");\n"
"    }\n"
"    return spear_hex_encode(scope, digest, sizeof(digest));\n"
"}\n"
"\n"
"static char *spear_native_pbkdf2_verify(SpearScope *scope, const char *payload) {\n"
"    char expected[128];\n"
"    char *actual = spear_native_pbkdf2_sha256(scope, payload);\n"
"    (void) scope;\n"
"    if (!spear_json_get_string(payload, \"expected\", expected, sizeof(expected))) expected[0] = '\\0';\n"
"    if (strchr(actual, '{')) return \"0\";\n"
"    return strcmp(actual, expected) == 0 ? \"1\" : \"0\";\n"
"}\n"
"\n"
"static char *spear_native_jwt_sign_hs256(SpearScope *scope, const char *payload) {\n"
"    char claims[8192];\n"
"    char key[4096];\n"
"    const char *header_json = \"{\\\"alg\\\":\\\"HS256\\\",\\\"typ\\\":\\\"JWT\\\"}\";\n"
"    unsigned char digest[32];\n"
"    if (!spear_json_get_raw(payload, \"claims\", claims, sizeof(claims))) strcpy(claims, \"{}\");\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    char *header_b64 = spear_base64url_encode(scope, (const unsigned char *) header_json, strlen(header_json));\n"
"    char *claims_b64 = spear_base64url_encode(scope, (const unsigned char *) claims, strlen(claims));\n"
"    size_t signing_len = strlen(header_b64) + strlen(claims_b64) + 2;\n"
"    char *signing = (char *) spear_alloc(scope, signing_len);\n"
"    snprintf(signing, signing_len, \"%s.%s\", header_b64, claims_b64);\n"
"    if (!spear_bcrypt_hash_bytes(1, (const unsigned char *) key, strlen(key), (const unsigned char *) signing, strlen(signing), digest, sizeof(digest))) {\n"
"        return spear_native_result_error(scope, \"jwt signing failed\");\n"
"    }\n"
"    char *sig_b64 = spear_base64url_encode(scope, digest, sizeof(digest));\n"
"    size_t token_len = strlen(signing) + strlen(sig_b64) + 2;\n"
"    char *token = (char *) spear_alloc(scope, token_len);\n"
"    snprintf(token, token_len, \"%s.%s\", signing, sig_b64);\n"
"    return token;\n"
"}\n"
"\n"
"static char *spear_native_jwt_decode(SpearScope *scope, const char *payload) {\n"
"    char token[8192];\n"
"    unsigned char decoded[8192];\n"
"    size_t decoded_len = 0;\n"
"    char *first_dot;\n"
"    char *second_dot;\n"
"    if (!spear_json_get_string(payload, \"token\", token, sizeof(token))) token[0] = '\\0';\n"
"    first_dot = strchr(token, '.');\n"
"    if (!first_dot) return spear_native_result_error(scope, \"invalid token format\");\n"
"    second_dot = strchr(first_dot + 1, '.');\n"
"    if (!second_dot) return spear_native_result_error(scope, \"invalid token format\");\n"
"    *second_dot = '\\0';\n"
"    if (!spear_base64url_decode(first_dot + 1, decoded, sizeof(decoded) - 1, &decoded_len)) {\n"
"        return spear_native_result_error(scope, \"invalid token payload\");\n"
"    }\n"
"    decoded[decoded_len] = '\\0';\n"
"    return spear_text_clone(scope, (const char *) decoded);\n"
"}\n"
"\n"
"static char *spear_native_jwt_verify_hs256(SpearScope *scope, const char *payload) {\n"
"    char token[8192];\n"
"    char key[4096];\n"
"    long long leeway = spear_json_get_int(payload, \"leeway\", 0);\n"
"    unsigned char header_dec[2048];\n"
"    unsigned char claims_dec[8192];\n"
"    unsigned char digest[32];\n"
"    size_t header_len = 0;\n"
"    size_t claims_len = 0;\n"
"    char alg[64];\n"
"    const char *tmp_value = NULL;\n"
"    char *first_dot;\n"
"    char *second_dot;\n"
"    if (!spear_json_get_string(payload, \"token\", token, sizeof(token))) token[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    first_dot = strchr(token, '.');\n"
"    if (!first_dot) return spear_native_result_error(scope, \"invalid token format\");\n"
"    second_dot = strchr(first_dot + 1, '.');\n"
"    if (!second_dot) return spear_native_result_error(scope, \"invalid token format\");\n"
"    *first_dot = '\\0';\n"
"    *second_dot = '\\0';\n"
"    if (!spear_base64url_decode(token, header_dec, sizeof(header_dec) - 1, &header_len)) return spear_native_result_error(scope, \"invalid token payload\");\n"
"    if (!spear_base64url_decode(first_dot + 1, claims_dec, sizeof(claims_dec) - 1, &claims_len)) return spear_native_result_error(scope, \"invalid token payload\");\n"
"    header_dec[header_len] = '\\0';\n"
"    claims_dec[claims_len] = '\\0';\n"
"    if (!spear_json_get_string((const char *) header_dec, \"alg\", alg, sizeof(alg)) || strcmp(alg, \"HS256\") != 0) {\n"
"        return spear_native_result_error(scope, \"unsupported jwt algorithm\");\n"
"    }\n"
"    size_t signing_len = strlen(token) + strlen(first_dot + 1) + 2;\n"
"    char *signing = (char *) spear_alloc(scope, signing_len);\n"
"    snprintf(signing, signing_len, \"%s.%s\", token, first_dot + 1);\n"
"    if (!spear_bcrypt_hash_bytes(1, (const unsigned char *) key, strlen(key), (const unsigned char *) signing, strlen(signing), digest, sizeof(digest))) {\n"
"        return spear_native_result_error(scope, \"invalid jwt signature\");\n"
"    }\n"
"    char *expected_sig = spear_base64url_encode(scope, digest, sizeof(digest));\n"
"    if (strcmp(expected_sig, second_dot + 1) != 0) return spear_native_result_error(scope, \"invalid jwt signature\");\n"
"    if (spear_json_seek_key((const char *) claims_dec, \"exp\", &tmp_value)) {\n"
"        long long exp = spear_json_get_int((const char *) claims_dec, \"exp\", 0);\n"
"        long long now = (long long) time(NULL);\n"
"        if (now > exp + leeway) return spear_native_result_error(scope, \"jwt has expired\");\n"
"    }\n"
"    if (spear_json_seek_key((const char *) claims_dec, \"nbf\", &tmp_value)) {\n"
"        long long nbf = spear_json_get_int((const char *) claims_dec, \"nbf\", 0);\n"
"        long long now = (long long) time(NULL);\n"
"        if (now + leeway < nbf) return spear_native_result_error(scope, \"jwt is not active yet\");\n"
"    }\n"
"    return spear_native_result_claims(scope, (const char *) claims_dec);\n"
"}\n"
"\n"
"static char *spear_native_safe_http_request(SpearScope *scope, const char *payload) {\n"
"    char url[4096];\n"
"    char method[32];\n"
"    char body[16384];\n"
"    char content_type[256];\n"
"    long long timeout = spear_json_get_int(payload, \"timeout\", 10);\n"
"    long long max_bytes = spear_json_get_int(payload, \"max_bytes\", 262144);\n"
"    wchar_t url_w[4096];\n"
"    wchar_t host_w[1024];\n"
"    wchar_t path_w[4096];\n"
"    wchar_t method_w[16];\n"
"    wchar_t headers_w[512];\n"
"    URL_COMPONENTSW parts;\n"
"    INTERNET_PORT port = 443;\n"
"    HMODULE winhttp_mod;\n"
"    HINTERNET session = NULL;\n"
"    HINTERNET connect = NULL;\n"
"    HINTERNET request = NULL;\n"
"    DWORD status = 0;\n"
"    DWORD status_len = sizeof(status);\n"
"    DWORD available = 0;\n"
"    char *response = NULL;\n"
"    size_t response_len = 0;\n"
"    size_t response_cap = 0;\n"
"    char error_buf[128];\n"
"    typedef HINTERNET (WINAPI *SpearWinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);\n"
"    typedef BOOL (WINAPI *SpearWinHttpCloseHandle)(HINTERNET);\n"
"    typedef BOOL (WINAPI *SpearWinHttpCrackUrl)(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTSW);\n"
"    typedef HINTERNET (WINAPI *SpearWinHttpConnect)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);\n"
"    typedef HINTERNET (WINAPI *SpearWinHttpOpenRequest)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR*, DWORD);\n"
"    typedef BOOL (WINAPI *SpearWinHttpSetTimeouts)(HINTERNET, int, int, int, int);\n"
"    typedef BOOL (WINAPI *SpearWinHttpSendRequest)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);\n"
"    typedef BOOL (WINAPI *SpearWinHttpReceiveResponse)(HINTERNET, LPVOID);\n"
"    typedef BOOL (WINAPI *SpearWinHttpQueryHeaders)(HINTERNET, DWORD, LPCWSTR, LPVOID, LPDWORD, LPDWORD);\n"
"    typedef BOOL (WINAPI *SpearWinHttpQueryDataAvailable)(HINTERNET, LPDWORD);\n"
"    typedef BOOL (WINAPI *SpearWinHttpReadData)(HINTERNET, LPVOID, DWORD, LPDWORD);\n"
"    SpearWinHttpOpen open_fn;\n"
"    SpearWinHttpCloseHandle close_fn;\n"
"    SpearWinHttpCrackUrl crack_fn;\n"
"    SpearWinHttpConnect connect_fn;\n"
"    SpearWinHttpOpenRequest open_req_fn;\n"
"    SpearWinHttpSetTimeouts timeouts_fn;\n"
"    SpearWinHttpSendRequest send_fn;\n"
"    SpearWinHttpReceiveResponse recv_fn;\n"
"    SpearWinHttpQueryHeaders query_headers_fn;\n"
"    SpearWinHttpQueryDataAvailable query_data_fn;\n"
"    SpearWinHttpReadData read_fn;\n"
"    if (!spear_json_get_string(payload, \"url\", url, sizeof(url))) url[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"method\", method, sizeof(method))) strcpy(method, \"GET\");\n"
"    if (!spear_json_get_string(payload, \"body\", body, sizeof(body))) body[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"content_type\", content_type, sizeof(content_type))) content_type[0] = '\\0';\n"
"    if (_stricmp(method, \"GET\") != 0 && _stricmp(method, \"POST\") != 0) return spear_native_result_error(scope, \"unsupported http method\");\n"
"    if (_strnicmp(url, \"https://\", 8) != 0) return spear_native_result_error(scope, \"https is required\");\n"
"    if (timeout < 1) timeout = 1;\n"
"    if (timeout > 30) timeout = 30;\n"
"    if (max_bytes < 1024) max_bytes = 1024;\n"
"    if (max_bytes > 1048576) max_bytes = 1048576;\n"
"    if (!spear_utf8_to_wide(url, url_w, (int) (sizeof(url_w) / sizeof(url_w[0])))) return spear_native_result_error(scope, \"invalid https url\");\n"
"    ZeroMemory(&parts, sizeof(parts));\n"
"    parts.dwStructSize = sizeof(parts);\n"
"    parts.lpszHostName = host_w;\n"
"    parts.dwHostNameLength = (DWORD) ((sizeof(host_w) / sizeof(host_w[0])) - 1);\n"
"    parts.lpszUrlPath = path_w;\n"
"    parts.dwUrlPathLength = (DWORD) ((sizeof(path_w) / sizeof(path_w[0])) - 1);\n"
"    if (!parts.dwHostNameLength || !parts.dwUrlPathLength) return spear_native_result_error(scope, \"invalid https url\");\n"
"    winhttp_mod = LoadLibraryA(\"winhttp.dll\");\n"
"    if (!winhttp_mod) return spear_native_result_error(scope, \"https client unavailable\");\n"
"    open_fn = (SpearWinHttpOpen) GetProcAddress(winhttp_mod, \"WinHttpOpen\");\n"
"    close_fn = (SpearWinHttpCloseHandle) GetProcAddress(winhttp_mod, \"WinHttpCloseHandle\");\n"
"    crack_fn = (SpearWinHttpCrackUrl) GetProcAddress(winhttp_mod, \"WinHttpCrackUrl\");\n"
"    connect_fn = (SpearWinHttpConnect) GetProcAddress(winhttp_mod, \"WinHttpConnect\");\n"
"    open_req_fn = (SpearWinHttpOpenRequest) GetProcAddress(winhttp_mod, \"WinHttpOpenRequest\");\n"
"    timeouts_fn = (SpearWinHttpSetTimeouts) GetProcAddress(winhttp_mod, \"WinHttpSetTimeouts\");\n"
"    send_fn = (SpearWinHttpSendRequest) GetProcAddress(winhttp_mod, \"WinHttpSendRequest\");\n"
"    recv_fn = (SpearWinHttpReceiveResponse) GetProcAddress(winhttp_mod, \"WinHttpReceiveResponse\");\n"
"    query_headers_fn = (SpearWinHttpQueryHeaders) GetProcAddress(winhttp_mod, \"WinHttpQueryHeaders\");\n"
"    query_data_fn = (SpearWinHttpQueryDataAvailable) GetProcAddress(winhttp_mod, \"WinHttpQueryDataAvailable\");\n"
"    read_fn = (SpearWinHttpReadData) GetProcAddress(winhttp_mod, \"WinHttpReadData\");\n"
"    if (!open_fn || !close_fn || !crack_fn || !connect_fn || !open_req_fn || !timeouts_fn || !send_fn || !recv_fn || !query_headers_fn || !query_data_fn || !read_fn) {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https client unavailable\");\n"
"    }\n"
"    if (!crack_fn(url_w, 0, 0, &parts)) {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"invalid https url\");\n"
"    }\n"
"    host_w[parts.dwHostNameLength] = L'\\0';\n"
"    path_w[parts.dwUrlPathLength] = L'\\0';\n"
"    if (host_w[0] == L'\\0') {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"missing https host\");\n"
"    }\n"
"    if (parts.nScheme != INTERNET_SCHEME_HTTPS) {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https is required\");\n"
"    }\n"
"    port = parts.nPort ? parts.nPort : 443;\n"
"    if (!spear_utf8_to_wide(_stricmp(method, \"POST\") == 0 ? \"POST\" : \"GET\", method_w, (int) (sizeof(method_w) / sizeof(method_w[0])))) {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"unsupported http method\");\n"
"    }\n"
"    session = open_fn(L\"Spear/0.1\", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);\n"
"    if (!session) {\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https request failed\");\n"
"    }\n"
"    timeouts_fn(session, (int) (timeout * 1000), (int) (timeout * 1000), (int) (timeout * 1000), (int) (timeout * 1000));\n"
"    connect = connect_fn(session, host_w, port, 0);\n"
"    if (!connect) {\n"
"        close_fn(session);\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https request failed\");\n"
"    }\n"
"    request = open_req_fn(connect, method_w, path_w[0] ? path_w : L\"/\", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);\n"
"    if (!request) {\n"
"        close_fn(connect);\n"
"        close_fn(session);\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https request failed\");\n"
"    }\n"
"    timeouts_fn(request, (int) (timeout * 1000), (int) (timeout * 1000), (int) (timeout * 1000), (int) (timeout * 1000));\n"
"    if (content_type[0]) {\n"
"        _snwprintf(headers_w, sizeof(headers_w) / sizeof(headers_w[0]), L\"Content-Type: %hs\\r\\n\", content_type);\n"
"    } else {\n"
"        headers_w[0] = L'\\0';\n"
"    }\n"
"    if (!send_fn(request, headers_w[0] ? headers_w : WINHTTP_NO_ADDITIONAL_HEADERS, headers_w[0] ? (DWORD) -1L : 0, body[0] ? (LPVOID) body : WINHTTP_NO_REQUEST_DATA, body[0] ? (DWORD) strlen(body) : 0, body[0] ? (DWORD) strlen(body) : 0, 0)) {\n"
"        close_fn(request);\n"
"        close_fn(connect);\n"
"        close_fn(session);\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https request failed\");\n"
"    }\n"
"    if (!recv_fn(request, NULL)) {\n"
"        close_fn(request);\n"
"        close_fn(connect);\n"
"        close_fn(session);\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"https request failed\");\n"
"    }\n"
"    if (!query_headers_fn(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_len, WINHTTP_NO_HEADER_INDEX)) {\n"
"        status = 0;\n"
"    }\n"
"    response_cap = (size_t) max_bytes + 1;\n"
"    response = (char *) malloc(response_cap + 1);\n"
"    if (!response) {\n"
"        close_fn(request);\n"
"        close_fn(connect);\n"
"        close_fn(session);\n"
"        FreeLibrary(winhttp_mod);\n"
"        return spear_native_result_error(scope, \"out of memory\");\n"
"    }\n"
"    response[0] = '\\0';\n"
"    for (;;) {\n"
"        DWORD read_now = 0;\n"
"        if (!query_data_fn(request, &available)) {\n"
"            free(response);\n"
"            close_fn(request);\n"
"            close_fn(connect);\n"
"            close_fn(session);\n"
"            FreeLibrary(winhttp_mod);\n"
"            return spear_native_result_error(scope, \"https request failed\");\n"
"        }\n"
"        if (available == 0) break;\n"
"        if (response_len + available > (size_t) max_bytes) {\n"
"            free(response);\n"
"            close_fn(request);\n"
"            close_fn(connect);\n"
"            close_fn(session);\n"
"            FreeLibrary(winhttp_mod);\n"
"            return spear_native_result_error(scope, \"response body too large\");\n"
"        }\n"
"        if (!read_fn(request, response + response_len, available, &read_now)) {\n"
"            free(response);\n"
"            close_fn(request);\n"
"            close_fn(connect);\n"
"            close_fn(session);\n"
"            FreeLibrary(winhttp_mod);\n"
"            return spear_native_result_error(scope, \"https request failed\");\n"
"        }\n"
"        response_len += read_now;\n"
"    }\n"
"    response[response_len] = '\\0';\n"
"    close_fn(request);\n"
"    close_fn(connect);\n"
"    close_fn(session);\n"
"    FreeLibrary(winhttp_mod);\n"
"    if (status >= 400) {\n"
"        _snprintf(error_buf, sizeof(error_buf), \"http status %lu\", (unsigned long) status);\n"
"        char *result = spear_native_result_http_error(scope, (long long) status, response, error_buf, url);\n"
"        free(response);\n"
"        return result;\n"
"    }\n"
"    char *result = spear_native_result_http_ok(scope, (long long) status, response, url);\n"
"    free(response);\n"
"    return result;\n"
"}\n"
"\n"
"static char *spear_native_http_request(SpearScope *scope, const char *payload) {\n"
"    char *result = spear_native_safe_http_request(scope, payload);\n"
"    char body[32768];\n"
"    char error_text[2048];\n"
"    long long status = 0;\n"
"    int ok = 0;\n"
"    SpearStrBuf out;\n"
"    if (!result) return NULL;\n"
"    status = spear_json_get_int(result, \"status\", 0);\n"
"    ok = spear_json_get_bool(result, \"ok\", status > 0 && status < 400);\n"
"    if (!spear_json_get_string(result, \"body\", body, sizeof(body))) body[0] = '\\0';\n"
"    if (!ok && status == 0) {\n"
"        if (!spear_json_get_string(result, \"error\", error_text, sizeof(error_text))) strcpy(error_text, \"http request failed\");\n"
"        spear_runtime_fail_at(1, 1, error_text);\n"
"    }\n"
"    spear_sb_init(&out);\n"
"    spear_sb_appendf(&out, \"{\\\"status\\\":%lld,\\\"body\\\":\\\"%s\\\",\\\"headers\\\":{}}\", status, spear_json_escape(scope, body));\n"
"    return spear_sb_take(scope, &out);\n"
"}\n"
"\n"
"static int spear_socket_api_load(SpearSocketApi *api) {\n"
"    WSADATA data;\n"
"    memset(api, 0, sizeof(*api));\n"
"    api->mod = LoadLibraryA(\"ws2_32.dll\");\n"
"    if (!api->mod) return 0;\n"
"    api->startup = (int (WSAAPI *)(WORD, LPWSADATA)) GetProcAddress(api->mod, \"WSAStartup\");\n"
"    api->cleanup = (int (WSAAPI *)(void)) GetProcAddress(api->mod, \"WSACleanup\");\n"
"    api->socket_fn = (SOCKET (WSAAPI *)(int, int, int)) GetProcAddress(api->mod, \"socket\");\n"
"    api->connect_fn = (int (WSAAPI *)(SOCKET, const struct sockaddr *, int)) GetProcAddress(api->mod, \"connect\");\n"
"    api->shutdown_fn = (int (WSAAPI *)(SOCKET, int)) GetProcAddress(api->mod, \"shutdown\");\n"
"    api->closesocket_fn = (int (WSAAPI *)(SOCKET)) GetProcAddress(api->mod, \"closesocket\");\n"
"    api->send_fn = (int (WSAAPI *)(SOCKET, const char *, int, int)) GetProcAddress(api->mod, \"send\");\n"
"    api->recv_fn = (int (WSAAPI *)(SOCKET, char *, int, int)) GetProcAddress(api->mod, \"recv\");\n"
"    api->bind_fn = (int (WSAAPI *)(SOCKET, const struct sockaddr *, int)) GetProcAddress(api->mod, \"bind\");\n"
"    api->listen_fn = (int (WSAAPI *)(SOCKET, int)) GetProcAddress(api->mod, \"listen\");\n"
"    api->accept_fn = (SOCKET (WSAAPI *)(SOCKET, struct sockaddr *, int *)) GetProcAddress(api->mod, \"accept\");\n"
"    api->setsockopt_fn = (int (WSAAPI *)(SOCKET, int, int, const char *, int)) GetProcAddress(api->mod, \"setsockopt\");\n"
"    api->getaddrinfo_fn = (int (WSAAPI *)(PCSTR, PCSTR, const ADDRINFOA *, PADDRINFOA *)) GetProcAddress(api->mod, \"getaddrinfo\");\n"
"    api->freeaddrinfo_fn = (void (WSAAPI *)(PADDRINFOA)) GetProcAddress(api->mod, \"freeaddrinfo\");\n"
"    api->sendto_fn = (int (WSAAPI *)(SOCKET, const char *, int, int, const struct sockaddr *, int)) GetProcAddress(api->mod, \"sendto\");\n"
"    api->recvfrom_fn = (int (WSAAPI *)(SOCKET, char *, int, int, struct sockaddr *, int *)) GetProcAddress(api->mod, \"recvfrom\");\n"
"    if (!api->startup || !api->cleanup || !api->socket_fn || !api->connect_fn || !api->shutdown_fn || !api->closesocket_fn || !api->send_fn || !api->recv_fn || !api->bind_fn || !api->listen_fn || !api->accept_fn || !api->setsockopt_fn || !api->getaddrinfo_fn || !api->freeaddrinfo_fn || !api->sendto_fn || !api->recvfrom_fn) {\n"
"        FreeLibrary(api->mod);\n"
"        memset(api, 0, sizeof(*api));\n"
"        return 0;\n"
"    }\n"
"    if (api->startup(MAKEWORD(2, 2), &data) != 0) {\n"
"        FreeLibrary(api->mod);\n"
"        memset(api, 0, sizeof(*api));\n"
"        return 0;\n"
"    }\n"
"    api->started = 1;\n"
"    return 1;\n"
"}\n"
"\n"
"static void spear_socket_api_unload(SpearSocketApi *api) {\n"
"    if (api->started && api->cleanup) api->cleanup();\n"
"    if (api->mod) FreeLibrary(api->mod);\n"
"    memset(api, 0, sizeof(*api));\n"
"}\n"
"\n"
"static int spear_socket_send_all(SpearSocketApi *api, SOCKET sock, const char *data, size_t len) {\n"
"    size_t sent = 0;\n"
"    while (sent < len) {\n"
"        int wrote = api->send_fn(sock, data + sent, (int) ((len - sent) > 16384 ? 16384 : (len - sent)), 0);\n"
"        if (wrote <= 0) return 0;\n"
"        sent += (size_t) wrote;\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_socket_open_client(SpearSocketApi *api, const char *host, const char *port_text, int socktype, int timeout_ms, SOCKET *out) {\n"
"    ADDRINFOA hints;\n"
"    ADDRINFOA *list = NULL;\n"
"    ADDRINFOA *it;\n"
"    SOCKET sock = INVALID_SOCKET;\n"
"    memset(&hints, 0, sizeof(hints));\n"
"    hints.ai_family = AF_UNSPEC;\n"
"    hints.ai_socktype = socktype;\n"
"    if (api->getaddrinfo_fn(host, port_text, &hints, &list) != 0) return 0;\n"
"    for (it = list; it; it = it->ai_next) {\n"
"        sock = api->socket_fn(it->ai_family, it->ai_socktype, it->ai_protocol);\n"
"        if (sock == INVALID_SOCKET) continue;\n"
"        api->setsockopt_fn(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout_ms, sizeof(timeout_ms));\n"
"        api->setsockopt_fn(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout_ms, sizeof(timeout_ms));\n"
"        if (socktype == SOCK_STREAM) {\n"
"            if (api->connect_fn(sock, it->ai_addr, (int) it->ai_addrlen) == 0) break;\n"
"            api->closesocket_fn(sock);\n"
"            sock = INVALID_SOCKET;\n"
"        } else {\n"
"            break;\n"
"        }\n"
"    }\n"
"    api->freeaddrinfo_fn(list);\n"
"    if (sock == INVALID_SOCKET) return 0;\n"
"    *out = sock;\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_socket_open_server(SpearSocketApi *api, const char *host, const char *port_text, SOCKET *out) {\n"
"    ADDRINFOA hints;\n"
"    ADDRINFOA *list = NULL;\n"
"    ADDRINFOA *it;\n"
"    SOCKET sock = INVALID_SOCKET;\n"
"    int reuse = 1;\n"
"    memset(&hints, 0, sizeof(hints));\n"
"    hints.ai_family = AF_INET;\n"
"    hints.ai_socktype = SOCK_STREAM;\n"
"    hints.ai_protocol = IPPROTO_TCP;\n"
"    hints.ai_flags = AI_PASSIVE;\n"
"    if (api->getaddrinfo_fn((host && host[0]) ? host : NULL, port_text, &hints, &list) != 0) return 0;\n"
"    for (it = list; it; it = it->ai_next) {\n"
"        sock = api->socket_fn(it->ai_family, it->ai_socktype, it->ai_protocol);\n"
"        if (sock == INVALID_SOCKET) continue;\n"
"        api->setsockopt_fn(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse));\n"
"        if (api->bind_fn(sock, it->ai_addr, (int) it->ai_addrlen) == 0 && api->listen_fn(sock, SOMAXCONN) == 0) break;\n"
"        api->closesocket_fn(sock);\n"
"        sock = INVALID_SOCKET;\n"
"    }\n"
"    api->freeaddrinfo_fn(list);\n"
"    if (sock == INVALID_SOCKET) return 0;\n"
"    *out = sock;\n"
"    return 1;\n"
"}\n"
"\n"
"static const char *spear_http_status_text(int status) {\n"
"    if (status == 200) return \"OK\";\n"
"    if (status == 201) return \"Created\";\n"
"    if (status == 204) return \"No Content\";\n"
"    if (status == 400) return \"Bad Request\";\n"
"    if (status == 403) return \"Forbidden\";\n"
"    if (status == 404) return \"Not Found\";\n"
"    if (status == 405) return \"Method Not Allowed\";\n"
"    if (status == 500) return \"Internal Server Error\";\n"
"    return \"OK\";\n"
"}\n"
"\n"
"static const char *spear_http_guess_content_type(const char *path) {\n"
"    const char *ext = strrchr(path, '.');\n"
"    if (!ext) return \"application/octet-stream\";\n"
"    if (_stricmp(ext, \".html\") == 0 || _stricmp(ext, \".htm\") == 0) return \"text/html; charset=utf-8\";\n"
"    if (_stricmp(ext, \".css\") == 0) return \"text/css; charset=utf-8\";\n"
"    if (_stricmp(ext, \".js\") == 0) return \"application/javascript; charset=utf-8\";\n"
"    if (_stricmp(ext, \".json\") == 0) return \"application/json; charset=utf-8\";\n"
"    if (_stricmp(ext, \".svg\") == 0) return \"image/svg+xml\";\n"
"    if (_stricmp(ext, \".txt\") == 0) return \"text/plain; charset=utf-8\";\n"
"    return \"application/octet-stream\";\n"
"}\n"
"\n"
"static int spear_http_path_ok(const char *path) {\n"
"    if (!path || !path[0]) return 1;\n"
"    if (strstr(path, \"..\")) return 0;\n"
"    for (size_t i = 0; path[i]; i++) {\n"
"        if (path[i] == ':' || path[i] == '\\\\') return 0;\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_read_blob_file(const char *path, char **out, size_t *len) {\n"
"    FILE *fp = fopen(path, \"rb\");\n"
"    long size;\n"
"    char *data;\n"
"    if (!fp) return 0;\n"
"    fseek(fp, 0, SEEK_END);\n"
"    size = ftell(fp);\n"
"    rewind(fp);\n"
"    if (size < 0) {\n"
"        fclose(fp);\n"
"        return 0;\n"
"    }\n"
"    data = (char *) malloc((size_t) size + 1);\n"
"    if (!data) {\n"
"        fclose(fp);\n"
"        return 0;\n"
"    }\n"
"    if (size > 0 && fread(data, 1, (size_t) size, fp) != (size_t) size) {\n"
"        free(data);\n"
"        fclose(fp);\n"
"        return 0;\n"
"    }\n"
"    fclose(fp);\n"
"    data[size] = '\\0';\n"
"    *out = data;\n"
"    *len = (size_t) size;\n"
"    return 1;\n"
"}\n"
"\n"
"static int spear_http_server_send_response(SpearSocketApi *api, SOCKET client, int status, const char *content_type, const char *body, size_t body_len, int head_only) {\n"
"    char header[1024];\n"
"    int header_len = _snprintf(header, sizeof(header), \"HTTP/1.1 %d %s\\r\\nContent-Type: %s\\r\\nContent-Length: %llu\\r\\nConnection: close\\r\\n\\r\\n\", status, spear_http_status_text(status), content_type ? content_type : \"text/plain; charset=utf-8\", (unsigned long long) body_len);\n"
"    if (header_len <= 0) return 0;\n"
"    if (!spear_socket_send_all(api, client, header, (size_t) header_len)) return 0;\n"
"    if (!head_only && body && body_len > 0) return spear_socket_send_all(api, client, body, body_len);\n"
"    return 1;\n"
"}\n"
"\n"
"static char *spear_native_tcp_request(SpearScope *scope, const char *payload) {\n"
"    SpearSocketApi api;\n"
"    char host[512], message[16384], port_text[32];\n"
"    long long port = spear_json_get_int(payload, \"port\", 0);\n"
"    long long timeout = spear_json_get_int(payload, \"timeout\", 3);\n"
"    SOCKET sock = INVALID_SOCKET;\n"
"    SpearStrBuf out;\n"
"    char chunk[4096];\n"
"    if (!spear_json_get_string(payload, \"host\", host, sizeof(host))) strcpy(host, \"127.0.0.1\");\n"
"    if (!spear_json_get_string(payload, \"message\", message, sizeof(message))) message[0] = '\\0';\n"
"    if (port <= 0 || port > 65535) spear_runtime_fail(\"invalid tcp port\");\n"
"    if (timeout < 1) timeout = 1;\n"
"    if (!spear_socket_api_load(&api)) spear_runtime_fail(\"tcp client unavailable\");\n"
"    snprintf(port_text, sizeof(port_text), \"%lld\", port);\n"
"    if (!spear_socket_open_client(&api, host, port_text, SOCK_STREAM, (int) (timeout * 1000), &sock)) {\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"tcp connect failed\");\n"
"    }\n"
"    if (message[0] && !spear_socket_send_all(&api, sock, message, strlen(message))) {\n"
"        api.closesocket_fn(sock);\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"tcp send failed\");\n"
"    }\n"
"    api.shutdown_fn(sock, SD_SEND);\n"
"    spear_sb_init(&out);\n"
"    for (;;) {\n"
"        int got = api.recv_fn(sock, chunk, sizeof(chunk), 0);\n"
"        if (got == 0) break;\n"
"        if (got < 0) {\n"
"            api.closesocket_fn(sock);\n"
"            spear_socket_api_unload(&api);\n"
"            spear_runtime_fail(\"tcp receive failed\");\n"
"        }\n"
"        spear_sb_append_len(&out, chunk, (size_t) got);\n"
"    }\n"
"    api.closesocket_fn(sock);\n"
"    spear_socket_api_unload(&api);\n"
"    return spear_sb_take(scope, &out);\n"
"}\n"
"\n"
"static char *spear_native_udp_send(SpearScope *scope, const char *payload) {\n"
"    SpearSocketApi api;\n"
"    char host[512], message[16384], port_text[32], reply[4096];\n"
"    long long port = spear_json_get_int(payload, \"port\", 0);\n"
"    long long timeout = spear_json_get_int(payload, \"timeout\", 3);\n"
"    int expect_reply = spear_json_get_bool(payload, \"expect_reply\", 0);\n"
"    SOCKET sock = INVALID_SOCKET;\n"
"    ADDRINFOA hints;\n"
"    ADDRINFOA *list = NULL;\n"
"    int sent;\n"
"    if (!spear_json_get_string(payload, \"host\", host, sizeof(host))) strcpy(host, \"127.0.0.1\");\n"
"    if (!spear_json_get_string(payload, \"message\", message, sizeof(message))) message[0] = '\\0';\n"
"    if (port <= 0 || port > 65535) spear_runtime_fail(\"invalid udp port\");\n"
"    if (timeout < 1) timeout = 1;\n"
"    if (!spear_socket_api_load(&api)) spear_runtime_fail(\"udp client unavailable\");\n"
"    snprintf(port_text, sizeof(port_text), \"%lld\", port);\n"
"    if (!spear_socket_open_client(&api, host, port_text, SOCK_DGRAM, (int) (timeout * 1000), &sock)) {\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"udp connect failed\");\n"
"    }\n"
"    memset(&hints, 0, sizeof(hints));\n"
"    hints.ai_family = AF_UNSPEC;\n"
"    hints.ai_socktype = SOCK_DGRAM;\n"
"    if (api.getaddrinfo_fn(host, port_text, &hints, &list) != 0 || !list) {\n"
"        api.closesocket_fn(sock);\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"udp resolve failed\");\n"
"    }\n"
"    sent = api.sendto_fn(sock, message, (int) strlen(message), 0, list->ai_addr, (int) list->ai_addrlen);\n"
"    api.freeaddrinfo_fn(list);\n"
"    if (sent < 0) {\n"
"        api.closesocket_fn(sock);\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"udp send failed\");\n"
"    }\n"
"    if (!expect_reply) {\n"
"        api.closesocket_fn(sock);\n"
"        spear_socket_api_unload(&api);\n"
"        return spear_text_clone(scope, \"\");\n"
"    }\n"
"    sent = api.recvfrom_fn(sock, reply, sizeof(reply) - 1, 0, NULL, NULL);\n"
"    if (sent < 0) {\n"
"        api.closesocket_fn(sock);\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"udp receive failed\");\n"
"    }\n"
"    reply[sent] = '\\0';\n"
"    api.closesocket_fn(sock);\n"
"    spear_socket_api_unload(&api);\n"
"    return spear_text_clone(scope, reply);\n"
"}\n"
"\n"
"static char *spear_native_http_serve_text(SpearScope *scope, const char *payload) {\n"
"    SpearSocketApi api;\n"
"    char host[128], body[32768], content_type[256], port_text[32], request[8192];\n"
"    long long port = spear_json_get_int(payload, \"port\", 4173);\n"
"    long long status = spear_json_get_int(payload, \"status\", 200);\n"
"    SOCKET server = INVALID_SOCKET;\n"
"    if (!spear_json_get_string(payload, \"host\", host, sizeof(host))) strcpy(host, \"127.0.0.1\");\n"
"    if (!spear_json_get_string(payload, \"body\", body, sizeof(body))) body[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"content_type\", content_type, sizeof(content_type))) strcpy(content_type, \"text/plain; charset=utf-8\");\n"
"    if (port <= 0 || port > 65535) spear_runtime_fail(\"invalid http port\");\n"
"    if (!spear_socket_api_load(&api)) spear_runtime_fail(\"http server unavailable\");\n"
"    snprintf(port_text, sizeof(port_text), \"%lld\", port);\n"
"    if (!spear_socket_open_server(&api, host, port_text, &server)) {\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"http server bind failed\");\n"
"    }\n"
"    for (;;) {\n"
"        SOCKET client = api.accept_fn(server, NULL, NULL);\n"
"        if (client == INVALID_SOCKET) continue;\n"
"        int got = api.recv_fn(client, request, sizeof(request) - 1, 0);\n"
"        int head_only = 0;\n"
"        if (got > 0) {\n"
"            request[got] = '\\0';\n"
"            head_only = strncmp(request, \"HEAD \", 5) == 0;\n"
"        }\n"
"        spear_http_server_send_response(&api, client, (int) status, content_type, body, strlen(body), head_only);\n"
"        api.closesocket_fn(client);\n"
"    }\n"
"    api.closesocket_fn(server);\n"
"    spear_socket_api_unload(&api);\n"
"    return spear_text_clone(scope, \"serving\");\n"
"}\n"
"\n"
"static char *spear_native_http_serve_directory(SpearScope *scope, const char *payload) {\n"
"    SpearSocketApi api;\n"
"    char host[128], directory[MAX_PATH], port_text[32], request[8192], rel[1024], target[MAX_PATH * 2];\n"
"    long long port = spear_json_get_int(payload, \"port\", 4173);\n"
"    SOCKET server = INVALID_SOCKET;\n"
"    if (!spear_json_get_string(payload, \"host\", host, sizeof(host))) strcpy(host, \"127.0.0.1\");\n"
"    if (!spear_json_get_string(payload, \"directory\", directory, sizeof(directory))) strcpy(directory, \".\");\n"
"    if (port <= 0 || port > 65535) spear_runtime_fail(\"invalid http port\");\n"
"    if (!spear_socket_api_load(&api)) spear_runtime_fail(\"http server unavailable\");\n"
"    snprintf(port_text, sizeof(port_text), \"%lld\", port);\n"
"    if (!spear_socket_open_server(&api, host, port_text, &server)) {\n"
"        spear_socket_api_unload(&api);\n"
"        spear_runtime_fail(\"http server bind failed\");\n"
"    }\n"
"    for (;;) {\n"
"        SOCKET client = api.accept_fn(server, NULL, NULL);\n"
"        int got;\n"
"        int head_only = 0;\n"
"        int status = 200;\n"
"        char *file_body = NULL;\n"
"        size_t file_len = 0;\n"
"        const char *ctype = \"text/plain; charset=utf-8\";\n"
"        if (client == INVALID_SOCKET) continue;\n"
"        got = api.recv_fn(client, request, sizeof(request) - 1, 0);\n"
"        if (got <= 0) {\n"
"            api.closesocket_fn(client);\n"
"            continue;\n"
"        }\n"
"        request[got] = '\\0';\n"
"        rel[0] = '\\0';\n"
"        if (strncmp(request, \"GET \", 4) == 0 || strncmp(request, \"HEAD \", 5) == 0) {\n"
"            char *start = strchr(request, ' ');\n"
"            char *end = start ? strchr(start + 1, ' ') : NULL;\n"
"            head_only = strncmp(request, \"HEAD \", 5) == 0;\n"
"            if (start && end && end > start + 1) {\n"
"                size_t len = (size_t) (end - (start + 1));\n"
"                if (len >= sizeof(rel)) len = sizeof(rel) - 1;\n"
"                memcpy(rel, start + 1, len);\n"
"                rel[len] = '\\0';\n"
"            }\n"
"        } else {\n"
"            status = 405;\n"
"        }\n"
"        if (status == 200) {\n"
"            char *q = strchr(rel, '?');\n"
"            char *hash = strchr(rel, '#');\n"
"            if (q) *q = '\\0';\n"
"            if (hash) *hash = '\\0';\n"
"            while (rel[0] == '/') memmove(rel, rel + 1, strlen(rel));\n"
"            if (rel[0] == '\\0') strcpy(rel, \"index.html\");\n"
"            for (size_t i = 0; rel[i]; i++) if (rel[i] == '/') rel[i] = '\\\\';\n"
"            if (!spear_http_path_ok(rel)) {\n"
"                status = 403;\n"
"            } else {\n"
"                snprintf(target, sizeof(target), \"%s\\\\%s\", directory, rel);\n"
"                if (GetFileAttributesA(target) != INVALID_FILE_ATTRIBUTES && (GetFileAttributesA(target) & FILE_ATTRIBUTE_DIRECTORY)) {\n"
"                    size_t pos = strlen(target);\n"
"                    snprintf(target + pos, sizeof(target) - pos, \"\\\\index.html\");\n"
"                }\n"
"                if (!spear_read_blob_file(target, &file_body, &file_len)) {\n"
"                    status = 404;\n"
"                } else {\n"
"                    ctype = spear_http_guess_content_type(target);\n"
"                }\n"
"            }\n"
"        }\n"
"        if (status != 200) {\n"
"            const char *msg = status == 403 ? \"forbidden\" : (status == 404 ? \"not found\" : \"method not allowed\");\n"
"            spear_http_server_send_response(&api, client, status, \"text/plain; charset=utf-8\", msg, strlen(msg), head_only);\n"
"        } else {\n"
"            spear_http_server_send_response(&api, client, 200, ctype, file_body, file_len, head_only);\n"
"        }\n"
"        if (file_body) free(file_body);\n"
"        api.closesocket_fn(client);\n"
"    }\n"
"    api.closesocket_fn(server);\n"
"    spear_socket_api_unload(&api);\n"
"    return spear_text_clone(scope, \"serving\");\n"
"}\n"
"\n"
"static char *spear_native_env_get(SpearScope *scope, const char *payload) {\n"
"    char name[512];\n"
"    char fallback[4096];\n"
"    char *value;\n"
"    if (!spear_json_get_string(payload, \"name\", name, sizeof(name))) name[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"default\", fallback, sizeof(fallback))) fallback[0] = '\\0';\n"
"    value = getenv(name);\n"
"    if (!value || value[0] == '\\0') return spear_text_clone(scope, fallback);\n"
"    return spear_text_clone(scope, value);\n"
"}\n"
"\n"
"static char *spear_ui_render_node(SpearScope *scope, const char *node_json, const char *states_json);\n"
"\n"
"static char *spear_ui_render_children_from_array(SpearScope *scope, const char *items_json, const char *states_json) {\n"
"    SpearStrBuf buf;\n"
"    char item[16384];\n"
"    int index = 0;\n"
"    spear_sb_init(&buf);\n"
"    while (spear_json_array_get(items_json, index++, item, sizeof(item))) {\n"
"        char *child = spear_ui_render_node(scope, item, states_json);\n"
"        spear_sb_append(&buf, child);\n"
"    }\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_ui_render_items_key(SpearScope *scope, const char *node_json, const char *key, const char *states_json) {\n"
"    char items[16384];\n"
"    if (!spear_json_get_raw(node_json, key, items, sizeof(items))) return spear_text_clone(scope, \"\");\n"
"    return spear_ui_render_children_from_array(scope, items, states_json);\n"
"}\n"
"\n"
"static char *spear_ui_render_input(SpearScope *scope, const char *node_json, const char *states_json) {\n"
"    SpearStrBuf buf;\n"
"    char state_name[256], label[512], placeholder[512], hint[1024], error_text[1024], kind[64], value[2048];\n"
"    int required = spear_json_get_bool(node_json, \"required\", 0);\n"
"    int disabled = spear_json_get_bool(node_json, \"disabled\", 0);\n"
"    if (!spear_json_get_string(node_json, \"state\", state_name, sizeof(state_name))) strcpy(state_name, \"value\");\n"
"    if (!spear_json_get_string(node_json, \"label\", label, sizeof(label))) strcpy(label, \"Field\");\n"
"    if (!spear_json_get_string(node_json, \"placeholder\", placeholder, sizeof(placeholder))) placeholder[0] = '\\0';\n"
"    if (!spear_json_get_string(node_json, \"hint\", hint, sizeof(hint))) hint[0] = '\\0';\n"
"    if (!spear_json_get_string(node_json, \"error_text\", error_text, sizeof(error_text))) error_text[0] = '\\0';\n"
"    if (!spear_json_get_string(node_json, \"kind\", kind, sizeof(kind))) strcpy(kind, \"text\");\n"
"    spear_ui_get_state_value(states_json, state_name, value, sizeof(value));\n"
"    if (value[0] == '\\0' && !spear_json_get_string(node_json, \"value\", value, sizeof(value))) value[0] = '\\0';\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"<label style=\\\"display:flex;flex-direction:column;gap:8px\\\"><span style=\\\"font-size:14px;color:#374151\\\">\");\n"
"    spear_sb_append(&buf, spear_html_escape_text(scope, label));\n"
"    spear_sb_append(&buf, \"</span>\");\n"
"    if (_stricmp(kind, \"multiline\") == 0) {\n"
"        spear_sb_appendf(&buf, \"<textarea style=\\\"%s;min-height:120px\\\" name=\\\"%s\\\" data-state-target=\\\"%s\\\" data-kind=\\\"%s\\\"\", error_text[0] ? spear_ui_style(\"field_error\") : spear_ui_style(\"field\"), spear_html_escape_text(scope, state_name), spear_html_escape_text(scope, state_name), spear_html_escape_text(scope, kind));\n"
"        if (placeholder[0]) spear_sb_appendf(&buf, \" placeholder=\\\"%s\\\"\", spear_html_escape_text(scope, placeholder));\n"
"        if (required) spear_sb_append(&buf, \" required=\\\"required\\\"\");\n"
"        if (disabled) spear_sb_append(&buf, \" disabled=\\\"disabled\\\" aria-disabled=\\\"true\\\"\");\n"
"        spear_sb_append(&buf, \">\");\n"
"        spear_sb_append(&buf, spear_html_escape_text(scope, value));\n"
"        spear_sb_append(&buf, \"</textarea>\");\n"
"    } else {\n"
"        const char *input_type = (_stricmp(kind, \"email\") == 0 || _stricmp(kind, \"password\") == 0 || _stricmp(kind, \"search\") == 0) ? kind : \"text\";\n"
"        spear_sb_appendf(&buf, \"<input style=\\\"%s\\\" type=\\\"%s\\\" name=\\\"%s\\\" data-state-target=\\\"%s\\\" data-kind=\\\"%s\\\"\", error_text[0] ? spear_ui_style(\"field_error\") : spear_ui_style(\"field\"), spear_html_escape_text(scope, input_type), spear_html_escape_text(scope, state_name), spear_html_escape_text(scope, state_name), spear_html_escape_text(scope, kind));\n"
"        if (placeholder[0]) spear_sb_appendf(&buf, \" placeholder=\\\"%s\\\"\", spear_html_escape_text(scope, placeholder));\n"
"        if (value[0]) spear_sb_appendf(&buf, \" value=\\\"%s\\\"\", spear_html_escape_text(scope, value));\n"
"        if (required) spear_sb_append(&buf, \" required=\\\"required\\\"\");\n"
"        if (disabled) spear_sb_append(&buf, \" disabled=\\\"disabled\\\" aria-disabled=\\\"true\\\"\");\n"
"        spear_sb_append(&buf, \" />\");\n"
"    }\n"
"    if (hint[0]) spear_sb_appendf(&buf, \"<small style=\\\"color:#6b7280\\\">%s</small>\", spear_html_escape_text(scope, hint));\n"
"    if (error_text[0]) spear_sb_appendf(&buf, \"<small role=\\\"alert\\\" style=\\\"color:#991b1b\\\">%s</small>\", spear_html_escape_text(scope, error_text));\n"
"    spear_sb_append(&buf, \"</label>\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_ui_render_node(SpearScope *scope, const char *node_json, const char *states_json) {\n"
"    SpearStrBuf buf;\n"
"    char kind[64], text[4096], title[1024], route[1024], body[16384], action[4096], left[8192], right[8192], value[256];\n"
"    if (!spear_json_get_string(node_json, \"type\", kind, sizeof(kind))) return spear_text_clone(scope, \"\");\n"
"    if (strcmp(kind, \"column\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<div style=\\\"%s\\\">%s</div>\", spear_ui_style(\"column\"), c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"row\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<div style=\\\"%s\\\">%s</div>\", spear_ui_style(\"row\"), c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"form\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<form style=\\\"%s\\\"\", spear_ui_style(\"form\")); if (spear_json_get_string(node_json, \"title\", title, sizeof(title))) spear_sb_appendf(&buf, \" aria-label=\\\"%s\\\"\", spear_html_escape_text(scope, title)); if (spear_json_get_raw(node_json, \"action\", action, sizeof(action))) spear_ui_append_action_attrs(scope, &buf, action); spear_sb_appendf(&buf, \">%s</form>\", c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"nav\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<nav aria-label=\\\"Primary\\\" style=\\\"%s\\\">%s</nav>\", spear_ui_style(\"nav\"), c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"toolbar\") == 0) { char *lc = spear_json_get_raw(node_json, \"left\", left, sizeof(left)) ? spear_ui_render_children_from_array(scope, left, states_json) : spear_text_clone(scope, \"\"); char *rc = spear_json_get_raw(node_json, \"right\", right, sizeof(right)) ? spear_ui_render_children_from_array(scope, right, states_json) : spear_text_clone(scope, \"\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<header style=\\\"%s\\\"><div style=\\\"%s\\\">%s</div>\", spear_ui_style(\"toolbar\"), spear_ui_style(\"nav\"), lc); if (spear_json_get_string(node_json, \"title\", title, sizeof(title)) && title[0]) spear_sb_appendf(&buf, \"<strong style=\\\"font-size:15px\\\">%s</strong>\", spear_html_escape_text(scope, title)); spear_sb_appendf(&buf, \"<div style=\\\"%s\\\">%s</div></header>\", spear_ui_style(\"nav\"), rc); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"tabs\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<nav aria-label=\\\"Tabs\\\" role=\\\"tablist\\\" style=\\\"%s\\\">%s</nav>\", spear_ui_style(\"tabs\"), c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"list\") == 0) { char *c = spear_ui_render_items_key(scope, node_json, \"items\", states_json); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<ul role=\\\"list\\\" style=\\\"%s\\\">%s</ul>\", spear_ui_style(\"list\"), c); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"item\") == 0) { spear_sb_init(&buf); spear_sb_appendf(&buf, \"<li style=\\\"%s\\\">\", spear_ui_style(\"item\")); if (spear_json_get_string(node_json, \"title\", title, sizeof(title)) && title[0]) spear_sb_appendf(&buf, \"<strong>%s</strong>\", spear_html_escape_text(scope, title)); if (spear_json_get_raw(node_json, \"body\", body, sizeof(body))) { char *c = spear_ui_render_node(scope, body, states_json); spear_sb_append(&buf, c); } spear_sb_append(&buf, \"</li>\"); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"title\") == 0) { if (!spear_json_get_string(node_json, \"text\", text, sizeof(text))) text[0]='\\0'; spear_sb_init(&buf); spear_sb_appendf(&buf, \"<h1 style=\\\"%s\\\">%s</h1>\", spear_ui_style(\"title\"), spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"text\") == 0) { if (!spear_json_get_string(node_json, \"text\", text, sizeof(text))) text[0]='\\0'; spear_sb_init(&buf); spear_sb_appendf(&buf, \"<p style=\\\"%s\\\">%s</p>\", spear_ui_style(\"text\"), spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"loading\") == 0) { if (!spear_json_get_string(node_json, \"text\", text, sizeof(text))) strcpy(text, \"Loading\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section aria-live=\\\"polite\\\" style=\\\"padding:18px;border:1px dashed #cbd5e1;border-radius:18px;background:#ffffff;color:#6b7280\\\">%s...</section>\", spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"empty\") == 0) { if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) strcpy(title, \"Empty\"); if (!spear_json_get_string(node_json, \"text\", text, sizeof(text))) text[0]='\\0'; spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section style=\\\"padding:22px;border:1px dashed #d1d5db;border-radius:20px;background:#ffffff\\\"><h3 style=\\\"margin:0 0 8px 0\\\">%s</h3><p style=\\\"margin:0;color:#6b7280\\\">%s</p></section>\", spear_html_escape_text(scope, title), spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"error\") == 0) { if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) strcpy(title, \"Error\"); if (!spear_json_get_string(node_json, \"text\", text, sizeof(text))) text[0]='\\0'; spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section role=\\\"alert\\\" style=\\\"padding:22px;border:1px solid #fecaca;border-radius:20px;background:#fff1f2\\\"><h3 style=\\\"margin:0 0 8px 0;color:#991b1b\\\">%s</h3><p style=\\\"margin:0;color:#7f1d1d\\\">%s</p></section>\", spear_html_escape_text(scope, title), spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"button\") == 0) { if (!spear_json_get_string(node_json, \"label\", text, sizeof(text))) strcpy(text, \"Action\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<button type=\\\"button\\\" style=\\\"%s\\\"\", spear_ui_style(\"button\")); if (spear_json_get_raw(node_json, \"action\", action, sizeof(action))) spear_ui_append_action_attrs(scope, &buf, action); if (spear_json_get_bool(node_json, \"disabled\", 0)) spear_sb_append(&buf, \" disabled=\\\"disabled\\\" aria-disabled=\\\"true\\\"\"); spear_sb_appendf(&buf, \">%s</button>\", spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"link\") == 0) { if (!spear_json_get_string(node_json, \"label\", text, sizeof(text))) strcpy(text, \"Open\"); if (!spear_json_get_string(node_json, \"route\", route, sizeof(route))) strcpy(route, \"#\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<a href=\\\"%s\\\" style=\\\"%s\\\"\", spear_html_escape_text(scope, route), spear_ui_style(spear_json_get_bool(node_json, \"tab\", 0) ? \"tab\" : \"button_secondary\")); if (spear_json_get_raw(node_json, \"action\", action, sizeof(action))) spear_ui_append_action_attrs(scope, &buf, action); if (spear_json_get_bool(node_json, \"tab\", 0)) spear_sb_append(&buf, \" role=\\\"tab\\\"\"); spear_sb_appendf(&buf, \">%s</a>\", spear_html_escape_text(scope, text)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"input\") == 0) return spear_ui_render_input(scope, node_json, states_json);\n"
"    if (strcmp(kind, \"stat\") == 0) { if (!spear_json_get_string(node_json, \"label\", title, sizeof(title))) strcpy(title, \"Metric\"); if (!spear_json_get_string(node_json, \"value\", value, sizeof(value))) strcpy(value, \"0\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section style=\\\"%s\\\"><p style=\\\"margin:0;font-size:14px;color:#6b7280\\\">%s</p><h3 style=\\\"margin:0;font-size:28px;line-height:1.05\\\">%s</h3></section>\", spear_ui_style(\"stat\"), spear_html_escape_text(scope, title), spear_html_escape_text(scope, value)); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"card\") == 0 || strcmp(kind, \"section\") == 0) { const char *style_name = strcmp(kind, \"card\") == 0 ? \"card\" : \"section\"; const char *heading_style = strcmp(kind, \"card\") == 0 ? \"margin:0;font-size:24px;line-height:1.1\" : \"margin:0;font-size:28px;line-height:1.1\"; spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section style=\\\"%s\\\">\", spear_ui_style(style_name)); if (spear_json_get_string(node_json, \"title\", title, sizeof(title)) && title[0]) spear_sb_appendf(&buf, \"<h2 style=\\\"%s\\\">%s</h2>\", heading_style, spear_html_escape_text(scope, title)); if (spear_json_get_raw(node_json, \"body\", body, sizeof(body))) { char *c = spear_ui_render_node(scope, body, states_json); spear_sb_append(&buf, c); } spear_sb_append(&buf, \"</section>\"); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"dialog\") == 0) { if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) strcpy(title, \"Dialog\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section style=\\\"%s\\\" role=\\\"dialog\\\" aria-modal=\\\"true\\\"\", spear_ui_style(\"dialog\")); if (spear_json_get_raw(node_json, \"action\", action, sizeof(action))) spear_ui_append_action_attrs(scope, &buf, action); spear_sb_appendf(&buf, \"><h2 style=\\\"margin:0\\\">%s</h2>\", spear_html_escape_text(scope, title)); if (spear_json_get_raw(node_json, \"body\", body, sizeof(body))) { char *c = spear_ui_render_node(scope, body, states_json); spear_sb_append(&buf, c); } spear_sb_append(&buf, \"</section>\"); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"sheet\") == 0) { if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) strcpy(title, \"Sheet\"); spear_sb_init(&buf); spear_sb_appendf(&buf, \"<section style=\\\"%s\\\" role=\\\"region\\\" aria-label=\\\"%s\\\"><h2 style=\\\"margin:0\\\">%s</h2>\", spear_ui_style(\"sheet\"), spear_html_escape_text(scope, title), spear_html_escape_text(scope, title)); if (spear_json_get_raw(node_json, \"body\", body, sizeof(body))) { char *c = spear_ui_render_node(scope, body, states_json); spear_sb_append(&buf, c); } spear_sb_append(&buf, \"</section>\"); return spear_sb_take(scope, &buf); }\n"
"    if (strcmp(kind, \"divider\") == 0) { spear_sb_init(&buf); spear_sb_appendf(&buf, \"<hr style=\\\"%s\\\" />\", spear_ui_style(\"divider\")); return spear_sb_take(scope, &buf); }\n"
"    return spear_text_clone(scope, \"\");\n"
"}\n"
"\n"
"static char *spear_native_ui_web_render(SpearScope *scope, const char *payload) {\n"
"    char screen[32768], body[32768], states[16384], title[1024], route[1024], screen_title[1024], item[4096], name[256], value_raw[1024];\n"
"    SpearStrBuf buf;\n"
"    char *body_html;\n"
"    int idx = 0;\n"
"    if (!spear_json_get_raw(payload, \"body\", body, sizeof(body))) strcpy(body, payload);\n"
"    if (body[0] == '{' && spear_json_get_string(body, \"type\", title, sizeof(title)) && strcmp(title, \"screen\") == 0) strcpy(screen, body); else strcpy(screen, payload);\n"
"    if (!spear_json_get_string(screen, \"title\", title, sizeof(title))) strcpy(title, \"Spear UI\");\n"
"    if (!spear_json_get_string(screen, \"route\", route, sizeof(route))) route[0] = '\\0';\n"
"    if (!spear_json_get_string(screen, \"title\", screen_title, sizeof(screen_title))) screen_title[0] = '\\0';\n"
"    if (!spear_json_get_raw(screen, \"states\", states, sizeof(states))) strcpy(states, \"[]\");\n"
"    if (!spear_json_get_raw(screen, \"body\", body, sizeof(body))) strcpy(body, \"{}\");\n"
"    body_html = spear_ui_render_node(scope, body, states);\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"<!doctype html><html><head><meta charset=\\\"utf-8\\\"><meta name=\\\"viewport\\\" content=\\\"width=device-width,initial-scale=1\\\"><title>\");\n"
"    spear_sb_append(&buf, spear_html_escape_text(scope, title));\n"
"    spear_sb_appendf(&buf, \"</title><style>*{box-sizing:border-box}html,body{margin:0;padding:0}h1,h2,h3,p,ul,li,blockquote,pre{margin:0;padding:0}ul{padding-left:0}img,svg,video,canvas{display:block;max-width:100%%}table{border-collapse:collapse;border-spacing:0}button,input,textarea,select{font:inherit;color:inherit}body{%s}main{%s}</style></head><body><main\", spear_ui_style(\"body\"), spear_ui_style(\"main\"));\n"
"    if (route[0]) spear_sb_appendf(&buf, \" data-route=\\\"%s\\\"\", spear_html_escape_text(scope, route));\n"
"    if (screen_title[0]) spear_sb_appendf(&buf, \" data-screen-title=\\\"%s\\\"\", spear_html_escape_text(scope, screen_title));\n"
"    while (spear_json_array_get(states, idx++, item, sizeof(item))) {\n"
"        if (!spear_json_get_string(item, \"name\", name, sizeof(name))) continue;\n"
"        if (!spear_json_get_string(item, \"value\", value_raw, sizeof(value_raw)) && !spear_json_get_raw(item, \"value\", value_raw, sizeof(value_raw))) value_raw[0] = '\\0';\n"
"        spear_sb_appendf(&buf, \" data-state-%s=\\\"%s\\\"\", spear_html_escape_text(scope, name), spear_html_escape_text(scope, value_raw));\n"
"    }\n"
"    spear_sb_append(&buf, \">\");\n"
"    spear_sb_append(&buf, body_html);\n"
"    spear_sb_append(&buf, \"</main></body></html>\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_kotlin_string(SpearScope *scope, const char *text) {\n"
"    char *escaped = spear_json_escape(scope, text ? text : \"\");\n"
"    size_t total = strlen(escaped) + 3;\n"
"    char *dst = (char *) spear_alloc(scope, total);\n"
"    snprintf(dst, total, \"\\\"%s\\\"\", escaped);\n"
"    return dst;\n"
"}\n"
"\n"
"static void spear_sb_indent(SpearStrBuf *buf, int indent) {\n"
"    for (int i = 0; i < indent; i++) spear_sb_append(buf, \" \");\n"
"}\n"
"\n"
"static char *spear_ui_compose_action_expr(SpearScope *scope, const char *action_json) {\n"
"    SpearStrBuf buf;\n"
"    char value[1024];\n"
"    int count = 0;\n"
"    if (!action_json || action_json[0] != '{') return spear_text_clone(scope, \"spearAction()\");\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"spearAction(\");\n"
"    if (spear_json_get_string(action_json, \"type\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"\\\"type\\\" to %s\", spear_kotlin_string(scope, value));\n"
"        count++;\n"
"    }\n"
"    if (spear_json_get_string(action_json, \"name\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"name\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, value));\n"
"        count++;\n"
"    }\n"
"    if (spear_json_get_string(action_json, \"route\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"route\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, value));\n"
"        count++;\n"
"    }\n"
"    if (spear_json_get_string(action_json, \"dialog\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"dialog\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, value));\n"
"        count++;\n"
"    }\n"
"    if (spear_json_get_string(action_json, \"state_target\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"state_target\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, value));\n"
"        count++;\n"
"    }\n"
"    if (spear_json_get_string(action_json, \"state_value\", value, sizeof(value)) || spear_json_get_raw(action_json, \"state_value\", value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"state_value\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, value));\n"
"    }\n"
"    spear_sb_append(&buf, \")\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_ui_compose_render_node(SpearScope *scope, const char *node_json, int indent);\n"
"\n"
"static char *spear_ui_compose_render_children(SpearScope *scope, const char *items_json, int indent) {\n"
"    SpearStrBuf buf;\n"
"    char item[16384];\n"
"    int index = 0;\n"
"    spear_sb_init(&buf);\n"
"    while (spear_json_array_get(items_json, index++, item, sizeof(item))) {\n"
"        char *child = spear_ui_compose_render_node(scope, item, indent);\n"
"        spear_sb_append(&buf, child);\n"
"    }\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_ui_compose_render_items_key(SpearScope *scope, const char *node_json, const char *key, int indent) {\n"
"    char items[16384];\n"
"    if (!spear_json_get_raw(node_json, key, items, sizeof(items))) return spear_text_clone(scope, \"\");\n"
"    return spear_ui_compose_render_children(scope, items, indent);\n"
"}\n"
"\n"
"static char *spear_ui_compose_render_node(SpearScope *scope, const char *node_json, int indent) {\n"
"    SpearStrBuf buf;\n"
"    char kind[64], text[4096], title[1024], route[1024], body[16384], action[4096], left[8192], right[8192], value[256], state[256], placeholder[1024], hint[1024], error_text[1024], label[1024];\n"
"    if (!spear_json_get_string(node_json, \"type\", kind, sizeof(kind))) return spear_text_clone(scope, \"\");\n"
"    if (strcmp(kind, \"column\") == 0 || strcmp(kind, \"form\") == 0 || strcmp(kind, \"list\") == 0 || strcmp(kind, \"nav\") == 0 || strcmp(kind, \"tabs\") == 0) {\n"
"        const char *arrangement = strcmp(kind, \"nav\") == 0 ? \"12\" : (strcmp(kind, \"tabs\") == 0 ? \"10\" : (strcmp(kind, \"list\") == 0 ? \"12\" : \"16\"));\n"
"        char *children = spear_ui_compose_render_items_key(scope, node_json, \"items\", indent + 4);\n"
"        spear_sb_init(&buf);\n"
"        spear_sb_indent(&buf, indent);\n"
"        if (strcmp(kind, \"nav\") == 0 || strcmp(kind, \"tabs\") == 0) spear_sb_appendf(&buf, \"Row(horizontalArrangement = Arrangement.spacedBy(%s.dp)) {\\n\", arrangement);\n"
"        else spear_sb_appendf(&buf, \"Column(verticalArrangement = Arrangement.spacedBy(%s.dp)) {\\n\", arrangement);\n"
"        spear_sb_append(&buf, children);\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"row\") == 0) {\n"
"        char *children = spear_ui_compose_render_items_key(scope, node_json, \"items\", indent + 4);\n"
"        spear_sb_init(&buf);\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {\\n\");\n"
"        spear_sb_append(&buf, children);\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"toolbar\") == 0) {\n"
"        char *left_children = spear_json_get_raw(node_json, \"left\", left, sizeof(left)) ? spear_ui_compose_render_children(scope, left, indent + 8) : spear_text_clone(scope, \"\");\n"
"        char *right_children = spear_json_get_raw(node_json, \"right\", right, sizeof(right)) ? spear_ui_compose_render_children(scope, right, indent + 8) : spear_text_clone(scope, \"\");\n"
"        if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) title[0] = '\\0';\n"
"        spear_sb_init(&buf);\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {\\n\");\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {\\n\");\n"
"        spear_sb_append(&buf, left_children);\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        if (title[0]) {\n"
"            spear_sb_indent(&buf, indent + 4);\n"
"            spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleMedium)\\n\", spear_kotlin_string(scope, title));\n"
"        }\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {\\n\");\n"
"        spear_sb_append(&buf, right_children);\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"item\") == 0) {\n"
"        char *child = spear_json_get_raw(node_json, \"body\", body, sizeof(body)) ? spear_ui_compose_render_node(scope, body, indent + 8) : spear_text_clone(scope, \"\");\n"
"        if (!spear_json_get_string(node_json, \"title\", title, sizeof(title))) title[0] = '\\0';\n"
"        spear_sb_init(&buf);\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"Card(modifier = Modifier.fillMaxWidth()) {\\n\");\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {\\n\");\n"
"        if (title[0]) {\n"
"            spear_sb_indent(&buf, indent + 8);\n"
"            spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleSmall)\\n\", spear_kotlin_string(scope, title));\n"
"        }\n"
"        spear_sb_append(&buf, child);\n"
"        spear_sb_indent(&buf, indent + 4);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        spear_sb_indent(&buf, indent);\n"
"        spear_sb_append(&buf, \"}\\n\");\n"
"        return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"title\") == 0) {\n"
"        spear_json_get_string(node_json, \"text\", text, sizeof(text));\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.headlineLarge)\\n\", spear_kotlin_string(scope, text)); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"text\") == 0) {\n"
"        spear_json_get_string(node_json, \"text\", text, sizeof(text));\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodyLarge)\\n\", spear_kotlin_string(scope, text)); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"notice\") == 0) {\n"
"        spear_json_get_string(node_json, \"title\", title, sizeof(title));\n"
"        spear_json_get_string(node_json, \"text\", text, sizeof(text));\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"Card(modifier = Modifier.fillMaxWidth()) {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {\\n\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleMedium)\\n\", spear_kotlin_string(scope, title[0] ? title : \"Notice\")); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodyMedium)\\n\", spear_kotlin_string(scope, text)); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"}\\n\"); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"loading\") == 0 || strcmp(kind, \"empty\") == 0 || strcmp(kind, \"error\") == 0) {\n"
"        const char *default_title = strcmp(kind, \"loading\") == 0 ? \"Loading\" : (strcmp(kind, \"empty\") == 0 ? \"Empty\" : \"Error\");\n"
"        spear_json_get_string(node_json, \"title\", title, sizeof(title));\n"
"        spear_json_get_string(node_json, \"text\", text, sizeof(text));\n"
"        spear_sb_init(&buf);\n"
"        if (strcmp(kind, \"loading\") == 0) { spear_sb_indent(&buf, indent); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodyMedium)\\n\", spear_kotlin_string(scope, text[0] ? text : default_title)); return spear_sb_take(scope, &buf); }\n"
"        spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"Card(modifier = Modifier.fillMaxWidth()) {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {\\n\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleMedium)\\n\", spear_kotlin_string(scope, title[0] ? title : default_title)); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodyMedium)\\n\", spear_kotlin_string(scope, text)); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"}\\n\"); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"button\") == 0) {\n"
"        char *action_expr;\n"
"        spear_json_get_string(node_json, \"label\", text, sizeof(text));\n"
"        action_expr = spear_json_get_raw(node_json, \"action\", action, sizeof(action)) ? spear_ui_compose_action_expr(scope, action) : spear_text_clone(scope, \"spearAction()\");\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"Button(onClick = {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_appendf(&buf, \"val actionMeta = %s\\n\", action_expr); spear_sb_indent(&buf, indent); spear_sb_appendf(&buf, \"}) { Text(text = %s) }\\n\", spear_kotlin_string(scope, text[0] ? text : \"Action\")); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"link\") == 0) {\n"
"        char fallback_action[2048];\n"
"        char *action_expr;\n"
"        spear_json_get_string(node_json, \"label\", text, sizeof(text));\n"
"        spear_json_get_string(node_json, \"route\", route, sizeof(route));\n"
"        if (!spear_json_get_raw(node_json, \"action\", action, sizeof(action))) snprintf(fallback_action, sizeof(fallback_action), \"{\\\"type\\\":\\\"go\\\",\\\"route\\\":\\\"%s\\\"}\", route); else snprintf(fallback_action, sizeof(fallback_action), \"%s\", action);\n"
"        action_expr = spear_ui_compose_action_expr(scope, fallback_action);\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"TextButton(onClick = {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_appendf(&buf, \"val actionMeta = %s\\n\", action_expr); spear_sb_indent(&buf, indent); spear_sb_appendf(&buf, \"}) { Text(text = %s) }\\n\", spear_kotlin_string(scope, text[0] ? text : \"Open\")); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"input\") == 0) {\n"
"        int disabled = spear_json_get_bool(node_json, \"disabled\", 0);\n"
"        spear_json_get_string(node_json, \"state\", state, sizeof(state));\n"
"        spear_json_get_string(node_json, \"label\", label, sizeof(label));\n"
"        spear_json_get_string(node_json, \"placeholder\", placeholder, sizeof(placeholder));\n"
"        spear_json_get_string(node_json, \"hint\", hint, sizeof(hint));\n"
"        spear_json_get_string(node_json, \"error_text\", error_text, sizeof(error_text));\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"OutlinedTextField(\\n\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"value = %s,\\n\", state[0] ? state : \"value\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"onValueChange = { %s = it },\\n\", state[0] ? state : \"value\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"label = { Text(text = %s) },\\n\", spear_kotlin_string(scope, label[0] ? label : \"Field\")); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"placeholder = { Text(text = %s) },\\n\", spear_kotlin_string(scope, placeholder)); spear_sb_indent(&buf, indent + 8); spear_sb_append(&buf, \"modifier = Modifier.fillMaxWidth(),\\n\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"enabled = %s,\\n\", disabled ? \"false\" : \"true\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"isError = %s\\n\", error_text[0] ? \"true\" : \"false\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \")\\n\"); if (hint[0]) { spear_sb_indent(&buf, indent + 4); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodySmall)\\n\", spear_kotlin_string(scope, hint)); } if (error_text[0]) { spear_sb_indent(&buf, indent + 4); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.bodySmall)\\n\", spear_kotlin_string(scope, error_text)); } spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"stat\") == 0) {\n"
"        spear_json_get_string(node_json, \"label\", label, sizeof(label)); spear_json_get_string(node_json, \"value\", value, sizeof(value)); spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"Card(modifier = Modifier.fillMaxWidth()) {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {\\n\"); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.labelMedium)\\n\", spear_kotlin_string(scope, label[0] ? label : \"Metric\")); spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.headlineSmall)\\n\", spear_kotlin_string(scope, value[0] ? value : \"0\")); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"}\\n\"); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"card\") == 0 || strcmp(kind, \"section\") == 0 || strcmp(kind, \"dialog\") == 0 || strcmp(kind, \"sheet\") == 0) {\n"
"        char *child = spear_json_get_raw(node_json, \"body\", body, sizeof(body)) ? spear_ui_compose_render_node(scope, body, indent + 8) : spear_text_clone(scope, \"\");\n"
"        spear_json_get_string(node_json, \"title\", title, sizeof(title));\n"
"        spear_sb_init(&buf); spear_sb_indent(&buf, indent); if (strcmp(kind, \"section\") == 0) { spear_sb_append(&buf, \"Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {\\n\"); if (title[0]) { spear_sb_indent(&buf, indent + 4); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleLarge)\\n\", spear_kotlin_string(scope, title)); } spear_sb_append(&buf, child); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf); } spear_sb_append(&buf, \"Card(modifier = Modifier.fillMaxWidth()) {\\n\"); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {\\n\"); if (title[0]) { spear_sb_indent(&buf, indent + 8); spear_sb_appendf(&buf, \"Text(text = %s, style = MaterialTheme.typography.titleLarge)\\n\", spear_kotlin_string(scope, title)); } spear_sb_append(&buf, child); spear_sb_indent(&buf, indent + 4); spear_sb_append(&buf, \"}\\n\"); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"}\\n\"); return spear_sb_take(scope, &buf);\n"
"    }\n"
"    if (strcmp(kind, \"divider\") == 0) { spear_sb_init(&buf); spear_sb_indent(&buf, indent); spear_sb_append(&buf, \"HorizontalDivider()\\n\"); return spear_sb_take(scope, &buf); }\n"
"    return spear_text_clone(scope, \"\");\n"
"}\n"
"\n"
"static char *spear_ui_compose_state_lines(SpearScope *scope, const char *states_json) {\n"
"    SpearStrBuf buf;\n"
"    char item[4096], name[256], value[1024], state_type[64];\n"
"    int index = 0;\n"
"    spear_sb_init(&buf);\n"
"    while (spear_json_array_get(states_json, index++, item, sizeof(item))) {\n"
"        if (!spear_json_get_string(item, \"name\", name, sizeof(name))) strcpy(name, \"state\");\n"
"        if (!spear_json_get_string(item, \"state_type\", state_type, sizeof(state_type))) strcpy(state_type, \"text\");\n"
"        if (strcmp(state_type, \"num\") == 0) {\n"
"            if (!spear_json_get_raw(item, \"value\", value, sizeof(value))) strcpy(value, \"0\");\n"
"            spear_sb_appendf(&buf, \"    var %s by remember { mutableIntStateOf(%s) }\\n\", name, value);\n"
"        } else {\n"
"            if (!spear_json_get_string(item, \"value\", value, sizeof(value))) value[0] = '\\0';\n"
"            spear_sb_appendf(&buf, \"    var %s by remember { mutableStateOf(%s) }\\n\", name, spear_kotlin_string(scope, value));\n"
"        }\n"
"    }\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_ui_compose_screen_meta(SpearScope *scope, const char *route, const char *title) {\n"
"    SpearStrBuf buf;\n"
"    int count = 0;\n"
"    if ((!route || !route[0]) && (!title || !title[0])) return spear_text_clone(scope, \"\");\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"    val screenMeta = mapOf(\");\n"
"    if (route && route[0]) { spear_sb_appendf(&buf, \"\\\"route\\\" to %s\", spear_kotlin_string(scope, route)); count++; }\n"
"    if (title && title[0]) { spear_sb_appendf(&buf, \"%s\\\"title\\\" to %s\", count ? \", \" : \"\", spear_kotlin_string(scope, title)); }\n"
"    spear_sb_append(&buf, \")\\n\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_ui_compose_render(SpearScope *scope, const char *payload, int desktop) {\n"
"    char screen[32768], body[32768], states[16384], title[1024], route[1024], screen_name[256], kind[64];\n"
"    char *state_lines;\n"
"    char *meta_lines;\n"
"    char *body_code;\n"
"    SpearStrBuf buf;\n"
"    if (!spear_json_get_string(payload, \"screen_name\", screen_name, sizeof(screen_name))) strcpy(screen_name, desktop ? \"DesktopScreen\" : \"MainScreen\");\n"
"    if (!spear_json_get_raw(payload, \"body\", body, sizeof(body))) strcpy(body, payload);\n"
"    if (body[0] == '{' && spear_json_get_string(body, \"type\", kind, sizeof(kind)) && strcmp(kind, \"screen\") == 0) strcpy(screen, body); else strcpy(screen, payload);\n"
"    if (!spear_json_get_string(screen, \"title\", title, sizeof(title))) strcpy(title, screen_name);\n"
"    if (!spear_json_get_string(screen, \"route\", route, sizeof(route))) route[0] = '\\0';\n"
"    if (!spear_json_get_raw(screen, \"states\", states, sizeof(states))) strcpy(states, \"[]\");\n"
"    if (!spear_json_get_raw(screen, \"body\", body, sizeof(body))) strcpy(body, \"{}\");\n"
"    state_lines = spear_ui_compose_state_lines(scope, states);\n"
"    meta_lines = spear_ui_compose_screen_meta(scope, route, title);\n"
"    body_code = spear_ui_compose_render_node(scope, body, 8);\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"import androidx.compose.foundation.layout.*\\nimport androidx.compose.material3.*\\nimport androidx.compose.runtime.*\\nimport androidx.compose.ui.Modifier\\nimport androidx.compose.ui.graphics.Color\\nimport androidx.compose.ui.unit.dp\\n\");\n"
"    if (desktop) spear_sb_append(&buf, \"import androidx.compose.ui.window.Window\\nimport androidx.compose.ui.window.application\\n\");\n"
"    spear_sb_append(&buf, \"\\nprivate fun spearAction(vararg pairs: Pair<String, String>): Map<String, String> = mapOf(*pairs)\\n\\n\");\n"
"    spear_sb_appendf(&buf, \"@Composable\\nfun %s() {\\n\", screen_name);\n"
"    spear_sb_append(&buf, state_lines);\n"
"    spear_sb_append(&buf, meta_lines);\n"
"    spear_sb_append(&buf, \"    Surface(modifier = Modifier.fillMaxSize(), color = Color(0xFFF5F3EE)) {\\n        Column(\\n            modifier = Modifier.fillMaxSize().padding(24.dp),\\n            verticalArrangement = Arrangement.spacedBy(16.dp)\\n        ) {\\n\");\n"
"    spear_sb_append(&buf, body_code);\n"
"    spear_sb_append(&buf, \"        }\\n    }\\n}\\n\");\n"
"    if (desktop) {\n"
"        spear_sb_appendf(&buf, \"\\nfun main() = application {\\n    Window(onCloseRequest = ::exitApplication, title = %s) {\\n        %s()\\n    }\\n}\\n\", spear_kotlin_string(scope, title), screen_name);\n"
"    }\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_ui_android_render(SpearScope *scope, const char *payload) {\n"
"    return spear_native_ui_compose_render(scope, payload, 0);\n"
"}\n"
"\n"
"static char *spear_native_ui_desktop_render(SpearScope *scope, const char *payload) {\n"
"    return spear_native_ui_compose_render(scope, payload, 1);\n"
"}\n"
"\n"
"static char *spear_native_json_quote(SpearScope *scope, const char *payload) {\n"
    "    char quoted[8192];\n"
    "    size_t len = 0;\n"
"    quoted[len++] = '\"';\n"
"    for (size_t i = 0; payload[i] && len + 3 < sizeof(quoted); i++) {\n"
"        char ch = payload[i];\n"
"        if (ch == '\\\\' || ch == '\"') {\n"
"            quoted[len++] = '\\\\';\n"
"            quoted[len++] = ch;\n"
"        } else if (ch == '\\n') {\n"
"            quoted[len++] = '\\\\';\n"
"            quoted[len++] = 'n';\n"
"        } else if (ch == '\\r') {\n"
"            quoted[len++] = '\\\\';\n"
"            quoted[len++] = 'r';\n"
"        } else if (ch == '\\t') {\n"
"            quoted[len++] = '\\\\';\n"
"            quoted[len++] = 't';\n"
"        } else {\n"
"            quoted[len++] = ch;\n"
"        }\n"
"    }\n"
"    quoted[len++] = '\"';\n"
"    quoted[len] = '\\0';\n"
"    return spear_text_clone(scope, quoted);\n"
"}\n"
"\n"
"static char *spear_native_json_format(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    return spear_text_clone(scope, text);\n"
"}\n"
"\n"
"static char *spear_native_json_get(SpearScope *scope, const char *payload) {\n"
"    char text[8192];\n"
"    char key[256];\n"
"    char fallback[4096];\n"
"    const char *value = NULL;\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"default\", fallback, sizeof(fallback))) fallback[0] = '\\0';\n"
"    if (!spear_json_seek_key(text, key, &value)) return spear_text_clone(scope, fallback);\n"
"    if (*value == '\"') {\n"
"        char extracted[4096];\n"
"        if (!spear_json_get_string(text, key, extracted, sizeof(extracted))) return spear_text_clone(scope, fallback);\n"
"        return spear_text_clone(scope, extracted);\n"
"    }\n"
"    const char *end = value;\n"
"    int depth = 0;\n"
"    if (*value == '[' || *value == '{') {\n"
"        char open = *value;\n"
"        char close = open == '[' ? ']' : '}';\n"
"        while (*end) {\n"
"            if (*end == open) depth++;\n"
"            else if (*end == close) {\n"
"                depth--;\n"
"                if (depth == 0) { end++; break; }\n"
"            }\n"
"            end++;\n"
"        }\n"
"    } else {\n"
"        while (*end && *end != ',' && *end != '}') end++;\n"
"    }\n"
"    while (end > value && (end[-1] == ' ' || end[-1] == '\\r' || end[-1] == '\\n' || end[-1] == '\\t')) end--;\n"
"    size_t out_len = (size_t) (end - value);\n"
"    char *dst = (char *) spear_alloc(scope, out_len + 1);\n"
"    memcpy(dst, value, out_len);\n"
"    dst[out_len] = '\\0';\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_json_keys(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char key[1024];\n"
"    char value[8192];\n"
"    int offset = 0;\n"
"    SpearStrBuf buf;\n"
"    int first = 1;\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"[\");\n"
"    while (spear_json_object_next(text, &offset, key, sizeof(key), value, sizeof(value))) {\n"
"        spear_sb_appendf(&buf, \"%s\\\"%s\\\"\", first ? \"\" : \",\", spear_json_escape(scope, key));\n"
"        first = 0;\n"
"    }\n"
"    spear_sb_append(&buf, \"]\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_map_get(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char key[1024];\n"
"    char fallback_raw[8192];\n"
"    char value[8192];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    if (!spear_json_get_raw(payload, \"default\", fallback_raw, sizeof(fallback_raw))) strcpy(fallback_raw, \"\\\"\\\"\");\n"
"    if (spear_json_get_raw(text, key, value, sizeof(value))) return spear_json_value_to_text(scope, value);\n"
"    return spear_json_value_to_text(scope, fallback_raw);\n"
"}\n"
"\n"
"static char *spear_native_map_has(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char key[1024];\n"
"    char value[8192];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_string(payload, \"key\", key, sizeof(key))) key[0] = '\\0';\n"
"    return spear_text_clone(scope, spear_json_get_raw(text, key, value, sizeof(value)) ? \"1\" : \"0\");\n"
"}\n"
"\n"
"static char *spear_native_map_set(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char target[1024];\n"
"    char new_value[8192];\n"
"    char key[1024];\n"
"    char value[8192];\n"
"    int offset = 0;\n"
"    int first = 1;\n"
"    int replaced = 0;\n"
"    SpearStrBuf buf;\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_string(payload, \"key\", target, sizeof(target))) target[0] = '\\0';\n"
"    if (!spear_json_get_raw(payload, \"value\", new_value, sizeof(new_value))) strcpy(new_value, \"\\\"\\\"\");\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"{\");\n"
"    while (spear_json_object_next(text, &offset, key, sizeof(key), value, sizeof(value))) {\n"
"        if (strcmp(key, target) == 0) {\n"
"            spear_sb_appendf(&buf, \"%s\\\"%s\\\":%s\", first ? \"\" : \",\", spear_json_escape(scope, key), new_value);\n"
"            first = 0;\n"
"            replaced = 1;\n"
"            continue;\n"
"        }\n"
"        spear_sb_appendf(&buf, \"%s\\\"%s\\\":%s\", first ? \"\" : \",\", spear_json_escape(scope, key), value);\n"
"        first = 0;\n"
"    }\n"
"    if (!replaced) spear_sb_appendf(&buf, \"%s\\\"%s\\\":%s\", first ? \"\" : \",\", spear_json_escape(scope, target), new_value);\n"
"    spear_sb_append(&buf, \"}\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_map_remove(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char target[1024];\n"
"    char key[1024];\n"
"    char value[8192];\n"
"    int offset = 0;\n"
"    int first = 1;\n"
"    SpearStrBuf buf;\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_string(payload, \"key\", target, sizeof(target))) target[0] = '\\0';\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"{\");\n"
"    while (spear_json_object_next(text, &offset, key, sizeof(key), value, sizeof(value))) {\n"
"        if (strcmp(key, target) == 0) continue;\n"
"        spear_sb_appendf(&buf, \"%s\\\"%s\\\":%s\", first ? \"\" : \",\", spear_json_escape(scope, key), value);\n"
"        first = 0;\n"
"    }\n"
"    spear_sb_append(&buf, \"}\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_result_is_ok(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    return spear_text_clone(scope, spear_json_get_bool(text, \"ok\", 0) ? \"1\" : \"0\");\n"
"}\n"
"\n"
"static char *spear_native_result_value_json(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char value[8192];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_raw(text, \"value\", value, sizeof(value))) return spear_text_clone(scope, \"\");\n"
"    return spear_json_value_to_text(scope, value);\n"
"}\n"
"\n"
"static char *spear_native_result_error_json(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    char value[8192];\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) strcpy(text, \"{}\");\n"
"    if (!spear_json_get_raw(text, \"error\", value, sizeof(value))) return spear_text_clone(scope, \"\");\n"
"    return spear_json_value_to_text(scope, value);\n"
"}\n"
"\n"
"static char *spear_native_vector_add(SpearScope *scope, const char *payload) {\n"
"    char left[32768], right[32768], a[256], b[256];\n"
"    SpearStrBuf buf;\n"
"    int index = 0;\n"
"    int first = 1;\n"
"    if (!spear_json_get_raw(payload, \"left\", left, sizeof(left))) strcpy(left, \"[]\");\n"
"    if (!spear_json_get_raw(payload, \"right\", right, sizeof(right))) strcpy(right, \"[]\");\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"[\");\n"
"    while (spear_json_array_get(left, index, a, sizeof(a)) && spear_json_array_get(right, index, b, sizeof(b))) {\n"
"        char raw[128];\n"
"        snprintf(raw, sizeof(raw), \"%.12g\", atof(a) + atof(b));\n"
"        spear_sb_appendf(&buf, \"%s%s\", first ? \"\" : \",\", raw);\n"
"        first = 0;\n"
"        index++;\n"
"    }\n"
"    spear_sb_append(&buf, \"]\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_fs_safe_read(SpearScope *scope, const char *payload) {\n"
"    char root[MAX_PATH];\n"
"    char path[MAX_PATH];\n"
"    char resolved[MAX_PATH];\n"
"    int resolved_ok;\n"
"    if (!spear_json_get_string(payload, \"root\", root, sizeof(root))) strcpy(root, \".\");\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    resolved_ok = spear_resolve_under_root(resolved, sizeof(resolved), root, path);\n"
"    if (resolved_ok < 0) return spear_native_result_error(scope, \"path escapes root\");\n"
"    if (resolved_ok == 0) return spear_native_result_error(scope, \"cannot resolve path\");\n"
"    DWORD attr = GetFileAttributesA(resolved);\n"
"    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return spear_native_result_error(scope, \"file was not found\");\n"
"    return spear_native_result_text_path(scope, spear_read_text_file(scope, resolved), resolved);\n"
"}\n"
"\n"
"static char *spear_native_fs_safe_write(SpearScope *scope, const char *payload) {\n"
"    char root[MAX_PATH];\n"
"    char path[MAX_PATH];\n"
"    char content[8192];\n"
"    char resolved[MAX_PATH];\n"
"    int resolved_ok;\n"
"    if (!spear_json_get_string(payload, \"root\", root, sizeof(root))) strcpy(root, \".\");\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"content\", content, sizeof(content))) content[0] = '\\0';\n"
"    resolved_ok = spear_resolve_under_root(resolved, sizeof(resolved), root, path);\n"
"    if (resolved_ok < 0) return spear_native_result_error(scope, \"path escapes root\");\n"
"    if (resolved_ok == 0) return spear_native_result_error(scope, \"cannot resolve path\");\n"
"    char parent[MAX_PATH];\n"
"    snprintf(parent, sizeof(parent), \"%s\", resolved);\n"
"    char *slash = strrchr(parent, '\\\\');\n"
"    if (!slash) slash = strrchr(parent, '/');\n"
"    if (slash) {\n"
"        *slash = '\\0';\n"
"        spear_ensure_dir_recursive(parent);\n"
"    }\n"
"    if (!spear_write_text_try(resolved, content)) return spear_native_result_error(scope, \"cannot write file\");\n"
"    return spear_native_result_path(scope, resolved);\n"
"}\n"
"\n"
"static char *spear_native_fs_safe_mkdir(SpearScope *scope, const char *payload) {\n"
"    char root[MAX_PATH];\n"
"    char path[MAX_PATH];\n"
"    char resolved[MAX_PATH];\n"
"    int resolved_ok;\n"
"    if (!spear_json_get_string(payload, \"root\", root, sizeof(root))) strcpy(root, \".\");\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    resolved_ok = spear_resolve_under_root(resolved, sizeof(resolved), root, path);\n"
"    if (resolved_ok < 0) return spear_native_result_error(scope, \"path escapes root\");\n"
"    if (resolved_ok == 0) return spear_native_result_error(scope, \"cannot resolve path\");\n"
"    spear_ensure_dir_recursive(resolved);\n"
"    return spear_native_result_path(scope, resolved);\n"
"}\n"
"\n"
"static char *spear_native_fs_safe_remove(SpearScope *scope, const char *payload) {\n"
"    char root[MAX_PATH];\n"
"    char path[MAX_PATH];\n"
"    char resolved[MAX_PATH];\n"
"    int resolved_ok;\n"
"    if (!spear_json_get_string(payload, \"root\", root, sizeof(root))) strcpy(root, \".\");\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    resolved_ok = spear_resolve_under_root(resolved, sizeof(resolved), root, path);\n"
"    if (resolved_ok < 0) return spear_native_result_error(scope, \"path escapes root\");\n"
"    if (resolved_ok == 0) return spear_native_result_error(scope, \"cannot resolve path\");\n"
"    DWORD attr = GetFileAttributesA(resolved);\n"
"    if (attr == INVALID_FILE_ATTRIBUTES) return spear_native_result_error(scope, \"path was not found\");\n"
"    if (attr & FILE_ATTRIBUTE_DIRECTORY) {\n"
"        WIN32_FIND_DATAA data;\n"
"        char pattern[MAX_PATH];\n"
"        snprintf(pattern, sizeof(pattern), \"%s\\\\*\", resolved);\n"
"        HANDLE handle = FindFirstFileA(pattern, &data);\n"
"        if (handle != INVALID_HANDLE_VALUE) {\n"
"            do {\n"
"                if (strcmp(data.cFileName, \".\") != 0 && strcmp(data.cFileName, \"..\") != 0) {\n"
"                    FindClose(handle);\n"
"                    return spear_native_result_error(scope, \"refusing to remove non-empty directory\");\n"
"                }\n"
"            } while (FindNextFileA(handle, &data));\n"
"            FindClose(handle);\n"
"        }\n"
"        if (!RemoveDirectoryA(resolved)) return spear_native_result_error(scope, \"path was not found\");\n"
"        return spear_native_result_path(scope, resolved);\n"
"    }\n"
"    if (!DeleteFileA(resolved)) return spear_native_result_error(scope, \"path was not found\");\n"
"    return spear_native_result_path(scope, resolved);\n"
"}\n"
"\n"
"static char *spear_native_fs_safe_list(SpearScope *scope, const char *payload) {\n"
"    char root[MAX_PATH];\n"
"    char path[MAX_PATH];\n"
"    char resolved[MAX_PATH];\n"
"    int resolved_ok;\n"
"    if (!spear_json_get_string(payload, \"root\", root, sizeof(root))) strcpy(root, \".\");\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    resolved_ok = spear_resolve_under_root(resolved, sizeof(resolved), root, path);\n"
"    if (resolved_ok < 0) return spear_native_result_error(scope, \"path escapes root\");\n"
"    if (resolved_ok == 0) return spear_native_result_error(scope, \"cannot resolve path\");\n"
"    DWORD attr = GetFileAttributesA(resolved);\n"
"    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) return spear_native_result_error(scope, \"directory was not found\");\n"
"    char pattern[MAX_PATH];\n"
"    snprintf(pattern, sizeof(pattern), \"%s\\\\*\", resolved);\n"
"    WIN32_FIND_DATAA data;\n"
"    HANDLE handle = FindFirstFileA(pattern, &data);\n"
"    size_t cap = 256;\n"
"    size_t len = 0;\n"
"    char *items = (char *) spear_alloc(scope, cap);\n"
"    items[0] = '\\0';\n"
"    if (handle != INVALID_HANDLE_VALUE) {\n"
"        int first = 1;\n"
"        do {\n"
"            if (strcmp(data.cFileName, \".\") == 0 || strcmp(data.cFileName, \"..\") == 0) continue;\n"
"            char *escaped = spear_json_escape(scope, data.cFileName);\n"
"            size_t need = len + strlen(escaped) + 4;\n"
"            if (need >= cap) {\n"
"                cap *= 2;\n"
"                if (need >= cap) cap = need + 32;\n"
"                char *grown = (char *) spear_alloc(scope, cap);\n"
"                memcpy(grown, items, len + 1);\n"
"                items = grown;\n"
"            }\n"
"            len += (size_t) snprintf(items + len, cap - len, \"%s\\\"%s\\\"\", first ? \"\" : \",\", escaped);\n"
"            first = 0;\n"
"        } while (FindNextFileA(handle, &data));\n"
"        FindClose(handle);\n"
"    }\n"
"    char *escaped_path = spear_json_escape(scope, resolved);\n"
"    size_t total = len + strlen(escaped_path) + 48;\n"
"    char *dst = (char *) spear_alloc(scope, total);\n"
"    snprintf(dst, total, \"{\\\"ok\\\":true,\\\"items\\\":[%s],\\\"path\\\":\\\"%s\\\"}\", items, escaped_path);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_fs_read(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    DWORD attr;\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    attr = GetFileAttributesA(path);\n"
"    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) {\n"
"        spear_runtime_fail_path(\"cannot read\", path[0] ? path : \"\");\n"
"    }\n"
"    return spear_read_text_file(scope, path);\n"
"}\n"
"\n"
"static char *spear_native_fs_write(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    char content[8192];\n"
"    char parent[MAX_PATH];\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"content\", content, sizeof(content))) content[0] = '\\0';\n"
"    snprintf(parent, sizeof(parent), \"%s\", path);\n"
"    char *slash = strrchr(parent, '\\\\');\n"
"    if (!slash) slash = strrchr(parent, '/');\n"
"    if (slash) {\n"
"        *slash = '\\0';\n"
"        if (parent[0]) spear_ensure_dir_recursive(parent);\n"
"    }\n"
"    if (!spear_write_text_try(path, content)) {\n"
"        spear_runtime_fail_path(\"cannot write\", path[0] ? path : \"\");\n"
"    }\n"
"    return spear_text_clone(scope, path);\n"
"}\n"
"\n"
"static char *spear_native_fs_exists(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    return spear_text_clone(scope, GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES ? \"0\" : \"1\");\n"
"}\n"
"\n"
"static char *spear_native_fs_list(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    char pattern[MAX_PATH];\n"
"    WIN32_FIND_DATAA data;\n"
"    HANDLE handle;\n"
"    size_t cap = 256;\n"
"    size_t len = 0;\n"
"    char *items = (char *) spear_alloc(scope, cap);\n"
"    items[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {\n"
"        return spear_text_clone(scope, \"[]\");\n"
"    }\n"
"    snprintf(pattern, sizeof(pattern), \"%s\\\\*\", path);\n"
"    handle = FindFirstFileA(pattern, &data);\n"
"    if (handle != INVALID_HANDLE_VALUE) {\n"
"        int first = 1;\n"
"        do {\n"
"            if (strcmp(data.cFileName, \".\") == 0 || strcmp(data.cFileName, \"..\") == 0) continue;\n"
"            char *escaped = spear_json_escape(scope, data.cFileName);\n"
"            size_t need = len + strlen(escaped) + 4;\n"
"            if (need >= cap) {\n"
"                cap *= 2;\n"
"                if (need >= cap) cap = need + 32;\n"
"                char *grown = (char *) spear_alloc(scope, cap);\n"
"                memcpy(grown, items, len + 1);\n"
"                items = grown;\n"
"            }\n"
"            len += (size_t) snprintf(items + len, cap - len, \"%s\\\"%s\\\"\", first ? \"\" : \",\", escaped);\n"
"            first = 0;\n"
"        } while (FindNextFileA(handle, &data));\n"
"        FindClose(handle);\n"
"    }\n"
"    size_t total = len + 3;\n"
"    char *dst = (char *) spear_alloc(scope, total);\n"
"    snprintf(dst, total, \"[%s]\", items);\n"
"    return dst;\n"
"}\n"
"\n"
"static char *spear_native_fs_mkdir(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    if (path[0]) spear_ensure_dir_recursive(path);\n"
"    return spear_text_clone(scope, path);\n"
"}\n"
"\n"
"static char *spear_native_fs_remove(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    DWORD attr;\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    attr = GetFileAttributesA(path);\n"
"    if (attr == INVALID_FILE_ATTRIBUTES) {\n"
"        return spear_text_clone(scope, path);\n"
"    }\n"
"    if (attr & FILE_ATTRIBUTE_DIRECTORY) {\n"
"        WIN32_FIND_DATAA data;\n"
"        char pattern[MAX_PATH];\n"
"        snprintf(pattern, sizeof(pattern), \"%s\\\\*\", path);\n"
"        HANDLE handle = FindFirstFileA(pattern, &data);\n"
"        if (handle != INVALID_HANDLE_VALUE) {\n"
"            do {\n"
"                if (strcmp(data.cFileName, \".\") == 0 || strcmp(data.cFileName, \"..\") == 0) continue;\n"
"                char child[MAX_PATH];\n"
"                snprintf(child, sizeof(child), \"%s\\\\%s\", path, data.cFileName);\n"
"                DWORD child_attr = GetFileAttributesA(child);\n"
"                if (child_attr != INVALID_FILE_ATTRIBUTES && !(child_attr & FILE_ATTRIBUTE_DIRECTORY)) {\n"
"                    DeleteFileA(child);\n"
"                }\n"
"            } while (FindNextFileA(handle, &data));\n"
"            FindClose(handle);\n"
"        }\n"
"        RemoveDirectoryA(path);\n"
"        return spear_text_clone(scope, path);\n"
"    }\n"
"    DeleteFileA(path);\n"
"    return spear_text_clone(scope, path);\n"
"}\n"
"\n"
"static void spear_temp_file(char *path, size_t cap, const char *prefix);\n"
"\n"
"static unsigned int spear_rand32(void) {\n"
"    static int seeded = 0;\n"
"    if (!seeded) {\n"
"        srand((unsigned int) (GetTickCount() ^ GetCurrentProcessId()));\n"
"        seeded = 1;\n"
"    }\n"
"    return ((unsigned int) rand() << 16) ^ (unsigned int) rand();\n"
"}\n"
"\n"
"static char *spear_native_env_all(SpearScope *scope, const char *payload) {\n"
"    LPCH block = GetEnvironmentStringsA();\n"
"    SpearStrBuf buf;\n"
"    (void) payload;\n"
"    if (!block) return spear_text_clone(scope, \"{}\");\n"
"    spear_sb_init(&buf);\n"
"    spear_sb_append(&buf, \"{\");\n"
"    int first = 1;\n"
"    for (LPCH p = block; *p; p += strlen(p) + 1) {\n"
"        const char *eq = strchr(p, '=');\n"
"        if (!eq || p[0] == '=') continue;\n"
"        size_t key_len = (size_t) (eq - p);\n"
"        char key[512];\n"
"        if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;\n"
"        memcpy(key, p, key_len);\n"
"        key[key_len] = '\\0';\n"
"        spear_sb_appendf(&buf, \"%s\\\"%s\\\":\\\"%s\\\"\", first ? \"\" : \",\", spear_json_escape(scope, key), spear_json_escape(scope, eq + 1));\n"
"        first = 0;\n"
"    }\n"
"    FreeEnvironmentStringsA(block);\n"
"    spear_sb_append(&buf, \"}\");\n"
"    return spear_sb_take(scope, &buf);\n"
"}\n"
"\n"
"static char *spear_native_logger_line(SpearScope *scope, const char *payload) {\n"
"    SYSTEMTIME st;\n"
"    char level[64];\n"
"    char message[4096];\n"
"    char line[8192];\n"
"    if (!spear_json_get_string(payload, \"level\", level, sizeof(level))) strcpy(level, \"INFO\");\n"
"    if (!spear_json_get_string(payload, \"message\", message, sizeof(message))) message[0] = '\\0';\n"
"    CharUpperBuffA(level, (DWORD) strlen(level));\n"
"    GetLocalTime(&st);\n"
"    snprintf(line, sizeof(line), \"[%04d-%02d-%02d %02d:%02d:%02d] %s %s\", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, level, message);\n"
"    return spear_text_clone(scope, line);\n"
"}\n"
"\n"
"static char *spear_native_random_int(SpearScope *scope, const char *payload) {\n"
"    unsigned int r = 0;\n"
"    char start_raw[64], end_raw[64], out[64];\n"
"    long long start = 0;\n"
"    long long end = 100;\n"
"    if (spear_json_get_raw(payload, \"start\", start_raw, sizeof(start_raw))) start = _strtoi64(start_raw, NULL, 10);\n"
"    if (spear_json_get_raw(payload, \"end\", end_raw, sizeof(end_raw))) end = _strtoi64(end_raw, NULL, 10);\n"
"    if (end < start) { long long tmp = start; start = end; end = tmp; }\n"
"    r = spear_rand32();\n"
"    unsigned long long span = (unsigned long long) (end - start + 1);\n"
"    unsigned long long value = start + (span ? ((unsigned long long) r % span) : 0ULL);\n"
"    snprintf(out, sizeof(out), \"%llu\", value);\n"
"    return spear_text_clone(scope, out);\n"
"}\n"
"\n"
"static char *spear_native_random_float(SpearScope *scope, const char *payload) {\n"
"    unsigned int r = 0;\n"
"    char start_raw[64], end_raw[64], out[128];\n"
"    double start = 0.0;\n"
"    double end = 1.0;\n"
"    if (spear_json_get_raw(payload, \"start\", start_raw, sizeof(start_raw))) start = atof(start_raw);\n"
"    if (spear_json_get_raw(payload, \"end\", end_raw, sizeof(end_raw))) end = atof(end_raw);\n"
"    r = spear_rand32();\n"
"    double unit = (double) r / (double) UINT_MAX;\n"
"    double value = start + (end - start) * unit;\n"
"    snprintf(out, sizeof(out), \"%.12g\", value);\n"
"    return spear_text_clone(scope, out);\n"
"}\n"
"\n"
"static char *spear_native_csv_parse(SpearScope *scope, const char *payload) {\n"
"    char text[32768];\n"
"    SpearStrBuf out;\n"
"    SpearStrBuf field;\n"
"    int in_quotes = 0;\n"
"    int row_open = 0;\n"
"    int first_row = 1;\n"
"    int first_col = 1;\n"
"    if (!spear_json_get_string(payload, \"text\", text, sizeof(text))) text[0] = '\\0';\n"
"    spear_sb_init(&out);\n"
"    spear_sb_init(&field);\n"
"    spear_sb_append(&out, \"[\");\n"
"    for (size_t i = 0;; i++) {\n"
"        char ch = text[i];\n"
"        char next = text[i + 1];\n"
"        if (!row_open) {\n"
"            spear_sb_append(&out, first_row ? \"[\" : \",[\" );\n"
"            row_open = 1;\n"
"            first_col = 1;\n"
"            first_row = 0;\n"
"        }\n"
"        if (ch == '\\0' || ((!in_quotes) && (ch == ',' || ch == '\\n' || ch == '\\r'))) {\n"
"            spear_sb_appendf(&out, \"%s\\\"%s\\\"\", first_col ? \"\" : \",\", spear_json_escape(scope, field.data));\n"
"            field.len = 0;\n"
"            field.data[0] = '\\0';\n"
"            first_col = 0;\n"
"            if (ch == ',') continue;\n"
"            spear_sb_append(&out, \"]\");\n"
"            row_open = 0;\n"
"            if (ch == '\\r' && next == '\\n') i++;\n"
"            if (ch == '\\0') break;\n"
"            continue;\n"
"        }\n"
"        if (ch == '\"') {\n"
"            if (in_quotes && next == '\"') {\n"
"                spear_sb_append(&field, \"\\\"\");\n"
"                i++;\n"
"            } else {\n"
"                in_quotes = !in_quotes;\n"
"            }\n"
"            continue;\n"
"        }\n"
"        char tmp[2];\n"
"        tmp[0] = ch;\n"
"        tmp[1] = '\\0';\n"
"        spear_sb_append(&field, tmp);\n"
"    }\n"
"    if (row_open) spear_sb_append(&out, \"]\");\n"
"    spear_sb_append(&out, \"]\");\n"
"    free(field.data);\n"
"    return spear_sb_take(scope, &out);\n"
"}\n"
"\n"
"static char *spear_native_csv_stringify(SpearScope *scope, const char *payload) {\n"
"    char rows[32768];\n"
"    char row[8192];\n"
"    char cell[4096];\n"
"    SpearStrBuf out;\n"
"    int row_index = 0;\n"
"    if (!spear_json_get_raw(payload, \"rows\", rows, sizeof(rows))) strcpy(rows, \"[]\");\n"
"    spear_sb_init(&out);\n"
"    while (spear_json_array_get(rows, row_index++, row, sizeof(row))) {\n"
"        int cell_index = 0;\n"
"        while (spear_json_array_get(row, cell_index++, cell, sizeof(cell))) {\n"
"            char raw[4096];\n"
"            if (cell[0] == '\"') {\n"
"                size_t len = strlen(cell);\n"
"                if (len >= 2) {\n"
"                    memcpy(raw, cell + 1, len - 2);\n"
"                    raw[len - 2] = '\\0';\n"
"                } else raw[0] = '\\0';\n"
"            } else {\n"
"                snprintf(raw, sizeof(raw), \"%s\", cell);\n"
"            }\n"
"            int needs_quotes = strchr(raw, ',') || strchr(raw, '\"') || strchr(raw, '\\n') || strchr(raw, '\\r');\n"
"            if (cell_index > 1) spear_sb_append(&out, \",\");\n"
"            if (needs_quotes) spear_sb_append(&out, \"\\\"\");\n"
"            for (size_t i = 0; raw[i]; i++) {\n"
"                if (raw[i] == '\"') spear_sb_append(&out, \"\\\"\\\"\");\n"
"                else { char tmp[2] = { raw[i], '\\0' }; spear_sb_append(&out, tmp); }\n"
"            }\n"
"            if (needs_quotes) spear_sb_append(&out, \"\\\"\");\n"
"        }\n"
"        spear_sb_append(&out, \"\\r\\n\");\n"
"    }\n"
"    return spear_sb_take(scope, &out);\n"
"}\n"
"\n"
"static char *spear_native_process_run(SpearScope *scope, const char *payload) {\n"
"    char command[4096];\n"
"    char cwd[MAX_PATH];\n"
"    char script_path[MAX_PATH];\n"
"    char out_path[MAX_PATH];\n"
"    char err_path[MAX_PATH];\n"
"    char launch[8192];\n"
"    char *stdout_text;\n"
"    char *stderr_text;\n"
"    char *stdout_json;\n"
"    char *stderr_json;\n"
"    int exit_code;\n"
"    FILE *fp;\n"
"    if (!spear_json_get_string(payload, \"command\", command, sizeof(command))) command[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"cwd\", cwd, sizeof(cwd))) cwd[0] = '\\0';\n"
"    spear_temp_file(script_path, sizeof(script_path), \"spr\");\n"
"    spear_temp_file(out_path, sizeof(out_path), \"spr\");\n"
"    spear_temp_file(err_path, sizeof(err_path), \"spr\");\n"
"    DeleteFileA(script_path);\n"
"    fp = fopen(script_path, \"wb\");\n"
"    if (!fp) spear_runtime_fail(\"cannot create temp script\");\n"
"    fprintf(fp, \"@echo off\\r\\n\");\n"
"    if (cwd[0]) fprintf(fp, \"cd /d \\\"%s\\\"\\r\\n\", cwd);\n"
"    fprintf(fp, \"%s 1>\\\"%s\\\" 2>\\\"%s\\\"\\r\\n\", command, out_path, err_path);\n"
"    fprintf(fp, \"exit /b %%errorlevel%%\\r\\n\");\n"
"    fclose(fp);\n"
"    snprintf(launch, sizeof(launch), \"cmd /d /c \\\"%s\\\"\", script_path);\n"
"    exit_code = system(launch);\n"
"    stdout_text = spear_read_text_file(scope, out_path);\n"
"    stderr_text = spear_read_text_file(scope, err_path);\n"
"    stdout_json = spear_json_escape(scope, stdout_text);\n"
"    stderr_json = spear_json_escape(scope, stderr_text);\n"
"    DeleteFileA(script_path);\n"
"    DeleteFileA(out_path);\n"
"    DeleteFileA(err_path);\n"
"    SpearStrBuf out;\n"
"    spear_sb_init(&out);\n"
"    spear_sb_appendf(&out, \"{\\\"code\\\":%d,\\\"stdout\\\":\\\"%s\\\",\\\"stderr\\\":\\\"%s\\\"}\", exit_code, stdout_json, stderr_json);\n"
"    return spear_sb_take(scope, &out);\n"
"}\n"
"\n"
"static int spear_find_sqlite_cli(char *out, size_t cap) {\n"
"    char candidate[MAX_PATH];\n"
"    snprintf(candidate, sizeof(candidate), \"%s\\\\runtime\\\\sqlite3.exe\", SPEAR_TOOL_DIR);\n"
"    if (GetFileAttributesA(candidate) != INVALID_FILE_ATTRIBUTES) {\n"
"        snprintf(out, cap, \"%s\", candidate);\n"
"        return 1;\n"
"    }\n"
"    if (SearchPathA(NULL, \"sqlite3.exe\", NULL, (DWORD) cap, out, NULL) && out[0]) return 1;\n"
"    return 0;\n"
"}\n"
"\n"
"static int spear_sqlite_run(const char *sqlite_path, const char *db_path, const char *sql_script, char *out_path, size_t out_cap, char *err_path, size_t err_cap) {\n"
"    char sql_path[MAX_PATH];\n"
"    char command[8192];\n"
"    int exit_code;\n"
"    spear_temp_file(sql_path, sizeof(sql_path), \"sql\");\n"
"    spear_temp_file(out_path, out_cap, \"sout\");\n"
"    spear_temp_file(err_path, err_cap, \"serr\");\n"
"    if (!spear_write_text_try(sql_path, sql_script)) {\n"
"        DeleteFileA(sql_path);\n"
"        return -1;\n"
"    }\n"
"    snprintf(command, sizeof(command), \"cmd /c \\\"\\\"%s\\\" -batch \\\"%s\\\" < \\\"%s\\\" > \\\"%s\\\" 2> \\\"%s\\\"\\\"\", sqlite_path, db_path, sql_path, out_path, err_path);\n"
"    exit_code = system(command);\n"
"    DeleteFileA(sql_path);\n"
"    return exit_code;\n"
"}\n"
"\n"
"static char *spear_native_sqlite_execute(SpearScope *scope, const char *payload) {\n"
"    char sqlite_path[MAX_PATH], db_path[MAX_PATH], sql[32768], script[65536], out_path[MAX_PATH], err_path[MAX_PATH];\n"
"    char *stdout_text;\n"
"    char *stderr_text;\n"
"    char first_row[4096];\n"
"    if (!spear_json_get_string(payload, \"path\", db_path, sizeof(db_path))) db_path[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"sql\", sql, sizeof(sql))) sql[0] = '\\0';\n"
"    if (!spear_find_sqlite_cli(sqlite_path, sizeof(sqlite_path))) spear_runtime_fail(\"sqlite runtime unavailable\");\n"
"    snprintf(script, sizeof(script), \".mode json\\n%s;\\nselect changes() as rows_affected;\\n\", sql);\n"
"    if (spear_sqlite_run(sqlite_path, db_path, script, out_path, sizeof(out_path), err_path, sizeof(err_path)) != 0) {\n"
"        stderr_text = spear_read_text_file(scope, err_path);\n"
"        DeleteFileA(out_path);\n"
"        DeleteFileA(err_path);\n"
"        spear_runtime_fail_at(1, 1, stderr_text[0] ? stderr_text : \"sqlite execute failed\");\n"
"    }\n"
"    stdout_text = spear_read_text_file(scope, out_path);\n"
"    DeleteFileA(out_path);\n"
"    DeleteFileA(err_path);\n"
"    if (spear_json_array_get(stdout_text, 0, first_row, sizeof(first_row))) return spear_text_clone(scope, first_row);\n"
"    return spear_text_clone(scope, \"{\\\"rows_affected\\\":0}\");\n"
"}\n"
"\n"
"static char *spear_native_sqlite_query(SpearScope *scope, const char *payload) {\n"
"    char sqlite_path[MAX_PATH], db_path[MAX_PATH], sql[32768], script[65536], out_path[MAX_PATH], err_path[MAX_PATH];\n"
"    char *stdout_text;\n"
"    if (!spear_json_get_string(payload, \"path\", db_path, sizeof(db_path))) db_path[0] = '\\0';\n"
"    if (!spear_json_get_string(payload, \"sql\", sql, sizeof(sql))) sql[0] = '\\0';\n"
"    if (!spear_find_sqlite_cli(sqlite_path, sizeof(sqlite_path))) spear_runtime_fail(\"sqlite runtime unavailable\");\n"
"    snprintf(script, sizeof(script), \".mode json\\n%s;\\n\", sql);\n"
"    if (spear_sqlite_run(sqlite_path, db_path, script, out_path, sizeof(out_path), err_path, sizeof(err_path)) != 0) {\n"
"        char *stderr_text = spear_read_text_file(scope, err_path);\n"
"        DeleteFileA(out_path);\n"
"        DeleteFileA(err_path);\n"
"        spear_runtime_fail_at(1, 1, stderr_text[0] ? stderr_text : \"sqlite query failed\");\n"
"    }\n"
"    stdout_text = spear_read_text_file(scope, out_path);\n"
"    DeleteFileA(out_path);\n"
"    DeleteFileA(err_path);\n"
"    if (!stdout_text[0]) return spear_text_clone(scope, \"[]\");\n"
"    return spear_text_clone(scope, stdout_text);\n"
"}\n"
"\n"
"static char *spear_native_config_load(SpearScope *scope, const char *payload) {\n"
"    char path[MAX_PATH];\n"
"    char *text;\n"
"    const char *ext;\n"
"    if (!spear_json_get_string(payload, \"path\", path, sizeof(path))) path[0] = '\\0';\n"
"    text = spear_read_text_file(scope, path);\n"
"    ext = strrchr(path, '.');\n"
"    if (ext && (_stricmp(ext, \".json\") == 0)) return spear_text_clone(scope, text);\n"
"    if (ext && (_stricmp(ext, \".csv\") == 0)) {\n"
"        char csv_payload[33024];\n"
"        snprintf(csv_payload, sizeof(csv_payload), \"{\\\"text\\\":\\\"%s\\\"}\", spear_json_escape(scope, text));\n"
"        return spear_native_csv_parse(scope, csv_payload);\n"
"    }\n"
"    if (ext && (_stricmp(ext, \".toml\") == 0)) {\n"
"        SpearStrBuf out;\n"
"        spear_sb_init(&out);\n"
"        spear_sb_appendf(&out, \"{\\\"text\\\":\\\"%s\\\"}\", spear_json_escape(scope, text));\n"
"        return spear_sb_take(scope, &out);\n"
"    }\n"
"    SpearStrBuf out;\n"
"    spear_sb_init(&out);\n"
"    spear_sb_appendf(&out, \"{\\\"text\\\":\\\"%s\\\"}\", spear_json_escape(scope, text));\n"
"    return spear_sb_take(scope, &out);\n"
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
"    char message[512];\n"
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
"    if (!spear_command_exists(runner)) {\n"
"        snprintf(message, sizeof(message), \"%s runtime is not installed. This program requested the optional %s bridge via %s('%s', '%s', ...).\", runner, runner, strcmp(runner, \"node\") == 0 ? \"nodecall\" : \"pycall\", target, fn);\n"
"        DeleteFileA(req_path);\n"
"        DeleteFileA(res_path);\n"
"        spear_runtime_fail_at(1, 1, message);\n"
"    }\n"
"    if (GetFileAttributesA(script_path) == INVALID_FILE_ATTRIBUTES) {\n"
"        snprintf(message, sizeof(message), \"missing optional bridge script: %s\", script_leaf);\n"
"        DeleteFileA(req_path);\n"
"        DeleteFileA(res_path);\n"
"        spear_runtime_fail_at(1, 1, message);\n"
"    }\n"
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
"    if (strcmp(target, \"spear_std\") == 0) {\n"
"        if (strcmp(fn, \"json_format\") == 0) return spear_native_json_format(scope, payload);\n"
"        if (strcmp(fn, \"json_quote\") == 0) return spear_native_json_quote(scope, payload);\n"
"        if (strcmp(fn, \"json_get\") == 0) return spear_native_json_get(scope, payload);\n"
"        if (strcmp(fn, \"json_keys\") == 0) return spear_native_json_keys(scope, payload);\n"
"        if (strcmp(fn, \"http_request\") == 0) return spear_native_http_request(scope, payload);\n"
"        if (strcmp(fn, \"env_get\") == 0) return spear_native_env_get(scope, payload);\n"
"        if (strcmp(fn, \"sha256_hex\") == 0) return spear_native_sha256_hex(scope, payload);\n"
"        if (strcmp(fn, \"hmac_sha256\") == 0) return spear_native_hmac_sha256(scope, payload);\n"
"        if (strcmp(fn, \"secure_token\") == 0) return spear_native_secure_token(scope, payload);\n"
"        if (strcmp(fn, \"secure_compare\") == 0) return spear_native_secure_compare(scope, payload);\n"
"        if (strcmp(fn, \"pbkdf2_sha256\") == 0) return spear_native_pbkdf2_sha256(scope, payload);\n"
"        if (strcmp(fn, \"pbkdf2_verify\") == 0) return spear_native_pbkdf2_verify(scope, payload);\n"
"        if (strcmp(fn, \"jwt_hs256\") == 0) return spear_native_jwt_sign_hs256(scope, payload);\n"
"        if (strcmp(fn, \"jwt_decode\") == 0) return spear_native_jwt_decode(scope, payload);\n"
"        if (strcmp(fn, \"jwt_verify_hs256\") == 0) return spear_native_jwt_verify_hs256(scope, payload);\n"
"        if (strcmp(fn, \"fs_read\") == 0) return spear_native_fs_read(scope, payload);\n"
"        if (strcmp(fn, \"fs_write\") == 0) return spear_native_fs_write(scope, payload);\n"
"        if (strcmp(fn, \"fs_exists\") == 0) return spear_native_fs_exists(scope, payload);\n"
"        if (strcmp(fn, \"fs_list\") == 0) return spear_native_fs_list(scope, payload);\n"
"        if (strcmp(fn, \"fs_mkdir\") == 0) return spear_native_fs_mkdir(scope, payload);\n"
"        if (strcmp(fn, \"fs_remove\") == 0) return spear_native_fs_remove(scope, payload);\n"
"        if (strcmp(fn, \"env_all\") == 0) return spear_native_env_all(scope, payload);\n"
"        if (strcmp(fn, \"process_run\") == 0) return spear_native_process_run(scope, payload);\n"
"        if (strcmp(fn, \"random_int\") == 0) return spear_native_random_int(scope, payload);\n"
"        if (strcmp(fn, \"random_float\") == 0) return spear_native_random_float(scope, payload);\n"
"        if (strcmp(fn, \"csv_parse\") == 0) return spear_native_csv_parse(scope, payload);\n"
"        if (strcmp(fn, \"csv_stringify\") == 0) return spear_native_csv_stringify(scope, payload);\n"
"        if (strcmp(fn, \"tcp_request\") == 0) return spear_native_tcp_request(scope, payload);\n"
"        if (strcmp(fn, \"udp_send\") == 0) return spear_native_udp_send(scope, payload);\n"
"        if (strcmp(fn, \"http_serve_text\") == 0) return spear_native_http_serve_text(scope, payload);\n"
"        if (strcmp(fn, \"http_serve_directory\") == 0) return spear_native_http_serve_directory(scope, payload);\n"
"        if (strcmp(fn, \"map_get\") == 0) return spear_native_map_get(scope, payload);\n"
"        if (strcmp(fn, \"map_has\") == 0) return spear_native_map_has(scope, payload);\n"
"        if (strcmp(fn, \"map_set\") == 0) return spear_native_map_set(scope, payload);\n"
"        if (strcmp(fn, \"map_remove\") == 0) return spear_native_map_remove(scope, payload);\n"
"        if (strcmp(fn, \"result_is_ok\") == 0) return spear_native_result_is_ok(scope, payload);\n"
"        if (strcmp(fn, \"result_value\") == 0) return spear_native_result_value_json(scope, payload);\n"
"        if (strcmp(fn, \"result_error\") == 0) return spear_native_result_error_json(scope, payload);\n"
"        if (strcmp(fn, \"vector_add\") == 0) return spear_native_vector_add(scope, payload);\n"
"        if (strcmp(fn, \"logger_line\") == 0) return spear_native_logger_line(scope, payload);\n"
"        if (strcmp(fn, \"config_load\") == 0) return spear_native_config_load(scope, payload);\n"
 "        if (strcmp(fn, \"sqlite_execute\") == 0) return spear_native_sqlite_execute(scope, payload);\n"
 "        if (strcmp(fn, \"sqlite_query\") == 0) return spear_native_sqlite_query(scope, payload);\n"
"        if (strcmp(fn, \"fs_safe_read\") == 0) return spear_native_fs_safe_read(scope, payload);\n"
"        if (strcmp(fn, \"fs_safe_write\") == 0) return spear_native_fs_safe_write(scope, payload);\n"
"        if (strcmp(fn, \"fs_safe_list\") == 0) return spear_native_fs_safe_list(scope, payload);\n"
"        if (strcmp(fn, \"fs_safe_mkdir\") == 0) return spear_native_fs_safe_mkdir(scope, payload);\n"
"        if (strcmp(fn, \"fs_safe_remove\") == 0) return spear_native_fs_safe_remove(scope, payload);\n"
"        if (strcmp(fn, \"ui_web_render\") == 0) return spear_native_ui_web_render(scope, payload);\n"
"        if (strcmp(fn, \"ui_android_render\") == 0) return spear_native_ui_android_render(scope, payload);\n"
"        if (strcmp(fn, \"ui_desktop_render\") == 0) return spear_native_ui_desktop_render(scope, payload);\n"
"        if (strcmp(fn, \"safe_http_request\") == 0) {\n"
"            char *native_http = spear_native_safe_http_request(scope, payload);\n"
"            if (native_http) return native_http;\n"
"        }\n"
"    }\n"
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
