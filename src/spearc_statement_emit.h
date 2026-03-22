#ifndef SPEARC_STATEMENT_EMIT_H
#define SPEARC_STATEMENT_EMIT_H

static bool parse_emit_statement(Parser *parser, int scope_id, bool *handled) {
    *handled = true;
    if (!match(parser, TOK_EMIT)) {
        *handled = false;
        return false;
    }
    if (parser->sharp_type_count <= 0) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "emit is only valid inside sharp blocks");
    }
    if (parser->sharp_types[parser->sharp_type_count - 1] != TYPE_TEXT) {
        fatal_at(parser->lexer.current.line, parser->lexer.current.col, "emit currently works only inside sharp text blocks");
    }

    Expr value = parse_text_expr(parser, scope_id);
    expect(parser, TOK_SEMI, "expected ';' after emit");
    {
        char *scope_name = make_scope_name(scope_id);
        emit_line(parser, "_sharp_value = spear_text_join(&%s, _sharp_value, %s);", scope_name, value.code);
        free(scope_name);
    }
    return false;
}

#endif
