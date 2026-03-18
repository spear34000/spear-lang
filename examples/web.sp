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
                intro(
                    "Spear should have its own design grammar.",
                    "The goal is not to hide styling completely. The goal is to replace raw CSS with reusable design tokens that still let you steer the result."
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
            };
        });
        box(modifier(dark_panel_mod(), modifier(max_width("1160px"), center_x()))) {
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
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
