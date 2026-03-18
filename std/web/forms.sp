package std;
module webforms;

function text name_attr(text value) {
    return attr("name", value);
}

function text value_attr(text value) {
    return attr("value", value);
}

function text type_attr(text value) {
    return attr("type", value);
}

function text placeholder_attr(text value) {
    return attr("placeholder", value);
}

function text rows_attr(text value) {
    return attr("rows", value);
}

function text method_attr(text value) {
    return attr("method", value);
}

function text action_attr(text value) {
    return attr("action", value);
}

function text required_attr() {
    return attr("required", "required");
}

function text selected_attr() {
    return attr("selected", "selected");
}

function view field_label(text value) {
    return markup("label", style_attr(eyebrow_mod())) {
        escape(value);
    };
}

function view field_shell(text label, text control, text hint) {
    return column_box(field_stack_mod()) {
        field_label(label);
        control;
        if (size(hint) > 0) {
            form_hint(hint);
        };
    };
}

function view text_field(text label, text placeholder) {
    return field_shell(label, markup("div", style_attr(input_shell_mod())) {
        escape(placeholder);
    }, "");
}

function view text_field_dark(text label, text placeholder) {
    return field_shell(label, markup("div", style_attr(input_dark_shell_mod())) {
        escape(placeholder);
    }, "");
}

function view text_input(text label, text name, text value, text placeholder) {
    return field_shell(
        label,
        markup("input", attrs(
            type_attr("text"),
            attrs(
                name_attr(name),
                attrs(
                    value_attr(value),
                    attrs(placeholder_attr(placeholder), style_attr(input_control_mod()))
                )
            )
        ), ""),
        ""
    );
}

function view email_input(text label, text name, text value, text placeholder) {
    return field_shell(
        label,
        markup("input", attrs(
            type_attr("email"),
            attrs(
                name_attr(name),
                attrs(
                    value_attr(value),
                    attrs(placeholder_attr(placeholder), style_attr(input_control_mod()))
                )
            )
        ), ""),
        ""
    );
}

function view password_input(text label, text name, text value, text placeholder) {
    return field_shell(
        label,
        markup("input", attrs(
            type_attr("password"),
            attrs(
                name_attr(name),
                attrs(
                    value_attr(value),
                    attrs(placeholder_attr(placeholder), style_attr(input_control_mod()))
                )
            )
        ), ""),
        ""
    );
}

function view search_input(text label, text name, text value, text placeholder) {
    return field_shell(
        label,
        markup("input", attrs(
            type_attr("search"),
            attrs(
                name_attr(name),
                attrs(
                    value_attr(value),
                    attrs(placeholder_attr(placeholder), style_attr(input_control_mod()))
                )
            )
        ), ""),
        ""
    );
}

function view text_area(text label, text name, text value, text placeholder, text rows_value) {
    return field_shell(
        label,
        markup("textarea", attrs(
            name_attr(name),
            attrs(
                rows_attr(rows_value),
                attrs(placeholder_attr(placeholder), style_attr(textarea_control_mod()))
            )
        )) {
            escape(value);
        },
        ""
    );
}

function view select_option(text value, text label, text active) {
    variable option_attrs = value_attr(value);
    if (same(active, "yes")) {
        option_attrs = attrs(option_attrs, selected_attr());
    }
    return markup("option", option_attrs) {
        escape(label);
    };
}

function view select_field(text label, text name, text options) {
    return field_shell(
        label,
        markup("select", attrs(name_attr(name), style_attr(select_control_mod()))) {
            options;
        },
        ""
    );
}

function view form_hint(text value) {
    return markup("p", style_attr(caption_mod())) {
        escape(value);
    };
}

function view submit_button(text label) {
    return markup("button", attrs(type_attr("submit"), style_attr(action_warm_mod()))) {
        escape(label);
    };
}

function view secondary_button(text label) {
    return markup("button", attrs(type_attr("button"), style_attr(button_secondary_mod()))) {
        escape(label);
    };
}

function view form_row(text controls) {
    return row_box(form_actions_mod()) {
        controls;
    };
}

function view form_panel(text action_url, text method_name, text body) {
    return markup("form", attrs(action_attr(action_url), attrs(method_attr(method_name), style_attr(field_stack_mod())))) {
        body;
    };
}

function view form_card(text title, text body, text fields, text action_href, text action_label) {
    return box(form_card_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(body);
                };
            };
            form_panel(action_href, "post", column_box(gap_space(space_2())) {
                fields;
                form_row(submit_button(action_label));
            });
        };
    };
}
