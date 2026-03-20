#ifndef SPEARC_STATEMENT_DEFER_H
#define SPEARC_STATEMENT_DEFER_H

static void emit_deferred_statements(Parser *parser, StringList *list) {
    for (size_t i = list->count; i > 0; i--) {
        buf_append(&parser->out, list->items[i - 1]);
    }
}

static bool parse_defer_statement(Parser *parser, int scope_id, bool *handled) {
    *handled = true;
    if (!match(parser, TOK_DEFER)) {
        *handled = false;
        return false;
    }
    if (parser->defer_list_count <= 0) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "defer is only valid inside blocks");
    }
    if (parser->in_defer_capture) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "defer cannot wrap another defer");
    }

    Buffer saved_out = parser->out;
    Buffer temp_out;
    bool saved_capture = parser->in_defer_capture;
    bool terminated = false;

    buf_init(&temp_out);
    parser->out = temp_out;
    parser->in_defer_capture = true;
    terminated = parse_statement(parser, scope_id);
    temp_out = parser->out;
    parser->out = saved_out;
    parser->in_defer_capture = saved_capture;

    if (terminated) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "defer cannot wrap control-flow statements");
    }

    string_list_push(&parser->defer_lists[parser->defer_list_count - 1], temp_out.data ? temp_out.data : "");
    return false;
}

#endif
