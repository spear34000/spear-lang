package std;
module webmodifiers;

function text no_modifier() {
    return "";
}

function text modifier(text left, text right) {
    if (size(left) == 0) {
        return right;
    }
    if (size(right) == 0) {
        return left;
    }
    return join(join(left, ";"), right);
}

function text style_rule(text name, text value) {
    return join(join(name, ":"), value);
}

function text class_token(text value) {
    return class_name(value);
}

function text id_token(text value) {
    return id_name(value);
}

function text style_token(text value) {
    return value;
}

function text padding(text value) {
    return join("padding:", value);
}

function text padding_all(text value) {
    return padding(value);
}

function text padding_xy(text vertical, text horizontal) {
    text first = join("padding:", vertical);
    text second = join(" ", horizontal);
    return join(first, second);
}

function text padding_x(text value) {
    return modifier(join("padding-left:", value), join("padding-right:", value));
}

function text padding_y(text value) {
    return modifier(join("padding-top:", value), join("padding-bottom:", value));
}

function text margin(text value) {
    return join("margin:", value);
}

function text margin_all(text value) {
    return margin(value);
}

function text margin_x(text value) {
    return modifier(join("margin-left:", value), join("margin-right:", value));
}

function text margin_y(text value) {
    return modifier(join("margin-top:", value), join("margin-bottom:", value));
}

function text gap_space(text value) {
    return join("gap:", value);
}

function text bg(text value) {
    return join("background:", value);
}

function text background(text value) {
    return bg(value);
}

function text text_color(text value) {
    return join("color:", value);
}

function text foreground(text value) {
    return text_color(value);
}

function text rounded(text value) {
    return join("border-radius:", value);
}

function text corner_radius(text value) {
    return rounded(value);
}

function text border_line(text value) {
    return join("border:", value);
}

function text border(text value) {
    return border_line(value);
}

function text width_fill() {
    return "width:100%";
}

function text fill_max_width() {
    return width_fill();
}

function text width_fixed(text value) {
    return join("width:", value);
}

function text min_width(text value) {
    return join("min-width:", value);
}

function text max_width(text value) {
    return join("max-width:", value);
}

function text height_fixed(text value) {
    return join("height:", value);
}

function text fill_max_height() {
    return "height:100%";
}

function text min_height(text value) {
    return join("min-height:", value);
}

function text box_size(text width, text height) {
    return modifier(width_fixed(width), height_fixed(height));
}

function text center_x() {
    return "margin-left:auto;margin-right:auto";
}

function text center_items() {
    return "align-items:center";
}

function text justify_center() {
    return "justify-content:center";
}

function text justify_between() {
    return "justify-content:space-between";
}

function text justify_start() {
    return "justify-content:flex-start";
}

function text justify_end() {
    return "justify-content:flex-end";
}

function text align_start() {
    return "align-items:flex-start";
}

function text align_end() {
    return "align-items:flex-end";
}

function text align_center() {
    return center_items();
}

function text shadow(text value) {
    return join("box-shadow:", value);
}

function text font_size(text value) {
    return join("font-size:", value);
}

function text font_weight(text value) {
    return join("font-weight:", value);
}

function text flex_col() {
    return "display:flex;flex-direction:column";
}

function text flex_row() {
    return "display:flex;flex-direction:row";
}

function text wrap() {
    return "flex-wrap:wrap";
}

function view vstack(text mods, text body) {
    return column_box(mods, body);
}

function view hstack(text mods, text body) {
    return row_box(mods, body);
}

function view text_block(text mods, text value) {
    return markup("p", style_attr(mods)) {
        escape(value);
    };
}

function view label_text(text mods, text value) {
    return text_block(mods, value);
}

function view button_link(text mods, text href, text label) {
    return markup("a", attrs(href_attr(href), style_attr(mods))) {
        escape(label);
    };
}

function view image_block(text mods, text src, text alt) {
    return markup("img", attrs(src_attr(src), attrs(alt_attr(alt), style_attr(mods))), "");
}

function view spacer(text height) {
    return markup("div", style_attr(height_fixed(height)), "");
}
