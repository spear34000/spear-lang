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
