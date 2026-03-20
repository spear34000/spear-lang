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
