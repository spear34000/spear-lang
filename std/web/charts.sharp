package std;
module webcharts;

function text trend_path_body(numlist values, text tone) {
    variable peak = chart_peak(values);
    variable i = 0;
    sharp {
        variable out = "";
        while (i < count(values)) {
            number left = i * 88;
            number width = 68;
            number height = (at(values, i) * 84) / peak;
            number top = 92 - height;
            out = join(out, markup("div", style_attr(
                modifier(
                    style_rule("position", "absolute"),
                    modifier(
                        style_rule("left", join(text(left), "px")),
                        modifier(
                            style_rule("top", join(text(top), "px")),
                            modifier(
                                style_rule("width", join(text(width), "px")),
                                modifier(
                                    style_rule("height", join(text(height), "px")),
                                    modifier(corner_radius("20px 20px 6px 6px"), background(tone))
                                )
                            )
                        )
                    )
                )
            ), ""));
            i = i + 1;
        }
        return out;
    }
}

function text timeline_feed_body(textlist stamps, textlist titles, textlist notes) {
    variable i = 0;
    sharp {
        variable out = "";
        while (i < count(stamps)) {
            out = join(out, markup("section", style_attr("display:flex;gap:14px;align-items:flex-start")) {
                markup("div", style_attr("padding-top:4px")) {
                    markup("span", style_attr("display:inline-block;width:12px;height:12px;border-radius:999px;background:#4f8cff;box-shadow:0 0 0 6px rgba(79,140,255,0.14)"), "");
                };
                column_box(gap_space("6px")) {
                    markup("p", style_attr(caption_mod())) {
                        escape(at(stamps, i));
                    };
                    markup("h4", style_attr(card_title_mod())) {
                        escape(at(titles, i));
                    };
                    markup("p", style_attr(body_copy_mod())) {
                        escape(at(notes, i));
                    };
                };
            });
            i = i + 1;
        }
        return out;
    }
}

function view chart_card(text title, text body) {
    return box(modifier(padding("22px"), modifier(corner_radius("26px"), modifier(background("#fffdf9"), shadow("0 18px 50px rgba(15,23,42,0.10)"))))) {
        column_box(gap_space("12px")) {
            markup("h3", style_attr("margin:0;font-size:24px;line-height:1.15;color:#0f172a")) {
                escape(title);
            };
            label_text(foreground("#475569"), body);
        };
    };
}

function view signal_bar(text label, text value, text width, text tone) {
    return column_box(gap_space("8px")) {
        row_box(modifier(justify_between(), align_center())) {
            label_text(foreground(tone_ink()), label);
            label_text(foreground(tone_muted()), value);
        };
        markup("div", style_attr(metric_track_mod())) {
            markup("div", style_attr(metric_fill_mod(tone, width))) {
                "";
            };
        };
    };
}

function view comparison_chart(text title, text body, text a_label, text a_value, text a_width, text a_tone, text b_label, text b_value, text b_width, text b_tone, text c_label, text c_value, text c_width, text c_tone) {
    return box(modifier(padding("24px"), modifier(corner_radius("28px"), modifier(background("#fffdf9"), shadow("0 18px 50px rgba(15,23,42,0.10)"))))) {
        column_box(gap_space("18px")) {
            intro(title, body);
            signal_bar(a_label, a_value, a_width, a_tone);
            signal_bar(b_label, b_value, b_width, b_tone);
            signal_bar(c_label, c_value, c_width, c_tone);
        };
    };
}

function num chart_peak(numlist values) {
    guard(count(values) > 0, "chart_peak requires at least one value");
    variable best = at(values, 0);
    variable i = 1;
    while (i < count(values)) {
        if (at(values, i) > best) {
            best = at(values, i);
        }
        i = i + 1;
    }
    return best;
}

function text metric_bars_body(textlist labels, numlist values, textlist tones) {
    variable peak = chart_peak(values);
    variable i = 0;
    sharp {
        variable out = "";
        while (i < count(labels)) {
            out = join(out, signal_bar(
                at(labels, i),
                join(text(at(values, i)), "%"),
                join(text((at(values, i) * 100) / peak), "%"),
                at(tones, i)
            ));
            i = i + 1;
        }
        return out;
    }
}

function view metric_bars(text title, text body, textlist labels, numlist values, textlist tones) {
    return box(paper_panel_mod()) {
        column_box(gap_space("18px")) {
            intro(title, body);
            metric_bars_body(labels, values, tones);
        };
    };
}

function view delta_chip(text label, text delta, text tone) {
    return markup("span", style_attr(
        modifier(
            padding_xy("8px", "12px"),
            modifier(
                corner_radius(radius_pill()),
                modifier(background("rgba(15,23,42,0.05)"), modifier(foreground(tone), font_weight("700")))
            )
        )
    )) {
        escape(join(join(label, " "), delta));
    };
}

function view trend_line_card(text title, text body, text delta, text tone, numlist values) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
                column_box(gap_space(space_1())) {
                    markup("h3", style_attr(card_title_mod())) {
                        escape(title);
                    };
                    markup("p", style_attr(body_copy_mod())) {
                        escape(body);
                    };
                };
                delta_chip("Trend", delta, tone);
            };
            markup("div", style_attr("position:relative;height:96px;border-radius:20px;background:linear-gradient(180deg,#f8fafc,#eef3f8);overflow:hidden")) {
                trend_path_body(values, tone);
            };
        };
    };
}

function view timeline_feed(text title, text body, textlist stamps, textlist titles, textlist notes) {
    return box(paper_panel_mod()) {
        column_box(gap_space(space_3())) {
            column_box(gap_space(space_1())) {
                markup("h3", style_attr(card_title_mod())) {
                    escape(title);
                };
                markup("p", style_attr(body_copy_mod())) {
                    escape(body);
                };
            };
            column_box(gap_space(space_3())) {
                timeline_feed_body(stamps, titles, notes);
            };
        };
    };
}
