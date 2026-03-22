package std;
module webactions;

function view actions(text primary_href, text primary_label, text secondary_href, text secondary_label) {
    return row {
        action(primary_href, primary_label);
        action(secondary_href, secondary_label);
    };
}

function view action_bar(text href_a, text label_a, text href_b, text label_b) {
    return markup("div", class_name("action-bar")) {
        actions(href_a, label_a, href_b, label_b);
    };
}

function view primary_action(text href, text label) {
    return markup("a", attrs(href_attr(href), style_attr("display:inline-block;padding:10px 16px;border-radius:12px;background:#111827;color:#ffffff;text-decoration:none"))) {
        escape(label);
    };
}

function view secondary_action(text href, text label) {
    return markup("a", attrs(href_attr(href), style_attr("display:inline-block;padding:10px 16px;border-radius:12px;border:1px solid #cbd5e1;color:#111827;text-decoration:none;background:#ffffff"))) {
        escape(label);
    };
}

function view link_row(text href_a, text label_a, text href_b, text label_b, text href_c, text label_c) {
    return row {
        secondary_action(href_a, label_a);
        secondary_action(href_b, label_b);
        secondary_action(href_c, label_c);
    };
}

function view nav_bar(text home_href, text home_label, text docs_href, text docs_label, text about_href, text about_label) {
    return markup("nav", style_attr("padding:8px 0")) {
        link_row(home_href, home_label, docs_href, docs_label, about_href, about_label);
    };
}

function view cta_section(text title, text body, text href, text label) {
    return markup("section") {
        heading2(title);
        paragraph(body);
        primary_action(href, label);
    };
}

function view docs_cta(text href, text label) {
    return markup("aside") {
        primary_action(href, label);
    };
}

function view button_row(text primary_href, text primary_label, text secondary_href, text secondary_label) {
    return row {
        primary_action(primary_href, primary_label);
        secondary_action(secondary_href, secondary_label);
    };
}
