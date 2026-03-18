package std;
module webcharts;

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
            label_text(foreground("#0f172a"), label);
            label_text(foreground("#475569"), value);
        };
        markup("div", style_attr("height:12px;border-radius:999px;background:#e2e8f0;overflow:hidden")) {
            markup("div", style_attr(join(join("height:12px;border-radius:999px;background:", tone), join(";width:", width)))) {
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
    return box(modifier(padding("24px"), modifier(corner_radius("28px"), modifier(background("#fffdf9"), shadow("0 18px 50px rgba(15,23,42,0.10)"))))) {
        column_box(gap_space("18px")) {
            intro(title, body);
            metric_bars_body(labels, values, tones);
        };
    };
}
