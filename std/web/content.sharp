package std;
module webcontent;

function view heading1(text value) {
    return markup("h1", style_attr("font-size:32px;line-height:1.1;margin:0")) {
        escape(value);
    };
}

function view heading2(text value) {
    return markup("h2", style_attr("font-size:24px;line-height:1.2;margin:0")) {
        escape(value);
    };
}

function view heading3(text value) {
    return markup("h3", style_attr("font-size:18px;line-height:1.3;margin:0")) {
        escape(value);
    };
}

function view paragraph(text value) {
    return markup("p", style_attr("margin:0;line-height:1.6")) {
        escape(value);
    };
}

function view lead(text value) {
    return markup("p", style_attr("margin:0;font-size:18px;line-height:1.7")) {
        escape(value);
    };
}

function view hero(text title, text body) {
    return markup("section", style_attr("padding:20px;border-radius:20px;background:#f5f7fb")) {
        heading1(title);
        lead(body);
    };
}

function view intro(text title, text body) {
    return markup("section") {
        heading2(title);
        paragraph(body);
    };
}

function view note(text value) {
    return markup("aside", style_attr("padding:14px;border-left:4px solid #64748b;background:#f8fafc")) {
        paragraph(value);
    };
}

function view success_note(text title, text body) {
    return markup("aside", style_attr("padding:14px;border-left:4px solid #16a34a;background:#f0fdf4")) {
        heading3(title);
        paragraph(body);
    };
}

function view warning_note(text title, text body) {
    return markup("aside", style_attr("padding:14px;border-left:4px solid #d97706;background:#fffbeb")) {
        heading3(title);
        paragraph(body);
    };
}

function view item(text value) {
    return markup("li", style_attr("margin:4px 0")) {
        escape(value);
    };
}

function view bullet_list(text body) {
    return markup("ul") {
        body;
    };
}

function view number_list(text body) {
    return markup("ol") {
        body;
    };
}

function view quote_block(text value) {
    return markup("blockquote", style_attr("margin:0;padding:12px 16px;border-left:4px solid #94a3b8;background:#f8fafc")) {
        paragraph(value);
    };
}

function view code_line(text value) {
    return markup("pre", style_attr("margin:0;padding:12px 14px;border-radius:12px;background:#111827;color:#f9fafb;overflow:auto")) {
        escape(value);
    };
}

function view metric(text label, text value) {
    return markup("section", style_attr("padding:12px 14px;border:1px solid #dbe2ea;border-radius:14px;background:#fff")) {
        paragraph(label);
        heading3(value);
    };
}

function view empty_state(text title, text body) {
    return markup("section", style_attr("padding:24px;border:1px dashed #cbd5e1;border-radius:16px;text-align:center")) {
        heading2(title);
        paragraph(body);
    };
}

function view faq_item(text question, text answer) {
    return markup("section", style_attr("padding:12px 0;border-bottom:1px solid #e5e7eb")) {
        heading3(question);
        paragraph(answer);
    };
}

function view badge(text value) {
    return markup("span", style_attr("display:inline-block;padding:4px 10px;border-radius:999px;background:#e2e8f0;font-size:12px")) {
        escape(value);
    };
}
