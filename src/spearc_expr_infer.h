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
