run {
    const title = "Spear Canvas";

    string html = page(title) {
        markup("main", style_attr("min-height:100vh;background:#ebe6dc;color:#121212")) {
            markup("section", style_attr("max-width:1180px;margin:0 auto;padding:28px 22px 72px 22px")) {
                column_box(gap_space("22px")) {
                    row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                        row_box(gap_space("12px")) {
                            markup("span", style_attr("display:inline-block;width:14px;height:14px;background:#121212"), "");
                            markup("strong", style_attr("font-size:15px;letter-spacing:0.14em;text-transform:uppercase")) {
                                "Spear Canvas";
                            };
                        };
                        row_box(gap_space("10px")) {
                            state_link("#index", "Index", "yes");
                            state_link("#system", "System", "no");
                            state_link("#brief", "Brief", "no");
                            state_link("#launch", "Launch", "no");
                        };
                    };
                    markup("section", style_attr("display:grid;grid-template-columns:minmax(0,1.35fr) minmax(320px,0.85fr);gap:18px")) {
                        box(style_attr("padding:42px 38px;background:#121212;color:#f6f1e8;min-height:520px;display:flex;flex-direction:column;justify-content:space-between")) {
                            column_box(gap_space("24px")) {
                                markup("span", style_attr("display:inline-block;padding:6px 10px;border:1px solid rgba(255,255,255,0.18);font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#d6cfbf")) {
                                    "Web Direction";
                                };
                                markup("h1", style_attr("max-width:760px;font-size:96px;line-height:0.88;letter-spacing:-0.07em")) {
                                    "Design pages that feel composed before the styling rabbit hole starts.";
                                };
                                markup("p", style_attr("max-width:560px;font-size:22px;line-height:1.55;color:#d8d1c4")) {
                                    "Spear should make strong layout, calm rhythm, and visual intent easier to reach without drowning the page in ornamental UI noise.";
                                };
                            };
                            row_box(modifier(justify_between(), modifier(align_end(), wrap()))) {
                                column_box(gap_space("8px")) {
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#bdb49f")) {
                                        "Principle";
                                    };
                                    markup("p", style_attr("max-width:300px;font-size:18px;line-height:1.5;color:#f6f1e8")) {
                                        "Use fewer surfaces, bigger type, harder alignment, and one real accent.";
                                    };
                                };
                                action_bar("#brief", "Open brief", "#system", "See system");
                            };
                        };
                        column_box(gap_space("18px")) {
                            box(style_attr("padding:24px;background:#f8f4ec;border:1px solid #d4ccbe;min-height:250px")) {
                                column_box(modifier(gap_space("12px"), justify_between())) {
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#7b7263")) {
                                        "Studio Note";
                                    };
                                    markup("p", style_attr("font-size:34px;line-height:1.02;letter-spacing:-0.04em;color:#121212")) {
                                        "Less widget theater. More hierarchy.";
                                    };
                                };
                            };
                            box(style_attr("padding:24px;background:#c96a3b;color:#fff7ef;min-height:250px")) {
                                column_box(modifier(gap_space("12px"), justify_between())) {
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:rgba(255,247,239,0.78)")) {
                                        "Accent";
                                    };
                                    markup("p", style_attr("font-size:30px;line-height:1.04;letter-spacing:-0.04em")) {
                                        "Use color where it changes the page, not everywhere at once.";
                                    };
                                };
                            };
                        };
                    };
                    markup("section", style_attr("display:grid;grid-template-columns:220px minmax(0,1fr);gap:18px")) {
                        box(style_attr("padding:22px;background:#ddd6c8;border:1px solid #cfc5b4")) {
                            column_box(gap_space("14px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                    "Contents";
                                };
                                column_box(gap_space("10px")) {
                                    state_link("#01", "01 Intro", "yes");
                                    state_link("#02", "02 Quality", "no");
                                    state_link("#03", "03 Brief", "no");
                                    state_link("#04", "04 Ship", "no");
                                };
                            };
                        };
                        box(style_attr("padding:28px;background:#f8f4ec;border:1px solid #d4ccbe")) {
                            column_box(gap_space("26px")) {
                                row_box(modifier(justify_between(), modifier(align_end(), wrap()))) {
                                    markup("h2", style_attr("font-size:42px;line-height:0.96;letter-spacing:-0.05em;color:#121212")) {
                                        "A stronger default page";
                                    };
                                    markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                        "Issue 01";
                                    };
                                };
                                split(
                                    column_box(gap_space("16px")) {
                                        markup("p", style_attr("font-size:18px;line-height:1.72;color:#3a342d")) {
                                            "This version trades the noisy product-demo look for a tighter editorial layout. The blocks are bigger, the text has more weight, and each section gets more room to breathe.";
                                        };
                                        markup("p", style_attr("font-size:18px;line-height:1.72;color:#3a342d")) {
                                            "It is still built out of the same language primitives, but the page now behaves more like a designed spread than a component dump.";
                                        };
                                    },
                                    metric_bars(
                                        "What should be easy",
                                        "A small amount of code should still land with confidence.",
                                        pack("Rhythm", "Clarity", "Freedom"),
                                        pack(92, 95, 89),
                                        pack("#121212", "#c96a3b", "#6f7d8f")
                                    )
                                );
                            };
                        };
                    };
                    inspector_panel(
                        "Project brief",
                        "Real form controls, but with a more restrained presentation.",
                        action_bar("#preview", "Preview layout", "#notes", "Read notes"),
                        split(
                            form_card(
                                "Start with intent",
                                "Good defaults matter most when the page is still rough.",
                                column_box(gap_space(space_2())) {
                                    text_input("Project", "project", "Spear Canvas", "Project name");
                                    email_input("Owner", "owner", "", "name@studio.dev");
                                    text_area("Direction", "direction", "", "Describe tone, audience, and visual tension.", "5");
                                },
                                "/submit",
                                "Send brief"
                            ),
                            column_box(gap_space("16px")) {
                                route_notice(
                                    "Compatibility first",
                                    "This sample sticks to simple grid, flex, borders, typography, and flat color so it behaves well across browsers.",
                                    "#compat",
                                    "Read compatibility note",
                                    "#6f7d8f"
                                );
                                box(style_attr("padding:24px;background:#121212;color:#f6f1e8")) {
                                    column_box(gap_space("10px")) {
                                        markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#bdb49f")) {
                                            "Rule";
                                        };
                                        markup("p", style_attr("font-size:28px;line-height:1.08")) {
                                            "Make the page memorable through composition before adding visual effects.";
                                        };
                                    };
                                };
                            }
                        )
                    );
                    box(style_attr("padding:30px;background:#f8f4ec;border:1px solid #d4ccbe")) {
                        row_box(modifier(justify_between(), modifier(align_end(), wrap()))) {
                            column_box(gap_space("8px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                    "Launch";
                                };
                                markup("h2", style_attr("font-size:48px;line-height:0.94;letter-spacing:-0.06em;color:#121212")) {
                                    "Build something with a point of view.";
                                };
                            };
                            button_link(action_warm_mod(), "#launch", "Build with Spear");
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
