function view shell_page(string title, string body) {
    return page(title) {
        markup("main", style_attr("min-height:100vh;background:radial-gradient(circle at top left, #2347a8 0, rgba(35,71,168,0.18) 22%, transparent 45%),radial-gradient(circle at 85% 15%, #ff8a3d 0, rgba(255,138,61,0.16) 18%, transparent 40%),linear-gradient(180deg, #08111f 0%, #0d1728 52%, #f3efe6 52%, #f4f1ea 100%);color:#ecf3ff")) {
            body;
        };
    };
}

function view page_wrap(string body) {
    return markup("section", style_attr("max-width:1160px;margin:0 auto;padding:28px 24px 60px 24px")) {
        body;
    };
}

function view brand_bar() {
    return row_box(modifier(justify_between(), modifier(align_center(), padding_y("10px")))) {
        row_box(modifier(align_center(), gap_space("14px"))) {
            markup("div", style_attr("width:46px;height:46px;border-radius:16px;background:linear-gradient(135deg,#4f8cff,#87b4ff);box-shadow:0 18px 40px rgba(79,140,255,0.35)"), "");
            column_box(gap_space("4px")) {
                markup("strong", style_attr("font-size:19px;letter-spacing:0.02em")) {
                    "Spear Web";
                };
                label_text(foreground("rgba(236,243,255,0.72)"), "Readable blocks. Sharp output.");
            };
        };
        row_box(gap_space("10px")) {
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", "#vision", "Vision");
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", "#components", "Components");
            button_link("display:inline-block;padding:10px 14px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", "#launch", "Launch");
        };
    };
}

function view stat_chip(string label, string value) {
    return box(modifier(padding("16px"), modifier(corner_radius("20px"), modifier(background("rgba(255,255,255,0.05)"), border("1px solid rgba(255,255,255,0.10)"))))) {
        column_box(gap_space("8px")) {
            label_text(foreground("rgba(236,243,255,0.70)"), label);
            markup("strong", style_attr("font-size:28px;line-height:1.05")) {
                escape(value);
            };
        };
    };
}

