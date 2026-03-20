#ifndef SPEARC_STATEMENT_CONTROL_H
#define SPEARC_STATEMENT_CONTROL_H

static bool parse_control_statement(Parser *parser, int scope_id, bool *handled) {
    *handled = true;

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
                Expr value = parse_value_expr(parser, scope_id, type);
                add_symbol(parser, name, type, false, name_tok.line, name_tok.col, false, false);
                buf_appendf(&init, "%s %s = %s", ctype_name(type), name, value.code);
            } else if (is_type_token(parser->lexer.current.kind)) {
                ValueType type = token_to_type(parser->lexer.current.kind);
                advance(parser);
                Token name_tok = parser->lexer.current;
                expect(parser, TOK_IDENT, "expected variable name");
                char *name = token_text(name_tok);
                expect(parser, TOK_ASSIGN, "expected '='");
                Expr value = parse_value_expr(parser, scope_id, type);
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
                Expr value = parse_value_expr(parser, scope_id, type);
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
                Expr value = parse_value_expr(parser, scope_id, type);
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

    *handled = false;
    return false;
}

#endif
