package std;
module webblocks;

function view landing_page(text title, text theme_fill, text sections) {
    return page(title) {
        markup("main", style_attr(theme_page(theme_fill))) {
            showcase_shell(showcase_flow(sections));
        };
    };
}

function view hero_scene(text header, text spotlight_block, text side_block) {
    return hero_showcase(header, spotlight_block, side_block);
}

function view insights_band(text title, text body, text content) {
    return feature_stage(title, body, content);
}

function view conversion_band(text title, text body, text href, text label) {
    return launch_banner(title, body, href, label);
}

function view feature_triptych(text a_eye, text a_title, text a_body, text b_eye, text b_title, text b_body, text c_eye, text c_title, text c_body) {
    return aura_triptych(a_eye, a_title, a_body, b_eye, b_title, b_body, c_eye, c_title, c_body);
}
