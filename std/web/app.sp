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
