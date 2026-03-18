package std;
module webapp;

function view activity_item(text stamp, text title, text body) {
    return markup("section", style_attr("display:flex;gap:14px;align-items:flex-start")) {
        markup("div", style_attr("padding-top:4px")) {
            markup("span", style_attr("display:inline-block;width:12px;height:12px;border-radius:999px;background:#ff8a3d;box-shadow:0 0 0 6px rgba(255,138,61,0.16)"), "");
        };
        column_box(gap_space("6px")) {
            markup("p", style_attr(caption_mod())) {
                escape(stamp);
            };
            markup("h4", style_attr(card_title_mod())) {
                escape(title);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(body);
            };
        };
    };
}

function view activity_feed(text title, text body, text items) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_mod())) {
                    escape(body);
                };
            };
            column_box(gap_space(space_3())) {
                items;
            };
        };
    };
}

function view kanban_card(text tag, text title, text note) {
    return box(modifier(padding(space_3()), modifier(corner_radius(radius_2()), modifier(background("#ffffff"), modifier(border(line_soft()), shadow(elevation_1())))))) {
        column_box(gap_space(space_2())) {
            markup("span", style_attr(badge_mod())) {
                escape(tag);
            };
            markup("h4", style_attr(card_title_mod())) {
                escape(title);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(note);
            };
        };
    };
}

function view kanban_lane(text title, text count_value, text cards) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            row_box(modifier(justify_between(), align_center())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("span", style_attr(caption_mod())) {
                    escape(count_value);
                };
            };
            column_box(gap_space(space_2())) {
                cards;
            };
        };
    };
}

function view kanban_board(text first, text second, text third) {
    return markup("section", style_attr("display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:18px")) {
        first;
        second;
        third;
    };
}

function view calendar_chip(text day, text label, text active) {
    variable mods = modifier(
        padding(space_2()),
        modifier(
            corner_radius(radius_2()),
            modifier(background("#ffffff"), modifier(border(line_soft()), shadow(elevation_1())))
        )
    );
    if (same(active, "yes")) {
        mods = modifier(
            padding(space_2()),
            modifier(
                corner_radius(radius_2()),
                modifier(background("rgba(79,140,255,0.14)"), modifier(border("1px solid rgba(79,140,255,0.36)"), shadow(elevation_1())))
            )
        );
    }
    return box(mods) {
        column_box(gap_space("6px")) {
            markup("p", style_attr(eyebrow_mod())) {
                escape(day);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(label);
            };
        };
    };
}

function view calendar_strip(text title, text body, text chips) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_mod())) {
                    escape(body);
                };
            };
            row_box(modifier(gap_space(space_2()), wrap())) {
                chips;
            };
        };
    };
}

function view modal_card(text title, text body, text actions) {
    return box(modifier(max_width("540px"), modifier(center_x(), dark_panel_mod()))) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("p", style_attr(eyebrow_mod())) {
                    "Modal";
                };
                markup("h3", style_attr("margin:0;font-size:28px;line-height:1.1;color:#f8fafc")) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(body);
                };
            };
            row_box(modifier(gap_space(space_2()), wrap())) {
                actions;
            };
        };
    };
}

function view drawer_panel(text title, text body, text content) {
    return box(modifier(max_width("380px"), modifier(background("#0b1220"), modifier(corner_radius(radius_3()), modifier(padding(space_4()), shadow(elevation_3())))))) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("p", style_attr(eyebrow_mod())) {
                    "Drawer";
                };
                markup("h3", style_attr("margin:0;font-size:24px;line-height:1.1;color:#f8fafc")) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(body);
                };
            };
            content;
        };
    };
}

function view toast_chip(text title, text body, text tone) {
    return box(modifier(
        padding_xy("12px", "14px"),
        modifier(
            corner_radius(radius_2()),
            modifier(
                background("#ffffff"),
                modifier(border(line_soft()), modifier(shadow(elevation_1()), style_rule("border-left", join("4px solid ", tone))))
            )
        )
    )) {
        column_box(gap_space("6px")) {
            markup("h4", style_attr(card_title_mod())) {
                escape(title);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(body);
            };
        };
    };
}

function view toast_stack(text first, text second, text third) {
    return column_box(gap_space(space_2())) {
        first;
        second;
        third;
    };
}

function view filter_chip(text label, text active) {
    variable mods = modifier(
        padding_xy("10px", "14px"),
        modifier(
            corner_radius(radius_pill()),
            modifier(background("#ffffff"), modifier(border(line_soft()), modifier(foreground(tone_muted()), shadow(elevation_1()))))
        )
    );
    if (same(active, "yes")) {
        mods = modifier(
            padding_xy("10px", "14px"),
            modifier(
                corner_radius(radius_pill()),
                modifier(background("rgba(79,140,255,0.14)"), modifier(border("1px solid rgba(79,140,255,0.36)"), foreground(tone_brand())))
            )
        );
    }
    return markup("span", style_attr(mods)) {
        escape(label);
    };
}