function view signal_card(string eyebrow, string title, string body) {
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

function view code_showcase() {
    return box(modifier(padding("24px"), modifier(corner_radius("28px"), modifier(background("#0c1322"), shadow("0 24px 70px rgba(2,6,23,0.45)"))))) {
        column_box(gap_space("18px")) {
            row_box(modifier(justify_between(), align_center())) {
                row_box(gap_space("8px")) {
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#ff6b6b"), "");
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#f7b955"), "");
                    markup("span", style_attr("width:12px;height:12px;border-radius:999px;background:#51cf66"), "");
                };
                label_text(foreground("rgba(236,243,255,0.62)"), "examples/web.sp");
            };
            markup("pre", style_attr("margin:0;color:#dbeafe;font-size:15px;line-height:1.65;white-space:pre-wrap")) {
                escape("run {\n    string html = shell_page(\"Spear\") {\n        page_wrap(hero_stage());\n    };\n    write(\"build/spear-ui.html\", html);\n}");
            };
        };
    };
}

function view stage_left() {
    return column_box(gap_space("22px")) {
        badge("Visual Example");
        markup("h1", style_attr("margin:0;font-size:78px;line-height:0.94;letter-spacing:-0.05em;max-width:580px")) {
            "Spear can ship a page that looks expensive.";
        };
        markup("p", style_attr("margin:0;max-width:560px;font-size:22px;line-height:1.55;color:rgba(236,243,255,0.76)")) {
            "Readable functions, clear modifiers, no brittle template soup. This example is built to sell the language in one screen.";
        };
        row_box(gap_space("12px")) {
            button_link("display:inline-block;padding:14px 18px;border-radius:999px;background:#ecf3ff;color:#08111f;text-decoration:none;font-weight:700;box-shadow:0 18px 40px rgba(236,243,255,0.24)", "#launch", "Open Launch Notes");
            button_link("display:inline-block;padding:14px 18px;border-radius:999px;border:1px solid rgba(255,255,255,0.16);background:rgba(255,255,255,0.04);color:#ecf3ff;text-decoration:none", "#components", "Inspect Components");
        };
        row_box(modifier(gap_space("14px"), wrap())) {
            stat_chip("Build Time", "< 1 min");
            stat_chip("Code Feel", "plain words");
            stat_chip("Output", "HTML");
        };
    };
}

function view stage_right() {
    return column_box(gap_space("20px")) {
        code_showcase();
        row_box(modifier(gap_space("16px"), wrap())) {
            signal_card("Readable", "Functions that read like intent", "Use `shell_page`, `page_wrap`, and `signal_card` instead of wrestling raw HTML.");
            signal_card("Safe", "Escaped text by default", "The runtime protects common output paths so simple apps stay safe without ceremony.");
        };
    };
}

function view hero_stage() {
    return box(modifier(padding("34px"), modifier(corner_radius("34px"), modifier(background("linear-gradient(180deg, rgba(255,255,255,0.04), rgba(255,255,255,0.02))"), border("1px solid rgba(255,255,255,0.08)"))))) {
        column_box(gap_space("30px")) {
            brand_bar();
            row_box(modifier(gap_space("30px"), modifier(align_start(), wrap()))) {
                markup("section", style_attr("flex:1;min-width:420px")) {
                    stage_left();
                };
                markup("section", style_attr("flex:1;min-width:360px")) {
                    stage_right();
                };
            };
        };
    };
}

function view feature_band() {
    return box(modifier(padding("28px"), modifier(corner_radius("30px"), modifier(background("#f9f6ef"), shadow("0 18px 50px rgba(15,23,42,0.08)"))))) {
        column_box(gap_space("20px")) {
            markup("div", id_name("components")) {
                intro("Design blocks with intent", "The standard web helpers can create strong layouts without hiding the HTML underneath.");
            };
            row_box(modifier(gap_space("18px"), wrap())) {
                markup("section", style_attr("flex:1;min-width:260px")) {
                    signal_card("Cards", "Rich presentation blocks", "Mix gradients, shadows, pills, and metrics while keeping each section readable.");
                };
                markup("section", style_attr("flex:1;min-width:260px")) {
                    signal_card("Compose", "Modifiers stay predictable", "Padding, gap, radius, color, and width helpers layer cleanly instead of turning into CSS chaos.");
                };
                markup("section", style_attr("flex:1;min-width:260px")) {
                    signal_card("Shipping", "Export one finished file", "Use `write(...)` and you have something you can preview, host, or hand off immediately.");
                };
            };
        };
    };
}

function view launch_strip() {
    return box(modifier(padding("28px"), modifier(corner_radius("30px"), modifier(background("#111827"), shadow("0 24px 60px rgba(15,23,42,0.18)"))))) {
        row_box(modifier(justify_between(), modifier(align_center(), wrap()))) {
            column_box(gap_space("12px")) {
                markup("div", id_name("launch")) {
                    markup("h2", style_attr("margin:0;font-size:34px;line-height:1.05;color:#f8fafc")) {
                        "Launch a polished page with Spear.";
                    };
                };
                label_text(foreground("rgba(248,250,252,0.72)"), "Start from readable blocks, keep the output safe, and ship without drowning in boilerplate.");
            };
            button_link("display:inline-block;padding:16px 20px;border-radius:999px;background:#ff8a3d;color:#111827;text-decoration:none;font-weight:700;box-shadow:0 18px 40px rgba(255,138,61,0.30)", "/start", "Build something now");
        };
    };
}

run {
    const title = "Spear Web";
    string html = shell_page(title, page_wrap(column_box(gap_space("26px")) {
            hero_stage();
            feature_band();
            launch_strip();
        }));
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
