#ifndef SPEARC_FRONTEND_SCAN_H
#define SPEARC_FRONTEND_SCAN_H

static void collect_functions(Parser *parser, const char *source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    char *current_package = NULL;
    char *current_module = NULL;

    for (;;) {
        Token token = lexer_next(&lexer);
        if (token.kind == TOK_EOF) {
            return;
        }

        if (token.kind == TOK_MODULE || token.kind == TOK_PACKAGE) {
            Token next = lexer_next(&lexer);
            if (next.kind != TOK_IDENT) {
                fatal_at(next.line, next.col, "expected name after %s", token_text(token));
            }
            if (token.kind == TOK_PACKAGE) {
                free(current_package);
                current_package = token_text(next);
            } else {
                free(current_module);
                current_module = token_text(next);
            }
            Token semi = lexer_next(&lexer);
            if (semi.kind != TOK_SEMI) {
                fatal_at(semi.line, semi.col, "expected ';' after metadata");
            }
            continue;
        }
        if (token.kind == TOK_CLASS) {
            fatal_at(token.line, token.col, "class is reserved but not implemented yet");
        }

        bool is_entry = false;
        ValueType return_type = TYPE_NUM;
        if (token.kind == TOK_FUNCTION) {
            token = lexer_next(&lexer);
        }
        if (token.kind == TOK_RUN) {
            FunctionInfo info;
            info.name = xstrdup("__spear_run_entry");
            info.package_name = current_package ? xstrdup(current_package) : xstrdup("");
            info.module_name = current_module ? xstrdup(current_module) : xstrdup("");
            info.c_name = make_function_c_name(info.package_name, info.module_name, info.name, true);
            info.return_type = TYPE_NUM;
            info.is_entry = true;
            info.used = true;
            info.line = token.line;
            info.col = token.col;
            info.params = NULL;
            info.param_count = 0;
            add_function_info(parser, info);
            scan_skip_block(&lexer);
            continue;
        }
        if (token.kind == TOK_SPEAR) {
            is_entry = true;
        } else if (token.kind == TOK_NUM || token.kind == TOK_TEXT || token.kind == TOK_NUMLIST || token.kind == TOK_TEXTLIST || token.kind == TOK_MAP || token.kind == TOK_RESULT || token.kind == TOK_VIEW) {
            return_type = token.kind == TOK_VIEW ? TYPE_TEXT : token_to_type(token.kind);
        } else {
            fatal_at(token.line, token.col, "top-level definition must start with function, run, spear, num, text, numlist, textlist, map, result, or view");
        }

        Token name_tok = lexer_next(&lexer);
        if (name_tok.kind != TOK_IDENT) {
            fatal_at(name_tok.line, name_tok.col, "expected function name");
        }

        Token open = lexer_next(&lexer);
        if (open.kind != TOK_LPAREN) {
            fatal_at(open.line, open.col, "expected '(' after function name");
        }

        Param *params = NULL;
        size_t param_count = 0;
        size_t param_cap = 0;

        Token next = lexer_next(&lexer);
        if (next.kind != TOK_RPAREN) {
            for (;;) {
                if (!is_type_token(next.kind)) {
                    fatal_at(next.line, next.col, "expected parameter type");
                }
                Token param_name = lexer_next(&lexer);
                if (param_name.kind != TOK_IDENT) {
                    fatal_at(param_name.line, param_name.col, "expected parameter name");
                }
                if (param_count == param_cap) {
                    size_t grow = param_cap ? param_cap * 2 : 4;
                    params = realloc(params, grow * sizeof(Param));
                    if (!params) {
                        fprintf(stderr, "spearc error: out of memory\n");
                        exit(1);
                    }
                    param_cap = grow;
                }
                params[param_count].name = token_text(param_name);
                params[param_count].type = token_to_type(next.kind);
                params[param_count].line = param_name.line;
                params[param_count].col = param_name.col;
                param_count++;

                Token sep = lexer_next(&lexer);
                if (sep.kind == TOK_RPAREN) {
                    break;
                }
                if (sep.kind != TOK_COMMA) {
                    fatal_at(sep.line, sep.col, "expected ',' or ')'");
                }
                next = lexer_next(&lexer);
            }
        }

        FunctionInfo info;
        info.name = token_text(name_tok);
        info.package_name = current_package ? xstrdup(current_package) : xstrdup("");
        info.module_name = current_module ? xstrdup(current_module) : xstrdup("");
        info.c_name = make_function_c_name(info.package_name, info.module_name, info.name, is_entry);
        info.return_type = return_type;
        info.is_entry = is_entry;
        info.used = is_entry;
        info.line = name_tok.line;
        info.col = name_tok.col;
        info.params = params;
        info.param_count = param_count;
        add_function_info(parser, info);

        scan_skip_block(&lexer);
    }
}

#endif
