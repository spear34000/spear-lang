static bool parse_tail_statement(Parser *parser, int scope_id, bool *handled) {
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
        *handled = true;
        return false;
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
        Expr value = parse_value_expr(parser, scope_id, type);
        expect(parser, TOK_SEMI, "expected ';'");
        emit_line(parser, "%s = %s;", name, value.code);
        *handled = true;
        return false;
    }

    *handled = false;
    return false;
}
