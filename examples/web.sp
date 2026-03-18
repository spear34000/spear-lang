run {
    const title = "Spear Web";
    string html = landing_page(title, theme_product_bg(), showcase_stack(
        hero_scene(
            brand_header("Spear Web", "Readable blocks. Sharp output.", "#vision", "Vision", "#signals", "Signals", "#launch", "Launch"),
            column_box(gap_space("22px")) {
                spotlight(
                    "Spear Scene",
                    "Design strong product pages without falling back to raw CSS all day.",
                    "Spear Web should make high-end layout, hierarchy, and conversion flow feel like normal language features.",
                    "#launch",
                    "Start with the launch block",
                    "#signals",
                    "See the signal charts"
                );
                prestige_stats("Shipping speed", "fast", "Design floor", "high", "Boilerplate", "low");
            },
            column_box(gap_space("20px")) {
                code_window("examples/web.sp", "run {\n    string html = landing_page(\"Spear\", theme_product_bg(), body);\n    write(\"build/spear-ui.html\", html);\n}");
                feature_triptych(
                    "Scene", "Think in sections", "Use `hero_scene`, `insights_band`, and `conversion_band` as the main shape of a page.",
                    "Signal", "Think in visuals", "Charts and stat bars should feel as native as text blocks.",
                    "Taste", "Think in presets", "Themes and premium blocks should make weak-looking pages harder to produce."
                );
            }
        ),
        insights_band(
            "Spear should have its own design grammar.",
            "Instead of exposing CSS more directly, Spear should expose layout scenes, signal blocks, premium cards, and conversion flows.",
            column_box(gap_space("18px")) {
                feature_triptych(
                    "Layouts", "Reusable page structure", "Landing pages, dashboards, launches, and showcases should start from named blocks.",
                    "Charts", "Real data visuals", "The language should accept labels and numbers directly for the most common visual patterns.",
                    "Themes", "Opinionated quality", "Built-in themes should push pages toward a premium baseline even before custom styling."
                );
                metric_bars(
                    "Signal board",
                    "Numbers should turn into usable visuals with almost no ceremony.",
                    pack("Ease of writing", "Visual quality", "Flexibility"),
                    pack(86, 78, 64),
                    pack("#4f8cff", "#ff8a3d", "#111827")
                );
            }
        ),
        conversion_band(
            "Build pages that already look like someone cared.",
            "That should be the default outcome of Spear Web, not the result of hand-writing a pile of CSS strings.",
            "/start",
            "Build with Spear"
        )
    ));
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