function view filter_row(text first, text second, text third, text fourth) {
    return row_box(modifier(gap_space(space_2()), wrap())) {
        first;
        second;
        third;
        fourth;
    };
}

function view command_palette(text title, text body, text items) {
    return box(modifier(max_width("640px"), modifier(center_x(), dark_panel_mod()))) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("p", style_attr(eyebrow_mod())) {
                    "Command palette";
                };
                markup("h3", style_attr("margin:0;font-size:28px;line-height:1.1;color:#f8fafc")) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(body);
                };
            };
            markup("div", style_attr(input_dark_shell_mod())) {
                "Search commands, pages, and actions";
            };
            column_box(gap_space(space_2())) {
                items;
            };
        };
    };
}

function view command_item(text keycap, text label, text note) {
    return box(modifier(
        padding_xy("12px", "14px"),
        modifier(
            corner_radius(radius_2()),
            modifier(background("rgba(255,255,255,0.04)"), border(line_inverse_soft()))
        )
    )) {
        row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
            column_box(gap_space("4px")) {
                markup("h4", style_attr("margin:0;font-size:18px;line-height:1.15;color:#f8fafc")) {
                    escape(label);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(note);
                };
            };
            markup("span", style_attr("display:inline-block;padding:6px 10px;border-radius:12px;background:rgba(255,255,255,0.08);color:#f8fafc;font-weight:700")) {
                escape(keycap);
            };
        };
    };
}

function view empty_state_panel(text title, text body, text href, text label) {
    return box(paper_panel_mod()) {
        column_box(modifier(gap_space(space_3()), align_center())) {
            markup("div", style_attr("width:72px;height:72px;border-radius:24px;background:linear-gradient(135deg,rgba(79,140,255,0.18),rgba(255,138,61,0.18))"), "");
            column_box(modifier(gap_space(space_1()), align_center())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_mod())) {
                    escape(body);
                };
            };
            button_link(action_warm_mod(), href, label);
        };
    };
}

function view master_detail(text master, text detail) {
    return row_box(modifier(gap_space(space_3()), modifier(align_start(), wrap()))) {
        markup("section", style_attr("flex:0 0 280px;min-width:260px")) {
            master;
        };
        markup("section", style_attr("flex:1;min-width:380px")) {
            detail;
        };
    };
}

function view step_chip(text index_value, text label, text active) {
    variable shell = modifier(
        padding_xy("10px", "14px"),
        modifier(
            corner_radius(radius_2()),
            modifier(background("#ffffff"), modifier(border(line_soft()), shadow(elevation_1())))
        )
    );
    if (same(active, "yes")) {
        shell = modifier(
            padding_xy("10px", "14px"),
            modifier(
                corner_radius(radius_2()),
                modifier(background("rgba(79,140,255,0.14)"), modifier(border("1px solid rgba(79,140,255,0.36)"), shadow(elevation_1())))
            )
        );
    }
    return box(shell) {
        row_box(modifier(gap_space(space_2()), align_center())) {
            markup("span", style_attr("display:inline-block;width:28px;height:28px;border-radius:999px;background:#0f172a;color:#f8fafc;text-align:center;line-height:28px;font-weight:700")) {
                escape(index_value);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(label);
            };
        };
    };
}

function view stepper(text first, text second, text third, text fourth) {
    return row_box(modifier(gap_space(space_2()), wrap())) {
        first;
        second;
        third;
        fourth;
    };
}

function view status_banner(text title, text body, text tone) {
    return box(modifier(
        padding(space_3()),
        modifier(
            corner_radius(radius_2()),
            modifier(background("#ffffff"), modifier(border(line_soft()), style_rule("border-left", join("4px solid ", tone))))
        )
    )) {
        column_box(gap_space(space_1())) {
            markup("h4", style_attr(card_title_mod())) {
                escape(title);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(body);
            };
        };
    };
}

function view selection_item(text title, text note, text active) {
    variable shell = modifier(
        padding(space_3()),
        modifier(
            corner_radius(radius_2()),
            modifier(background("#ffffff"), modifier(border(line_soft()), shadow(elevation_1())))
        )
    );
    if (same(active, "yes")) {
        shell = modifier(
            padding(space_3()),
            modifier(
                corner_radius(radius_2()),
                modifier(background("rgba(79,140,255,0.10)"), modifier(border("1px solid rgba(79,140,255,0.36)"), shadow(elevation_1())))
            )
        );
    }
    return box(shell) {
        column_box(gap_space("6px")) {
            markup("h4", style_attr(card_title_mod())) {
                escape(title);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(note);
            };
        };
    };
}

function view selection_list(text title, text body, text items) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_mod())) {
                    escape(body);
                };
            };
            column_box(gap_space(space_2())) {
                items;
            };
        };
    };
}

function view detail_panel(text title, text body, text content) {
    return box(dark_panel_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("p", style_attr(eyebrow_mod())) {
                    "Detail";
                };
                markup("h3", style_attr("margin:0;font-size:28px;line-height:1.1;color:#f8fafc")) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_inverse_mod())) {
                    escape(body);
                };
            };
            content;
        };
    };
}
