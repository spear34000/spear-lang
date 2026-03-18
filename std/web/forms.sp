package std;
module webforms;

function view field_label(text value) {
    return markup("label", style_attr(eyebrow_mod())) {
        escape(value);
    };
}

function view text_field(text label, text placeholder) {
    return column_box(field_stack_mod()) {
        field_label(label);
        markup("div", style_attr(input_shell_mod())) {
            escape(placeholder);
        };
    };
}

function view text_field_dark(text label, text placeholder) {
    return column_box(field_stack_mod()) {
        field_label(label);
        markup("div", style_attr(input_dark_shell_mod())) {
            escape(placeholder);
        };
    };
}

function view form_hint(text value) {
    return markup("p", style_attr(caption_mod())) {
        escape(value);
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
            fields;
            button_link(action_warm_mod(), action_href, action_label);
        };
    };
}
