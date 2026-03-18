package std;
module webpresets;

function view spectacle_page(text title, text body) {
    return page(title) {
        markup("main", style_attr("min-height:100vh;background:radial-gradient(circle at top left, #2347a8 0, rgba(35,71,168,0.18) 22%, transparent 45%),radial-gradient(circle at 85% 15%, #ff8a3d 0, rgba(255,138,61,0.16) 18%, transparent 40%),linear-gradient(180deg, #08111f 0%, #0d1728 52%, #f3efe6 52%, #f4f1ea 100%);color:#ecf3ff")) {
            body;
        };
    };
}

function view showcase_shell(text body) {
    return markup("section", style_attr("max-width:1160px;margin:0 auto;padding:28px 24px 60px 24px")) {
        body;
    };
}

function view showcase_stack(text top, text middle, text bottom) {
    return column_box(gap_space("26px")) {
        top;
        middle;
        bottom;
    };
}

function view brand_header(text name, text strap, text href_a, text label_a, text href_b, text label_b, text href_c, text label_c) {
    return row_box(modifier(justify_between(), modifier(align_center(), modifier(padding_y("10px"), wrap())))) {
        row_box(modifier(align_center(), gap_space("14px"))) {
            markup("div", style_attr("width:46px;height:46px;border-radius:16px;background:linear-gradient(135deg,#4f8cff,#87b4ff);box-shadow:0 18px 40px rgba(79,140,255,0.35)"), "");
            column_box(gap_space("4px")) {
                markup("strong", style_attr("font-size:19px;letter-spacing:0.02em")) {
                    escape(name);
                };
                label_text(foreground("rgba(236,243,255,0.72)"), strap);
            };
        };
        row_box(gap_space("10px")) {
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", href_a, label_a);
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", href_b, label_b);
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", href_c, label_c);
        };
    };
}

function view spotlight(text eyebrow, text title, text body, text href_primary, text label_primary, text href_secondary, text label_secondary) {
    return column_box(gap_space("22px")) {
        badge(eyebrow);
        markup("h1", style_attr("margin:0;font-size:78px;line-height:0.94;letter-spacing:-0.05em;max-width:580px")) {
            escape(title);
        };
        markup("p", style_attr("margin:0;max-width:560px;font-size:22px;line-height:1.55;color:rgba(236,243,255,0.76)")) {
            escape(body);
        };
        row_box(gap_space("12px")) {
            button_link("display:inline-block;padding:14px 18px;border-radius:999px;background:#ecf3ff;color:#08111f;text-decoration:none;font-weight:700;box-shadow:0 18px 40px rgba(236,243,255,0.24)", href_primary, label_primary);
            button_link("display:inline-block;padding:14px 18px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", href_secondary, label_secondary);
        };
    };
}

function view prestige_stat(text label, text value) {
    return box(modifier(padding("16px"), modifier(corner_radius("20px"), modifier(background("rgba(255,255,255,0.05)"), border("1px solid rgba(255,255,255,0.10)"))))) {
        column_box(gap_space("8px")) {
            label_text(foreground("rgba(236,243,255,0.70)"), label);
            markup("strong", style_attr("font-size:28px;line-height:1.05")) {
                escape(value);
            };
        };
    };
}

function view prestige_stats(text a_label, text a_value, text b_label, text b_value, text c_label, text c_value) {
    return row_box(modifier(gap_space("14px"), wrap())) {
        prestige_stat(a_label, a_value);
        prestige_stat(b_label, b_value);
        prestige_stat(c_label, c_value);
    };
}

function view aura_card(text eyebrow, text title, text body) {
    return box(modifier(padding("22px"), modifier(corner_radius("26px"), modifier(background("#fffdf9"), shadow("0 18px 50px rgba(15,23,42,0.10)"))))) {
        column_box(gap_space("12px")) {
            badge(eyebrow);
            markup("h3", style_attr("margin:0;font-size:24px;line-height:1.15;color:#0f172a")) {
                escape(title);
            };
            label_text(foreground("#475569"), body);
        };
    };
}

function view aura_triptych(text a_eye, text a_title, text a_body, text b_eye, text b_title, text b_body, text c_eye, text c_title, text c_body) {
    return row_box(modifier(gap_space("18px"), wrap())) {
        markup("section", style_attr("flex:1;min-width:260px")) {
            aura_card(a_eye, a_title, a_body);
        };
        markup("section", style_attr("flex:1;min-width:260px")) {
            aura_card(b_eye, b_title, b_body);
        };
        markup("section", style_attr("flex:1;min-width:260px")) {
            aura_card(c_eye, c_title, c_body);
        };
    };
}

function view code_window(text caption, text code) {
    return box(modifier(padding("24px"), modifier(corner_radius("28px"), modifier(background("#0c1322"), shadow("0 24px 70px rgba(2,6,23,0.45)"))))) {
        column_box(gap_space("18px")) {
            row_box(modifier(justify_between(), align_center())) {
                row_box(gap_space("8px")) {
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#ff6b6b"), "");
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#f7b955"), "");
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#51cf66"), "");
                };
                label_text(foreground("rgba(236,243,255,0.62)"), caption);
            };
            markup("pre", style_attr("margin:0;color:#dbeafe;font-size:15px;line-height:1.65;white-space:pre-wrap")) {
                escape(code);
            };
        };
    };
}

function view hero_showcase(text header, text spotlight_block, text showcase_block) {
    return box(modifier(padding("34px"), modifier(corner_radius("34px"), modifier(background("linear-gradient(180deg, rgba(255,255,255,0.04), rgba(255,255,255,0.02))"), border("1px solid rgba(255,255,255,0.08)"))))) {
        column_box(gap_space("30px")) {
            header;
            row_box(modifier(gap_space("30px"), modifier(align_start(), wrap()))) {
                markup("section", style_attr("flex:1;min-width:420px")) {
                    spotlight_block;
                };
                markup("section", style_attr("flex:1;min-width:360px")) {
                    showcase_block;
                };
            };
        };
    };
}

function view feature_stage(text title, text body, text features) {
    return box(modifier(padding("28px"), modifier(corner_radius("30px"), modifier(background("#f9f6ef"), shadow("0 18px 50px rgba(15,23,42,0.08)"))))) {
        column_box(gap_space("20px")) {
            intro(title, body);
            features;
        };
    };
}

function view launch_banner(text title, text body, text href, text label) {
    return box(modifier(padding("28px"), modifier(corner_radius("30px"), modifier(background("#111827"), shadow("0 24px 60px rgba(15,23,42,0.18)"))))) {
        row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
            column_box(gap_space("12px")) {
                markup("h2", style_attr("margin:0;font-size:34px;line-height:1.05;color:#f8fafc")) {
                    escape(title);
                };
                label_text(foreground("rgba(248,250,252,0.72)"), body);
            };
            button_link("display:inline-block;padding:16px 20px;border-radius:999px;background:#ff8a3d;color:#111827;text-decoration:none;font-weight:700;box-shadow:0 18px 40px rgba(255,138,61,0.30)", href, label);
        };
    };
}
