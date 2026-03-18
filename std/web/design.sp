package std;
module webdesign;

function text space_1() {
    return "8px";
}

function text space_2() {
    return "12px";
}

function text space_3() {
    return "18px";
}

function text space_4() {
    return "24px";
}

function text space_5() {
    return "34px";
}

function text space_6() {
    return "48px";
}

function text radius_1() {
    return "14px";
}

function text radius_2() {
    return "20px";
}

function text radius_3() {
    return "28px";
}

function text radius_4() {
    return "36px";
}

function text radius_pill() {
    return "999px";
}

function text tone_ink() {
    return "#0f172a";
}

function text tone_soft_ink() {
    return "#1f2937";
}

function text tone_muted() {
    return "#475569";
}

function text tone_inverse() {
    return "#ecf3ff";
}

function text tone_inverse_muted() {
    return "rgba(236,243,255,0.72)";
}

function text tone_brand() {
    return "#4f8cff";
}

function text tone_brand_soft() {
    return "#87b4ff";
}

function text tone_warm() {
    return "#ff8a3d";
}

function text tone_panel() {
    return "#fffdf9";
}

function text tone_canvas() {
    return "#f6f2ea";
}

function text tone_dark_panel() {
    return "#0c1322";
}

function text line_soft() {
    return "1px solid rgba(15,23,42,0.08)";
}

function text line_inverse_soft() {
    return "1px solid rgba(255,255,255,0.10)";
}

function text elevation_1() {
    return "0 12px 32px rgba(15,23,42,0.08)";
}

function text elevation_2() {
    return "0 18px 50px rgba(15,23,42,0.10)";
}

function text elevation_3() {
    return "0 24px 70px rgba(2,6,23,0.40)";
}

function text shell_frame_mod() {
    return modifier(max_width("1160px"), modifier(center_x(), padding_xy(space_4(), space_4())));
}

function text glass_panel_mod() {
    return modifier(
        padding(space_5()),
        modifier(
            corner_radius(radius_4()),
            modifier(
                background("linear-gradient(180deg, rgba(255,255,255,0.04), rgba(255,255,255,0.02))"),
                modifier(border(line_inverse_soft()), shadow(elevation_2()))
            )
        )
    );
}

function text paper_panel_mod() {
    return modifier(
        padding(space_4()),
        modifier(
            corner_radius(radius_3()),
            modifier(background(tone_panel()), modifier(border(line_soft()), shadow(elevation_2())))
        )
    );
}

function text dark_panel_mod() {
    return modifier(
        padding(space_4()),
        modifier(
            corner_radius(radius_3()),
            modifier(background(tone_dark_panel()), shadow(elevation_3()))
        )
    );
}

function text stat_chip_mod() {
    return modifier(
        padding("16px"),
        modifier(
            corner_radius(radius_2()),
            modifier(background("rgba(255,255,255,0.05)"), border(line_inverse_soft()))
        )
    );
}

function text badge_mod() {
    return modifier(
        padding_xy("4px", "10px"),
        modifier(corner_radius(radius_pill()), modifier(background("#e2e8f0"), font_size("12px")))
    );
}

function text button_primary_mod() {
    return modifier(
        padding_xy("14px", "18px"),
        modifier(
            corner_radius(radius_pill()),
            modifier(
                background(tone_inverse()),
                modifier(
                    foreground("#08111f"),
                    modifier(style_rule("text-decoration", "none"), modifier(font_weight("700"), shadow("0 18px 40px rgba(236,243,255,0.24)")))
                )
            )
        )
    );
}

function text button_secondary_mod() {
    return modifier(
        padding_xy("14px", "18px"),
        modifier(
            corner_radius(radius_pill()),
            modifier(
                border("1px solid rgba(255,255,255,0.16)"),
                modifier(background("rgba(255,255,255,0.04)"), modifier(foreground(tone_inverse()), style_rule("text-decoration", "none")))
            )
        )
    );
}

function text action_warm_mod() {
    return modifier(
        padding_xy("16px", "20px"),
        modifier(
            corner_radius(radius_pill()),
            modifier(
                background(tone_warm()),
                modifier(
                    foreground("#111827"),
                    modifier(style_rule("text-decoration", "none"), modifier(font_weight("700"), shadow("0 18px 40px rgba(255,138,61,0.30)")))
                )
            )
        )
    );
}

function text hero_title_mod() {
    return modifier(
        font_size("78px"),
        modifier(
            style_rule("line-height", "0.94"),
            modifier(style_rule("letter-spacing", "-0.05em"), modifier(max_width("580px"), margin("0")))
        )
    );
}

function text display_title_mod() {
    return modifier(
        margin("0"),
        modifier(font_size("64px"), modifier(style_rule("line-height", "0.98"), style_rule("letter-spacing", "-0.045em")))
    );
}

function text section_title_mod() {
    return modifier(
        margin("0"),
        modifier(font_size("34px"), modifier(style_rule("line-height", "1.05"), foreground(tone_ink())))
    );
}

function text card_title_mod() {
    return modifier(
        margin("0"),
        modifier(font_size("20px"), modifier(style_rule("line-height", "1.2"), foreground(tone_ink())))
    );
}

function text eyebrow_mod() {
    return modifier(
        margin("0"),
        modifier(font_size("12px"), modifier(font_weight("700"), modifier(style_rule("letter-spacing", "0.08em"), foreground(tone_brand()))))
    );
}

function text lead_copy_mod() {
    return modifier(
        margin("0"),
        modifier(max_width("560px"), modifier(font_size("22px"), modifier(style_rule("line-height", "1.55"), foreground(tone_inverse_muted()))))
    );
}

function text body_copy_mod() {
    return modifier(margin("0"), modifier(style_rule("line-height", "1.65"), foreground(tone_muted())));
}

function text body_copy_inverse_mod() {
    return modifier(margin("0"), modifier(style_rule("line-height", "1.65"), foreground(tone_inverse_muted())));
}

function text caption_mod() {
    return modifier(margin("0"), modifier(font_size("13px"), modifier(style_rule("line-height", "1.5"), foreground(tone_muted()))));
}

function text title_3_mod() {
    return modifier(margin("0"), modifier(font_size("24px"), modifier(style_rule("line-height", "1.15"), foreground(tone_ink()))));
}

function text input_shell_mod() {
    return modifier(
        padding_xy("13px", "15px"),
        modifier(
            corner_radius(radius_2()),
            modifier(
                background("rgba(255,255,255,0.92)"),
                modifier(border("1px solid rgba(15,23,42,0.10)"), foreground(tone_soft_ink()))
            )
        )
    );
}

function text input_dark_shell_mod() {
    return modifier(
        padding_xy("13px", "15px"),
        modifier(
            corner_radius(radius_2()),
            modifier(
                background("rgba(12,19,34,0.92)"),
                modifier(border(line_inverse_soft()), foreground(tone_inverse()))
            )
        )
    );
}

function text field_stack_mod() {
    return modifier(gap_space(space_1()), width_fill());
}

function text form_card_mod() {
    return modifier(
        padding(space_4()),
        modifier(
            corner_radius(radius_3()),
            modifier(background("rgba(255,255,255,0.05)"), modifier(border(line_inverse_soft()), shadow(elevation_1())))
        )
    );
}

function text metric_track_mod() {
    return "height:12px;border-radius:999px;background:#e2e8f0;overflow:hidden";
}

function text metric_fill_mod(text tone, text width) {
    return modifier(height_fixed("12px"), modifier(corner_radius(radius_pill()), modifier(background(tone), width_fixed(width))));
}
