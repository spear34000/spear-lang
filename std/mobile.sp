import "ui.sp";

package std;
module mobile;

function text mobile_title(text value) {
    return ui_title(value);
}

function text mobile_text(text value) {
    return ui_text(value);
}

function text mobile_button(text label, text action_json) {
    return ui_button(label, action_json);
}

function text mobile_stat(text label, text value) {
    return ui_stat(label, value);
}

function text mobile_card(text title, text body_json) {
    return ui_card(title, body_json);
}

function text mobile_column2(text a, text b) {
    return ui_column2(a, b);
}

function text mobile_column3(text a, text b, text c) {
    return ui_column3(a, b, c);
}

function text mobile_screen(text screen_name, text route, text title, text states_json, text body_json) {
    return ui_screen_route(screen_name, route, title, states_json, body_json);
}

function text mobile_android(text screen_name, text screen_json) {
    return ui_android(screen_name, screen_json);
}

function text mobile_desktop(text screen_name, text screen_json) {
    return ui_desktop(screen_name, screen_json);
}
