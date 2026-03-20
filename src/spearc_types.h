#ifndef SPEARC_TYPES_H
#define SPEARC_TYPES_H

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

static Expr make_expr(ValueType type, char *code) {
    Expr expr;
    expr.type = type;
    expr.code = code;
    return expr;
}

#endif
