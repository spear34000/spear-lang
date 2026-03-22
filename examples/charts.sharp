run {
    const title = "Spear Signals";

    string html = page(title) {
        markup("main", style_attr("min-height:100vh;background:#f4f1ea;color:#141414")) {
            markup("section", style_attr("max-width:1180px;margin:0 auto;padding:28px 22px 72px 22px")) {
                column_box(gap_space("20px")) {
                    row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                        row_box(gap_space("12px")) {
                            markup("span", style_attr("display:inline-block;width:14px;height:14px;background:#141414"), "");
                            markup("strong", style_attr("font-size:15px;letter-spacing:0.14em;text-transform:uppercase")) {
                                "Spear Signals";
                            };
                        };
                        row_box(gap_space("10px")) {
                            state_link("#summary", "Summary", "yes");
                            state_link("#trends", "Trends", "no");
                            state_link("#timeline", "Timeline", "no");
                            state_link("#table", "Table", "no");
                        };
                    };
                    markup("section", style_attr("display:grid;grid-template-columns:minmax(0,1.1fr) minmax(320px,0.9fr);gap:18px")) {
                        box(style_attr("padding:38px;background:#141414;color:#f8f4ea;min-height:420px;display:flex;flex-direction:column;justify-content:space-between")) {
                            column_box(gap_space("22px")) {
                                markup("span", style_attr("display:inline-block;padding:6px 10px;border:1px solid rgba(255,255,255,0.18);font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#d8cfbf")) {
                                    "Quarterly Review";
                                };
                                markup("h1", style_attr("max-width:720px;font-size:88px;line-height:0.88;letter-spacing:-0.07em")) {
                                    "A chart-first example with a calmer review-board layout.";
                                };
                                markup("p", style_attr("max-width:540px;font-size:22px;line-height:1.55;color:#d8d1c4")) {
                                    "This sample is not a landing page. It is a signal board for a product review, built around trends, comparisons, and release movement.";
                                };
                            };
                            action_bar("#trends", "Open trends", "#timeline", "Read timeline");
                        };
                        dashboard_grid(
                            stat_panel("Weekly reach", "184k", "Audience stayed strong through the release window."),
                            stat_panel("Activation", "5.2%", "Activation moved after simplifying the entry flow."),
                            stat_panel("Retention", "71%", "Cohorts held better than the previous quarter."),
                            stat_panel("Risk", "low", "No major blockers remain in the current review cycle.")
                        );
                    };
                    markup("section", style_attr("display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px")) {
                        trend_line_card(
                            "Activation trend",
                            "A steady rise after the onboarding surface was simplified.",
                            "+18%",
                            "#2f6fed",
                            pack(31, 34, 40, 49, 58, 66)
                        );
                        trend_line_card(
                            "Retention trend",
                            "Retention improved with a slower but more stable slope.",
                            "+9%",
                            "#b76536",
                            pack(52, 54, 55, 58, 60, 61)
                        );
                    };
                    split(
                        comparison_chart(
                            "Channel mix",
                            "A simple three-way comparison is often more useful than decorative complexity.",
                            "Product-led", "68%%", "68%%", "#141414",
                            "Referrals", "44%%", "44%%", "#b76536",
                            "Outbound", "29%%", "29%%", "#6e7a8f"
                        ),
                        metric_bars(
                            "Quality signals",
                            "These bars make the page feel like a review board rather than a promo site.",
                            pack("Clarity", "Momentum", "Confidence"),
                            pack(93, 87, 90),
                            pack("#141414", "#2f6fed", "#b76536")
                        )
                    );
                    split(
                        timeline_feed(
                            "Release timeline",
                            "Timeline blocks let the board explain why the charts changed.",
                            pack("Jan 08", "Jan 19", "Feb 02"),
                            pack("Entry flow simplified", "Navigation cleaned", "Review checklist shipped"),
                            pack(
                                "Removed duplicate choices on the first screen.",
                                "Cut visual clutter and grouped actions by intent.",
                                "Gave reviewers a calmer, more structured approval flow."
                            )
                        ),
                        data_table(
                            "Segment table",
                            "A compact table grounds the visuals in something operational.",
                            join(
                                join(table_header("Segment"), table_header("Change")),
                                table_header("State")
                            ),
                            join(
                                join(
                                    table_row("New teams", "+14%%", "Healthy"),
                                    table_row("Returning teams", "+8%%", "Stable")
                                ),
                                table_row("Enterprise", "+5%%", "Watching")
                            )
                        )
                    );
                    box(style_attr("padding:30px;background:#ebe3d5;border:1px solid #d7ccbb")) {
                        row_box(modifier(justify_between(), modifier(align_end(), wrap()))) {
                            column_box(gap_space("8px")) {
                                markup("span", style_attr("font-size:12px;letter-spacing:0.08em;text-transform:uppercase;color:#6f6658")) {
                                    "Readout";
                                };
                                markup("h2", style_attr("font-size:48px;line-height:0.94;letter-spacing:-0.06em;color:#141414")) {
                                    "Build review boards, not just landing pages.";
                                };
                            };
                            button_link(action_warm_mod(), "#summary", "Build with Spear");
                        };
                    };
                };
            };
        };
    };

    guard(size(html) > 0, "body must not be empty");
    write("build/signals.html", html);
    print(html);
}
