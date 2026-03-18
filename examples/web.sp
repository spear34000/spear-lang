run {
    const title = "Spear Web";
    string html = spectacle_page(title, showcase_shell(showcase_stack(
        hero_showcase(
            brand_header("Spear Web", "Readable blocks. Sharp output.", "#vision", "Vision", "#components", "Components", "#launch", "Launch"),
            column_box(gap_space("22px")) {
                spotlight(
                    "Visual Example",
                    "Build a page that looks expensive with beginner-friendly code.",
                    "Spear Web is moving toward semantic blocks, reusable page presets, and built-in visuals instead of raw CSS strings everywhere.",
                    "#launch",
                    "Open Launch Notes",
                    "#components",
                    "Inspect Components"
                );
                prestige_stats("Build Time", "< 1 min", "Code Feel", "plain words", "Output", "HTML");
            },
            column_box(gap_space("20px")) {
                code_window("examples/web.sp", "run {\n    string html = spectacle_page(\"Spear Web\", dashboard);\n    write(\"build/spear-ui.html\", html);\n}");
                aura_triptych(
                    "Readable", "Functions that read like intent", "Use semantic blocks like `spotlight`, `aura_triptych`, and `launch_banner`.",
                    "Safe", "Escaped output by default", "The runtime still protects the common output path while you compose bold layouts.",
                    "Fast", "Reusable premium presets", "High-level building blocks make strong pages easier to ship."
                );
            }
        ),
        feature_stage(
            "Design with a Spear voice",
            "The goal is not to expose CSS more cleverly. The goal is to let Spear express layout, tone, and visual hierarchy in its own words.",
            column_box(gap_space("18px")) {
                aura_triptych(
                    "Scenes", "Page-level storytelling", "Start from big blocks like `hero_showcase`, `feature_stage`, and `launch_banner`.",
                    "Signals", "Data that still feels designed", "Use charts and stats as first-class presentation pieces instead of ad-hoc div piles.",
                    "Presets", "A consistent premium tone", "Cards, spacing, glow, rhythm, and CTA patterns stay aligned across sections."
                );
                comparison_chart(
                    "Signal snapshot",
                    "Data visuals should be just as easy to stage as content.",
                    "Ease of writing", "high", "88%", "#4f8cff",
                    "Design quality", "rising", "74%", "#ff8a3d",
                    "Boilerplate", "low", "22%", "#111827"
                );
            }
        ),
        launch_banner(
            "Ship premium pages with fewer decisions.",
            "Use Spear presets to get strong hierarchy, safer output, and sharper visuals without dropping into raw CSS every few lines.",
            "/start",
            "Build something now"
        )
    )));
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
