view pill(text label) {
    return markup("span") {
        escape(label);
    };
}

view hero(text title, text body) {
    return markup("section") {
        markup("h1") {
            escape(title);
        };
        markup("p") {
            escape(body);
        };
        row {
            action("/start", "Open App");
            action("#docs", "Docs");
            pill("Compose-style UI");
        };
    };
}

spear launch() {
    const let title = "Spear UI";
    text html = page(title) {
        column {
            hero(title, "Declarative blocks for safe web output");
            markup("section") {
                markup("h2") {
                    "Highlights";
                };
                column {
                    markup("p") {
                        "Use page(title) { ... } for document layout.";
                    };
                    markup("p") {
                        "Nest column { ... } and row { ... } like a Compose tree.";
                    };
                    markup("p") {
                        "Keep escape(...) around user data and write() the final page.";
                    };
                };
            };
        };
    };
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    say(html);
}
