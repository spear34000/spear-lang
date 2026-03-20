static void parse_block(Parser *parser, int parent_scope_id, bool creates_scope) {
    if (parser->defer_list_count >= (int) (sizeof(parser->defer_lists) / sizeof(parser->defer_lists[0]))) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "block nesting is too deep");
    }
    int defer_index = parser->defer_list_count++;
    parser->defer_lists[defer_index].items = NULL;
    parser->defer_lists[defer_index].count = 0;
    parser->defer_lists[defer_index].cap = 0;

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

    emit_deferred_statements(parser, &parser->defer_lists[defer_index]);

    if (creates_scope) {
        char *scope_name = make_scope_name(scope_id);
        emit_line(parser, "spear_scope_leave(&%s);", scope_name);
        free(scope_name);
        parser->active_scope_count--;
    }
    parser->defer_list_count--;
    pop_symbols(parser, symbol_depth);
    parser->depth--;
    emit_line(parser, "}");
}
