run {
    const title = "Spear Atelier";
    const issue_count = 3;

    string html = landing_page(title, theme_editorial_bg()) {
        centered(box(modifier(glass_panel_mod(), modifier(padding(space_5()), max_width("1120px")))) {
            column_box(gap_space(space_5())) {
                row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                    column_box(gap_space("4px")) {
                        markup("span", style_attr(eyebrow_mod())) {
                            "Spear Web";
                        };
                        markup("strong", style_attr("font-size:20px;letter-spacing:0.02em;color:#f8fafc")) {
                            "Atelier";
                        };
                    };
                    row_box(gap_space(space_2())) {
                        state_link("#story", "Story", "yes");
                        state_link("#system", "System", "no");
                        state_link("#studio", "Studio", "no");
                        state_link("#start", "Start", "no");
                    };
                };
                row_box(modifier(gap_space(space_5()), modifier(align_start(), wrap()))) {
                    markup("section", style_attr("flex:1;min-width:440px")) {
                        column_box(gap_space("22px")) {
                            markup("span", style_attr(badge_mod())) {
                                "Editorial Theme";
                            };
                            markup("h1", style_attr("margin:0;max-width:620px;font-size:82px;line-height:0.9;letter-spacing:-0.06em;color:#f8fafc")) {
                                "Make web pages that feel art-directed before the polish pass begins.";
                            };
                            markup("p", style_attr(lead_copy_mod())) {
                                "Spear should not trap you in templates. It should give you a design system that already has rhythm, restraint, and taste.";
                            };
                            action_bar("#start", "Open the starter", "#system", "See the system");
                        };
                    };
                    markup("section", style_attr("flex:1;min-width:320px")) {
                        box(modifier(dark_panel_mod(), max_width("420px"))) {
                            column_box(gap_space(space_3())) {
                                row_box(modifier(justify_between(), align_center())) {
                                    markup("span", style_attr(eyebrow_mod())) {
                                        "Current note";
                                    };
                                    label_text(foreground(tone_inverse_muted()), "editorial.web");
                                };
                                markup("p", style_attr("margin:0;font-size:28px;line-height:1.08;color:#f8fafc")) {
                                    "Less dashboard noise. More composition, hierarchy, and negative space.";
                                };
                                markup("p", style_attr(body_copy_inverse_mod())) {
                                    "The point is not raw CSS avoidance. The point is being able to move quickly without flattening the visual result.";
                                };
                            };
                        };
                    };
                };
            };
        });
        centered(column_box(modifier(gap_space(space_4()), max_width("1120px"))) {
            split(
                box(paper_panel_mod()) {
                    column_box(gap_space(space_3())) {
                        markup("span", style_attr(eyebrow_mod())) {
                            "Story";
                        };
                        markup("h2", style_attr(section_title_mod())) {
                            "A calmer sample page";
                        };
                        markup("p", style_attr(body_copy_mod())) {
                            "This example intentionally drops the overloaded product-demo feel. It reads more like a launch brief or studio note than a dashboard dump.";
                        };
                        feature_triptych(
                            "Surface", "Use fewer layers", "Wide rhythm, strong typography, and clean surfaces do more than stacking ten flashy widgets.",
                            "Flow", "Keep it readable", "A page should still scan well if someone only reads the headings and notices the shape.",
                            "Control", "Leave room to steer", "The system should guide the result without freezing you into one branded layout."
                        );
                    };
                },
                box(paper_panel_mod()) {
                    column_box(gap_space(space_3())) {
                        markup("span", style_attr(eyebrow_mod())) {
                            "Signal";
                        };
                        metric_bars(
                            "What the language should optimize for",
                            "A simple composition should already hold together visually.",
                            pack("Readability", "Default taste", "Freedom to remix"),
                            pack(93, 88, 91),
                            pack(tone_ink(), tone_brand(), tone_warm())
                        );
                    };
                }
            );
            row_box(modifier(gap_space(space_3()), wrap())) {
                markup("section", style_attr("flex:1;min-width:340px")) {
                    trend_line_card(
                        "Draft quality",
                        "The first pass should already look publishable.",
                        "+24%",
                        tone_ink(),
                        pack(38, 44, 52, 61, 74, 88)
                    );
                };
                markup("section", style_attr("flex:1;min-width:340px")) {
                    activity_feed(
                        "Studio notes",
                        "A more editorial page still needs useful supporting information.",
                        join(
                            join(
                                activity_item("09:20", "Theme set", "Switched the page from glossy product energy to a calmer studio tone."),
                                activity_item("11:10", "Spacing tuned", "Reduced visual clutter and let the typography carry more weight.")
                            ),
                            activity_item("14:40", "Form surface added", "The call-to-action form now feels like part of the composition, not an afterthought.")
                        )
                    );
                };
            };
            inspector_panel(
                "Start a page",
                "Real controls should still look intentional inside the same visual system.",
                action_bar("#preview", "Preview structure", "#tokens", "Review tokens"),
                split(
                    form_card(
                        "Request a build",
                        "This is a real HTML form surface, not a fake shell.",
                        column_box(gap_space(space_2())) {
                            text_input("Project", "project", "Atelier", "Project name");
                            email_input("Owner", "owner", "", "editor@studio.dev");
                            select_field("Mode", "mode", join(
                                join(
                                    select_option("launch", "Launch page", "yes"),
                                    select_option("notes", "Studio notes", "no")
                                ),
                                select_option("case", "Case study", "no")
                            ));
                            text_area("Direction", "direction", "", "Describe the mood, hierarchy, and outcome.", "5");
                        },
                        "/submit",
                        "Send brief"
                    ),
                    column_box(gap_space(space_3())) {
                        route_notice(
                            "Review queue",
                            "Use notice blocks for decisions and next actions instead of loud banner spam.",
                            "#queue",
                            "Open queue",
                            tone_brand()
                        );
                        if (issue_count > 0) {
                            status_banner(
                                "Open design notes",
                                "Three notes still need a final pass before publishing.",
                                tone_warm()
                            );
                        } else {
                            status_banner(
                                "Ready to publish",
                                "The page is clean, approved, and ready to ship.",
                                "#16a34a"
                            );
                        };
                        quote_block("A design system should raise the floor without flattening the result.");
                    }
                )
            );
            launch_banner(
                "Write less layout glue.",
                "Let the page start from a strong editorial baseline, then push it where the project needs to go.",
                "#start",
                "Build with Spear"
            );
        });
    };

    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
