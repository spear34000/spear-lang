run {
    const title = "Spear Journal";

    string html = page(title) {
        markup("main", style_attr("min-height:100vh;background:#f3efe7;color:#161616")) {
            markup("section", style_attr("max-width:1120px;margin:0 auto;padding:40px 24px 64px 24px")) {
                column_box(gap_space("28px")) {
                    row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                        markup("div", style_attr("display:inline-block;padding:8px 12px;border:1px solid #d7d0c3;background:#fbf8f2")) {
                            markup("strong", style_attr("font-size:16px;letter-spacing:0.08em;text-transform:uppercase")) {
                                "Spear";
                            };
                        };
                        row_box(gap_space("12px")) {
                            state_link("#approach", "Approach", "yes");
                            state_link("#sample", "Sample", "no");
                            state_link("#start", "Start", "no");
                            state_link("#notes", "Notes", "no");
                        };
                    };
                    split(
                        box(style_attr("padding:36px;background:#111111;color:#f7f3ea;border:1px solid #1f1f1f")) {
                            column_box(gap_space("22px")) {
                                markup("span", style_attr("display:inline-block;padding:5px 10px;border:1px solid rgba(255,255,255,0.18);font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#d8d1c4")) {
                                    "Calm Web Example";
                                };
                                markup("h1", style_attr("max-width:640px;font-size:78px;line-height:0.92;letter-spacing:-0.05em")) {
                                    "A quieter Spear page with less product-demo noise.";
                                };
                                markup("p", style_attr("max-width:540px;font-size:22px;line-height:1.55;color:#d5cec2")) {
                                    "This example aims for something more timeless: strong type, clean blocks, restrained color, and no fake dashboard clutter.";
                                };
                                action_bar("#start", "Open starter", "#sample", "See sample block");
                            };
                        },
                        column_box(gap_space("18px")) {
                            box(style_attr("padding:24px;background:#fbf8f2;border:1px solid #d7d0c3")) {
                                column_box(gap_space("10px")) {
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                        "Direction";
                                    };
                                    markup("p", style_attr("font-size:30px;line-height:1.08;color:#161616")) {
                                        "Default pages should look composed before the final polish pass.";
                                    };
                                };
                            };
                            box(style_attr("padding:24px;background:#e5ded1;border:1px solid #d0c6b7")) {
                                column_box(gap_space("8px")) {
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                        "Goal";
                                    };
                                    markup("p", style_attr("font-size:18px;line-height:1.6;color:#332d25")) {
                                        "Keep the visual floor high without boxing users into one loud aesthetic.";
                                    };
                                };
                            };
                        }
                    );
                    split(
                        box(style_attr("padding:28px;background:#fbf8f2;border:1px solid #d7d0c3")) {
                            column_box(gap_space("18px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                    "Approach";
                                };
                                markup("h2", style_attr("font-size:34px;line-height:1.02;color:#161616")) {
                                    "Use fewer, stronger moves.";
                                };
                                bullet_list(join(
                                    join(
                                        item("Let typography carry more of the page."),
                                        item("Use wide spacing instead of stacking more widgets.")
                                    ),
                                    item("Keep accent color rare enough that it still means something.")
                                ));
                            };
                        },
                        box(style_attr("padding:28px;background:#fbf8f2;border:1px solid #d7d0c3")) {
                            column_box(gap_space("18px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                    "Sample";
                                };
                                metric_bars(
                                    "What should improve",
                                    "The page should read better and look less eager to impress.",
                                    pack("Readability", "Restraint", "Flexibility"),
                                    pack(94, 89, 91),
                                    pack("#161616", "#8c6b3b", "#5a677d")
                                );
                            };
                        }
                    );
                    inspector_panel(
                        "Request a page",
                        "Real controls stay, but the composition stays calm.",
                        action_bar("#preview", "Preview blocks", "#tokens", "Review tokens"),
                        split(
                            form_card(
                                "Start from a brief",
                                "Use a real HTML form surface with simple, compatible styling.",
                                column_box(gap_space(space_2())) {
                                    text_input("Project", "project", "Spear Journal", "Project name");
                                    email_input("Owner", "owner", "", "editor@studio.dev");
                                    text_area("Intent", "intent", "", "Describe the tone, layout, and audience.", "5");
                                },
                                "/submit",
                                "Send brief"
                            ),
                            column_box(gap_space("16px")) {
                                route_notice(
                                    "Compatibility",
                                    "This page avoids flashy effects and leans on basic, widely supported HTML and CSS.",
                                    "#notes",
                                    "Read notes",
                                    "#5a677d"
                                );
                                quote_block("Good default design feels deliberate, not overproduced.");
                            }
                        )
                    );
                    box(style_attr("padding:32px;background:#111111;color:#f7f3ea;border:1px solid #1f1f1f")) {
                        row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                            column_box(gap_space("10px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#c9c1b3")) {
                                    "Start";
                                };
                                markup("h2", style_attr("font-size:38px;line-height:0.98;color:#f7f3ea")) {
                                    "Build something clean first.";
                                };
                            };
                            button_link(action_warm_mod(), "#start", "Build with Spear");
                        };
                    };
                };
            };
        };
    };

    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
