static const char *sharp_default_code(ValueType type) {
    switch (type) {
        case TYPE_NUM: return "0";
        case TYPE_TEXT: return "NULL";
        case TYPE_NUMLIST: return "NULL";
        case TYPE_TEXTLIST: return "NULL";
        case TYPE_MAP: return "NULL";
        case TYPE_RESULT: return "NULL";
    }
    return "NULL";
}

static char *sharp_keep_code(ValueType type, int scope_id, Expr value) {
    Buffer code;
    char *scope_name = make_scope_name(scope_id);
    buf_init(&code);
    if (type == TYPE_NUM) {
        buf_appendf(&code, "_sharp_value = %s;", value.code);
    } else if (type == TYPE_TEXT) {
        buf_appendf(&code, "_sharp_value = spear_text_clone(&%s, %s);", scope_name, value.code);
    } else if (type == TYPE_NUMLIST) {
        buf_appendf(&code, "_sharp_value = spear_numlist_clone(&%s, %s);", scope_name, value.code);
    } else if (type == TYPE_TEXTLIST) {
        buf_appendf(&code, "_sharp_value = spear_textlist_clone(&%s, %s);", scope_name, value.code);
    } else if (type == TYPE_MAP) {
        buf_appendf(&code, "_sharp_value = spear_map_clone(&%s, %s);", scope_name, value.code);
    } else if (type == TYPE_RESULT) {
        buf_appendf(&code, "_sharp_value = spear_result_clone(&%s, %s);", scope_name, value.code);
    } else {
        buf_appendf(&code, "_sharp_value = %s;", value.code);
    }
    free(scope_name);
    return buf_take(&code);
}

static Expr parse_sharp_expr(Parser *parser, int scope_id, ValueType expected_type) {
    Token sharp_tok = parser->lexer.current;
    Buffer saved_out = parser->out;
    Buffer temp_out;
    int saved_depth = parser->depth;
    int saved_active_scope_count = parser->active_scope_count;
    int inner_scope_id;
    int symbol_depth;
    bool has_keep = false;

    match(parser, TOK_SHARP);
    if (is_type_token(parser->lexer.current.kind)) {
        ValueType annotated = token_to_type(parser->lexer.current.kind);
        if (annotated != expected_type) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "sharp type does not match expected value type");
        }
        advance(parser);
    }
    expect(parser, TOK_LBRACE, "expected '{' after sharp");

    inner_scope_id = ++parser->scope_counter;
    parser->active_scope_ids[parser->active_scope_count++] = inner_scope_id;
    parser->active_scope_loop_depths[parser->active_scope_count - 1] = parser->loop_depth;

    buf_init(&temp_out);
    parser->out = temp_out;
    parser->depth = saved_depth + 1;

    {
        char *inner_scope_name = make_scope_name(inner_scope_id);
        char *outer_scope_name = make_scope_name(scope_id);
        emit_line(parser, "SpearScope %s = spear_scope_enter(&%s);", inner_scope_name, outer_scope_name);
        emit_line(parser, "%s _sharp_value = %s;", ctype_name(expected_type), sharp_default_code(expected_type));
        free(inner_scope_name);
        free(outer_scope_name);
    }

    symbol_depth = parser->depth;
    while (parser->lexer.current.kind != TOK_RBRACE && parser->lexer.current.kind != TOK_EOF) {
        if (match(parser, TOK_KEEP)) {
            Expr value = parse_value_expr(parser, inner_scope_id, expected_type);
            char *line = sharp_keep_code(expected_type, scope_id, value);
            expect(parser, TOK_SEMI, "expected ';' after keep");
            emit_line(parser, "%s", line);
            free(line);
            has_keep = true;
            continue;
        }
        if (parser->lexer.current.kind == TOK_RETURN) {
            fatal_at(parser->lexer.current.line, parser->lexer.current.col, "return is not allowed inside sharp expressions; use keep");
        }
        parse_statement(parser, inner_scope_id);
    }
    expect(parser, TOK_RBRACE, "expected '}'");
    if (!has_keep) {
        fatal_at(sharp_tok.line, sharp_tok.col, "sharp expression requires keep");
    }

    {
        char *inner_scope_name = make_scope_name(inner_scope_id);
        emit_line(parser, "spear_scope_leave(&%s);", inner_scope_name);
        emit_line(parser, "_sharp_value;");
        free(inner_scope_name);
    }

    pop_symbols(parser, symbol_depth);
    parser->active_scope_count = saved_active_scope_count;
    parser->depth = saved_depth;
    temp_out = parser->out;
    parser->out = saved_out;

    {
        Buffer code;
        buf_init(&code);
        buf_append(&code, "({\n");
        if (temp_out.data) {
            buf_append(&code, temp_out.data);
        }
        buf_append(&code, "})");
        return make_expr(expected_type, buf_take(&code));
    }
}
