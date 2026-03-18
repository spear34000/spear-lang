package std;
module webdashboard;

function view sidebar_shell(text title, text body) {
    return box(form_card_mod()) {
        column_box(gap_space(space_3())) {
            markup("h3", style_attr(card_title_mod())) {
                escape(title);
            };
            body;
        };
    };
}

function view sidebar_link(text href, text label, text active) {
    variable mods = modifier(
        padding_xy("12px", "14px"),
        modifier(
            corner_radius(radius_2()),
            modifier(
                background("rgba(255,255,255,0.04)"),
                modifier(foreground(tone_inverse()), style_rule("text-decoration", "none"))
            )
        )
    );
    if (same(active, "yes")) {
        mods = modifier(
            padding_xy("12px", "14px"),
            modifier(
                corner_radius(radius_2()),
                modifier(background("rgba(79,140,255,0.18)"), modifier(border("1px solid rgba(79,140,255,0.38)"), foreground(tone_inverse())))
            )
        );
    }
    return button_link(mods, href, label);
}

function view tab_chip(text label, text active) {
    variable mods = modifier(
        padding_xy("10px", "14px"),
        modifier(
            corner_radius(radius_pill()),
            modifier(background("#eef2f7"), modifier(foreground(tone_muted()), border("1px solid rgba(15,23,42,0.06)")))
        )
    );
    if (same(active, "yes")) {
        mods = modifier(
            padding_xy("10px", "14px"),
            modifier(
                corner_radius(radius_pill()),
                modifier(background("#0f172a"), modifier(foreground("#f8fafc"), shadow(elevation_1())))
            )
        );
    }
    return markup("span", style_attr(mods)) {
        escape(label);
    };
}

function view tab_row(text first, text second, text third) {
    return row_box(modifier(gap_space(space_2()), wrap())) {
        first;
        second;
        third;
    };
}

function view stat_panel(text label, text value, text note) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_2())) {
            markup("p", style_attr(eyebrow_mod())) {
                escape(label);
            };
            markup("strong", style_attr(display_title_mod())) {
                escape(value);
            };
            markup("p", style_attr(body_copy_mod())) {
                escape(note);
            };
        };
    };
}

function view dashboard_grid(text first, text second, text third, text fourth) {
    return markup("section", style_attr("display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:18px")) {
        first;
        second;
        third;
        fourth;
    };
}

function view table_cell(text value, text tone) {
    return markup("td", style_attr(join("padding:12px 14px;border-top:1px solid rgba(15,23,42,0.06);color:", tone))) {
        escape(value);
    };
}

function view table_header(text value) {
    return markup("th", style_attr("padding:12px 14px;text-align:left;color:#475569;font-size:13px;letter-spacing:0.04em;text-transform:uppercase")) {
        escape(value);
    };
}

function view table_row(text first, text second, text third) {
    return markup("tr") {
        table_cell(first, tone_ink());
        table_cell(second, tone_muted());
        table_cell(third, tone_muted());
    };
}

function view data_table(text title, text body, text columns, text rows) {
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
            markup("table", style_attr("width:100%;border-collapse:collapse")) {
                markup("thead") {
                    markup("tr") {
                        columns;
                    };
                };
                markup("tbody") {
                    rows;
                };
            };
        };
    };
}
