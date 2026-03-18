function view pill(string label) {
    return markup("span") {
        escape(label);
    };
}

function view hero_card(string title, string body) {
    return surface_box(
        modifier(
            corner_radius("24px"),
            modifier(border("1px solid #dbe2ea"), shadow("0 10px 30px rgba(15, 23, 42, 0.08)"))
        )
    ) {
        column_box(gap_space("14px")) {
            badge("Spear Web");
            hero(title, body);
            button_row("/start", "Open App", "#docs", "Docs");
            label_text(foreground("#475569"), "Compose-style UI");
        };
    };
}

run {
    const title = "Spear UI";
    string html = page_frame(
        title,
        centered(header_bar(title, "Declarative blocks for safe web output")),
        centered(column_box(modifier(max_width("1040px"), gap_space("24px"))) {
            nav_bar("/", "Home", "#docs", "Docs", "#about", "About");
            hero_card(title, "Declarative blocks for safe web output");
            split(
                section_block("Highlights", bullet_list(
                    column_box(gap_space("10px")) {
                        item("Use page_frame(...) for full-page layout.");
                        item("Use box(...), column_box(...), row_box(...) with modifiers.");
                        item("Use nav_bar(...) and cta_section(...) for app-style flows.");
                    }
                )),
                section_block("Status", column_box(gap_space("12px")) {
                    metric("Compiler", "ready");
                    metric("Runtime", "safe");
                    metric("Installer", "included");
                })
            );
            spacer("8px");
            success_note("Safe by default", "HTML text is escaped before it lands in the page.");
            quote_block("Build pages with plain functions, then export finished HTML.");
            cta_section("Get Started", "Build a page, export HTML, and ship it fast.", "/start", "Open App");
            footer_note("Built with the Spear standard web library.");
        }),
        footer_note("Spear UI example")
    );
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
