run {
    const title = "Spear Web";
    string html = landing_page(title, theme_product_bg()) {
        centered(box(glass_panel_mod()) {
            column_box(gap_space(space_5())) {
                row_box(modifier(justify_between(), modifier(align_center(), modifier(padding_y(space_1()), wrap())))) {
                    row_box(modifier(align_center(), gap_space(space_2()))) {
                        markup("div", style_attr("width:46px;height:46px;border-radius:16px;background:linear-gradient(135deg,#4f8cff,#87b4ff);box-shadow:0 18px 40px rgba(79,140,255,0.35)"), "");
                        column_box(gap_space("4px")) {
                            markup("strong", style_attr("font-size:19px;letter-spacing:0.02em")) {
                                "Spear Web";
                            };
                            label_text(foreground(tone_inverse_muted()), "Readable blocks. Sharp output.");
                        };
                    };
                    row_box(gap_space("10px")) {
                        button_link(button_secondary_mod(), "#vision", "Vision");
                        button_link(button_secondary_mod(), "#signals", "Signals");
                        button_link(button_secondary_mod(), "#launch", "Launch");
                    };
                };
                row_box(modifier(gap_space(space_5()), modifier(align_start(), wrap()))) {
                    markup("section", style_attr("flex:1;min-width:420px")) {
                        column_box(gap_space("22px")) {
                            markup("span", style_attr(badge_mod())) {
                                "Spear Scene";
                            };
                            markup("h1", style_attr(hero_title_mod())) {
                                "Design strong product pages without falling back to raw CSS all day.";
                            };
                            markup("p", style_attr(lead_copy_mod())) {
                                "Spear Web should give you sharp defaults without locking you into a rigid template.";
                            };
                            row_box(gap_space(space_2())) {
                                button_link(button_primary_mod(), "#launch", "Start with the launch block");
                                button_link(button_secondary_mod(), "#signals", "See the signal charts");
                            };
                            row_box(modifier(gap_space(space_2()), wrap())) {
                                box(stat_chip_mod()) {
                                    column_box(gap_space("8px")) {
                                        label_text(foreground(tone_inverse_muted()), "Shipping speed");
                                        markup("strong", style_attr("font-size:28px;line-height:1.05")) {
                                            "fast";
                                        };
                                    };
                                };
                                box(stat_chip_mod()) {
                                    column_box(gap_space("8px")) {
                                        label_text(foreground(tone_inverse_muted()), "Design floor");
                                        markup("strong", style_attr("font-size:28px;line-height:1.05")) {
                                            "high";
                                        };
                                    };
                                };
                                box(stat_chip_mod()) {
                                    column_box(gap_space("8px")) {
                                        label_text(foreground(tone_inverse_muted()), "Autonomy");
                                        markup("strong", style_attr("font-size:28px;line-height:1.05")) {
                                            "kept";
                                        };
                                    };
                                };
                            };
                        };
                    };
                    markup("section", style_attr("flex:1;min-width:360px")) {
                        column_box(gap_space(space_3())) {
                            box(dark_panel_mod()) {
                                column_box(gap_space(space_3())) {
                                    row_box(modifier(justify_between(), align_center())) {
                                        row_box(gap_space("8px")) {
                                            markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#ff6b6b"), "");
                                            markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#f7b955"), "");
                                            markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#51cf66"), "");
                                        };
                                        label_text(foreground("rgba(236,243,255,0.62)"), "examples/web.sp");
                                    };
                                    markup("pre", style_attr("margin:0;color:#dbeafe;font-size:15px;line-height:1.65;white-space:pre-wrap")) {
                                        "run {\n    string html = landing_page(\"Spear\", theme_product_bg()) {\n        section;\n    };\n}";
                                    };
                                };
                            };
                            form_card(
                                "Prototype faster",
                                "Use form primitives that already sit inside the same tone and spacing system.",
                                column_box(gap_space(space_2())) {
                                    text_field_dark("Project name", "Spear Commerce");
                                    text_field_dark("Goal", "Premium launch page with charts");
                                    form_hint("These are design-first input shells. Real form submission can layer on later.");
                                },
                                "/start",
                                "Start prototype"
                            );
                            feature_triptych(
                                "Scene", "Think in sections", "Use layout primitives and token bundles instead of hardcoded CSS strings.",
                                "Signal", "Think in visuals", "Charts and stat bars should feel like normal Spear building blocks.",
                                "Taste", "Think in systems", "Tone, spacing, surface, and elevation should stay composable."
                            );
                        };
                    };
                };
            };
        });
        centered(box(paper_panel_mod()) {
            column_box(gap_space(space_3())) {
                tab_row(
                    tab_chip("Overview", "yes"),
                    tab_chip("Signals", "no"),
                    tab_chip("Launches", "no")
                );
                intro(
                    "Spear should have its own design grammar.",
                    "The goal is not to hide styling completely. The goal is to replace raw CSS with reusable design tokens that still let you steer the result."
                );
                dashboard_grid(
                    stat_panel("Weekly visitors", "184k", "Traffic should be readable at a glance."),
                    stat_panel("Conversion rate", "4.8%", "The visual baseline should stay strong under real numbers."),
                    stat_panel("Build friction", "low", "Primitives should remove ceremony, not control."),
                    stat_panel("Theme drift", "stable", "Tokens keep pages coherent while you remix layouts.")
                );
                feature_triptych(
                    "Layouts", "Reusable structure", "Use rows, columns, boxes, and sections freely without dropping design quality.",
                    "Charts", "Real data visuals", "The language can now turn numbers into text and chart widths without ceremony.",
                    "Themes", "Opinionated quality", "Built-in tones and surfaces raise the visual floor while preserving freedom."
                );
                metric_bars(
                    "Signal board",
                    "Good primitives should make expressive UI easier, not narrower.",
                    pack("Ease of writing", "Visual quality", "Freedom to remix"),
                    pack(86, 82, 88),
                    pack(tone_brand(), tone_warm(), tone_ink())
                );
                split(
                    trend_line_card(
                        "Release momentum",
                        "This style of summary should be easy to drop into a product dashboard.",
                        "+18%",
                        tone_brand(),
                        pack(42, 58, 49, 71, 84, 92)
                    ),
                    timeline_feed(
                        "Ship log",
                        "The same system should handle status storytelling without custom CSS.",
                        pack("Mon", "Tue", "Wed"),
                        pack("Design tokens added", "Form kit added", "Dashboard kit added"),
                        pack("Spacing, tone, and panel bundles now compose across pages.", "Input shells match the product surface.", "Tables, tabs, and sidebars now fit the same visual language.")
                    )
                );
                data_table(
                    "Recent launches",
                    "Structured data should fit the same design language as marketing sections.",
                    join(
                        join(table_header("Project"), table_header("Theme")),
                        table_header("Status")
                    ),
                    join(
                        join(
                            table_row("Spear Commerce", "Product", "Shipped"),
                            table_row("Atlas Docs", "Editorial", "Review")
                        ),
                        table_row("Northstar CRM", "Cinema", "Draft")
                    )
                );
            };
        });
        centered(row_box(modifier(gap_space(space_3()), modifier(align_start(), wrap()))) {
            markup("section", style_attr("flex:0 0 260px;min-width:240px")) {
                sidebar_shell("Workspace") {
                    column_box(gap_space(space_2())) {
                        sidebar_link("#overview", "Overview", "yes");
                        sidebar_link("#signals", "Signals", "no");
                        sidebar_link("#launches", "Launches", "no");
                        sidebar_link("#settings", "Settings", "no");
                    };
                };
            };
            markup("section", style_attr("flex:1;min-width:420px")) {
                box(modifier(dark_panel_mod(), width_fill())) {
                    row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                        column_box(gap_space(space_2())) {
                            markup("h2", style_attr("margin:0;font-size:34px;line-height:1.05;color:#f8fafc")) {
                                "Build pages that already look like someone cared.";
                            };
                            label_text(foreground("rgba(248,250,252,0.72)"), "That should come from the language's design system, not from losing control.");
                        };
                        button_link(action_warm_mod(), "/start", "Build with Spear");
                    };
                };
            };
        });
        centered(column_box(gap_space(space_3())) {
            calendar_strip(
                "Release calendar",
                "Scheduling UI should match the rest of the system instead of starting from scratch.",
                join(
                    join(
                        calendar_chip("Mon", "Design audit", "no"),
                        calendar_chip("Tue", "Ship tokens", "yes")
                    ),
                    join(
                        calendar_chip("Wed", "Review forms", "no"),
                        calendar_chip("Thu", "Launch page", "no")
                    )
                )
            );
            kanban_board(
                kanban_lane("Backlog", "2 items", join(
                    kanban_card("Research", "Chart patterns", "Compare timelines, trends, and scoreboards."),
                    kanban_card("UI", "Drawer system", "Keep side panels inside the same tone rules.")
                )),
                kanban_lane("In progress", "2 items", join(
                    kanban_card("Tokens", "Refine elevation", "Tune how panels stack in dense dashboards."),
                    kanban_card("Apps", "Modal actions", "Make high-emphasis decisions feel premium.")
                )),
                kanban_lane("Done", "1 item", kanban_card("Foundation", "Design grammar", "Spacing, surfaces, charts, and data tables now share one system."))
            );
            split(
                activity_feed(
                    "Activity feed",
                    "Operational updates should read cleanly without losing visual polish.",
                    join(
                        join(
                            activity_item("10:32", "Published token update", "Form primitives now inherit the same spacing and panel language."),
                            activity_item("12:18", "Reviewed launch draft", "Trend cards and timeline feeds were added to the product dashboard.")
                        ),
                        activity_item("14:05", "Prepared release candidate", "Calendar and kanban blocks now exist for app-style screens.")
                    )
                ),
                column_box(gap_space(space_3())) {
                    modal_card(
                        "Promote this release?",
                        "High-emphasis decisions should already feel designed before you fine-tune the product.",
                        join(
                            button_link(button_secondary_mod(), "#later", "Review later"),
                            button_link(action_warm_mod(), "#ship", "Ship now")
                        )
                    );
                    drawer_panel(
                        "Right-side tools",
                        "Drawers should be easy to stage for settings, comments, or quick actions.",
                        column_box(gap_space(space_2())) {
                            text_field_dark("Owner", "Spear UI team");
                            text_field_dark("Status", "Ready for review");
                            form_hint("This panel uses the same form tokens as the rest of the page.");
                        }
                    );
                }
            );
        });
    };
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
