package std;
module weblayout;

function view page_shell(text title, text body) {
    return page(title) {
        markup("main", class_name("page-shell")) {
            markup("section", class_name("page-shell__body")) {
                body;
            };
        };
    };
}

function view page_frame(text title, text hero_block, text body, text footer_block) {
    return page(title) {
        markup("header", class_name("page-frame__header")) {
            hero_block;
        };
        markup("main", class_name("page-frame__main")) {
            markup("section", class_name("page-frame__body")) {
                body;
            };
        };
        markup("footer", class_name("page-frame__footer")) {
            footer_block;
        };
    };
}

function view container(text body) {
    return markup("section", class_name("container")) {
        body;
    };
}

function view section_block(text title, text body) {
    return markup("section", class_name("section-block")) {
        heading2(title);
        body;
    };
}

function view article_block(text title, text body) {
    return markup("article", class_name("article-block")) {
        heading2(title);
        body;
    };
}

function view panel(text title, text body) {
    return markup("section", class_name("panel")) {
        heading1(title);
        paragraph(body);
    };
}

function view card(text title, text body) {
    return panel(title, body);
}

function view split(text left, text right) {
    return markup("div", style_attr("display:flex;gap:16px;flex-wrap:wrap;align-items:flex-start")) {
        markup("section", style_attr("flex:1;min-width:260px")) {
            left;
        };
        markup("section", style_attr("flex:1;min-width:260px")) {
            right;
        };
    };
}

function view stack_block(text top, text bottom) {
    return markup("section", class_name("stack-block")) {
        top;
        bottom;
    };
}

function view header_bar(text title, text body) {
    return markup("header", style_attr("padding:20px 0")) {
        heading1(title);
        paragraph(body);
    };
}

function view footer_note(text value) {
    return markup("footer", style_attr("padding:16px 0;opacity:0.8")) {
        paragraph(value);
    };
}

function view surface(text body) {
    return markup("section", style_attr("padding:16px;border:1px solid #d0d7de;border-radius:16px;background:#ffffff")) {
        body;
    };
}

function view centered(text body) {
    return markup("section", style_attr("max-width:960px;margin:0 auto;padding:24px")) {
        body;
    };
}
