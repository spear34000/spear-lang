#ifndef SPEARC_STATEMENT_CORE_H
#define SPEARC_STATEMENT_CORE_H

static bool parse_core_statement(Parser *parser, int scope_id, bool *handled) {
    *handled = true;

    if (match(parser, TOK_MODULE) || match(parser, TOK_PACKAGE)) {
        expect(parser, TOK_IDENT, "expected name after metadata");
        expect(parser, TOK_SEMI, "expected ';' after metadata");
        return false;
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
            Expr value = parse_value_expr(parser, scope_id, type);
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
        Expr value = parse_value_expr(parser, scope_id, type);
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
        Expr value = parse_value_expr(parser, scope_id, type);
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
        Expr value = parse_value_expr(parser, scope_id, type);
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

    *handled = false;
    return false;
}

#endif
