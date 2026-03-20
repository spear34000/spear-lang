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
