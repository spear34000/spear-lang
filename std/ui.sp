import "bridge.sp";
import "json.sp";

package std;
module ui;

function text ui_state_text(text name, text value) {
    return json_object3(
        json_field("type", json_text("state")),
        json_field("state_type", json_text("text")),
        json_field("name", json_text(name))
    );
}

function text ui_state_text_value(text name, text value) {
    return json_object4(
        json_field("type", json_text("state")),
        json_field("state_type", json_text("text")),
        json_field("name", json_text(name)),
        json_field("value", json_text(value))
    );
}

function text ui_state_num(text name, num value) {
    return json_object4(
        json_field("type", json_text("state")),
        json_field("state_type", json_text("num")),
        json_field("name", json_text(name)),
        json_field("value", json_number(value))
    );
}

function text ui_states1(text a) {
    return json_array1(a);
}

function text ui_states2(text a, text b) {
    return json_array2(a, b);
}

function text ui_states3(text a, text b, text c) {
    return json_array3(a, b, c);
}

function text ui_title(text value) {
    return json_object2(
        json_field("type", json_text("title")),
        json_field("text", json_text(value))
    );
}

function text ui_text(text value) {
    return json_object2(
        json_field("type", json_text("text")),
        json_field("text", json_text(value))
    );
}

function text ui_notice(text title, text body, text tone) {
    return json_object4(
        json_field("type", json_text("notice")),
        json_field("title", json_text(title)),
        json_field("text", json_text(body)),
        json_field("tone", json_text(tone))
    );
}

function text ui_loading(text value) {
    return json_object2(
        json_field("type", json_text("loading")),
        json_field("text", json_text(value))
    );
}

function text ui_empty(text title, text body) {
    return json_object3(
        json_field("type", json_text("empty")),
        json_field("title", json_text(title)),
        json_field("text", json_text(body))
    );
}

function text ui_error(text title, text body) {
    return json_object3(
        json_field("type", json_text("error")),
        json_field("title", json_text(title)),
        json_field("text", json_text(body))
    );
}

function text ui_button(text label, text action_name) {
    return json_object3(
        json_field("type", json_text("button")),
        json_field("label", json_text(label)),
        json_field("action", json_text(action_name))
    );
}

function text ui_link(text label, text route) {
    return json_object3(
        json_field("type", json_text("link")),
        json_field("label", json_text(label)),
        json_field("route", json_text(route))
    );
}

function text ui_input(text state_name, text label, text placeholder) {
    return json_object4(
        json_field("type", json_text("input")),
        json_field("state", json_text(state_name)),
        json_field("label", json_text(label)),
        json_field("placeholder", json_text(placeholder))
    );
}

function text ui_stat(text label, text value) {
    return json_object3(
        json_field("type", json_text("stat")),
        json_field("label", json_text(label)),
        json_field("value", json_text(value))
    );
}

function text ui_card(text title, text body_json) {
    return json_object3(
        json_field("type", json_text("card")),
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_section(text title, text body_json) {
    return json_object3(
        json_field("type", json_text("section")),
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_column1(text a) {
    return json_object2(
        json_field("type", json_text("column")),
        json_field("items", json_array1(a))
    );
}

function text ui_column2(text a, text b) {
    return json_object2(
        json_field("type", json_text("column")),
        json_field("items", json_array2(a, b))
    );
}

function text ui_column3(text a, text b, text c) {
    return json_object2(
        json_field("type", json_text("column")),
        json_field("items", json_array3(a, b, c))
    );
}

function text ui_column4(text a, text b, text c, text d) {
    return json_object2(
        json_field("type", json_text("column")),
        json_field("items", brackets(join(join(join3(a, ",", b), join(",", c)), join(",", d))))
    );
}

function text ui_column5(text a, text b, text c, text d, text e) {
    return json_object2(
        json_field("type", json_text("column")),
        json_field("items", brackets(join(join(join(join3(a, ",", b), join(",", c)), join(",", d)), join(",", e))))
    );
}

function text ui_row2(text a, text b) {
    return json_object2(
        json_field("type", json_text("row")),
        json_field("items", json_array2(a, b))
    );
}

function text ui_row3(text a, text b, text c) {
    return json_object2(
        json_field("type", json_text("row")),
        json_field("items", json_array3(a, b, c))
    );
}

function text ui_form2(text a, text b) {
    return json_object2(
        json_field("type", json_text("form")),
        json_field("items", json_array2(a, b))
    );
}

function text ui_form3(text a, text b, text c) {
    return json_object2(
        json_field("type", json_text("form")),
        json_field("items", json_array3(a, b, c))
    );
}

function text ui_nav2(text a, text b) {
    return json_object2(
        json_field("type", json_text("nav")),
        json_field("items", json_array2(a, b))
    );
}

function text ui_nav3(text a, text b, text c) {
    return json_object2(
        json_field("type", json_text("nav")),
        json_field("items", json_array3(a, b, c))
    );
}

function text ui_screen(text title, text states_json, text body_json) {
    return json_object4(
        json_field("type", json_text("screen")),
        json_field("title", json_text(title)),
        json_field("states", states_json),
        json_field("body", body_json)
    );
}

function text ui_web(text screen_json) {
    return python_json("spear_std", "ui_web_render", screen_json);
}

function text ui_android(text screen_name, text screen_json) {
    return python_json("spear_std", "ui_android_render", json_object2(
        json_field("screen_name", json_text(screen_name)),
        json_field("body", screen_json)
    ));
}
