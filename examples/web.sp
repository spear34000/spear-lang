view hero(text title, text body) {
    return stack(
        markup("h1", escape(title)),
        stack(
            markup("p", escape(body)),
            action("/start", "Open App")
        )
    );
}

spear launch() {
    const let title = "Spear UI";
    let body = hero(title, "safe, fast, reliable");
    guard(size(body) > 0, "body must not be empty");
    text html = page(title, body);
    write("build/spear-ui.html", html);
    say(html);
}
