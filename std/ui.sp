import "bridge.sp";
import "json.sp";

package std;
module ui;

function text ui_node2(text kind, text a) {
    return json_object2(
        json_field("type", json_text(kind)),
        a
    );
}

function text ui_node3(text kind, text a, text b) {
    return json_object3(
        json_field("type", json_text(kind)),
        a,
        b
    );
}

function text ui_node4(text kind, text a, text b, text c) {
    return json_object4(
        json_field("type", json_text(kind)),
        a,
        b,
        c
    );
}

function text ui_node5(text kind, text a, text b, text c, text d) {
    return json_object5(
        json_field("type", json_text(kind)),
        a,
        b,
        c,
        d
    );
}

function text ui_node6(text kind, text a, text b, text c, text d, text e) {
    return json_object6(
        json_field("type", json_text(kind)),
        a,
        b,
        c,
        d,
        e
    );
}

function text ui_items1(text a) {
    return json_array1(a);
}

function text ui_items2(text a, text b) {
    return json_array2(a, b);
}

function text ui_items3(text a, text b, text c) {
    return json_array3(a, b, c);
}

function text ui_items4(text a, text b, text c, text d) {
    return json_array4(a, b, c, d);
}

function text ui_items5(text a, text b, text c, text d, text e) {
    return json_array5(a, b, c, d, e);
}

function text ui_state_text(text name, text value) {
    return json_object4(
        json_field("type", json_text("state")),
        json_field("state_type", json_text("text")),
        json_field("name", json_text(name)),
        json_field("value", json_text(value))
    );
}

function text ui_state_text_value(text name, text value) {
    return ui_state_text(name, value);
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

function text ui_action_tap(text name) {
    return json_object2(
        json_field("type", json_text("tap")),
        json_field("name", json_text(name))
    );
}

function text ui_action_submit(text name) {
    return json_object2(
        json_field("type", json_text("submit")),
        json_field("name", json_text(name))
    );
}

function text ui_go(text route) {
    return json_object2(
        json_field("type", json_text("go")),
        json_field("route", json_text(route))
    );
}

function text ui_back() {
    return json_object1(
        json_field("type", json_text("back"))
    );
}

function text ui_action_open_dialog(text dialog_name) {
    return json_object2(
        json_field("type", json_text("open_dialog")),
        json_field("dialog", json_text(dialog_name))
    );
}

function text ui_action_close_dialog(text dialog_name) {
    return json_object2(
        json_field("type", json_text("close_dialog")),
        json_field("dialog", json_text(dialog_name))
    );
}

function text ui_action_set_state(text state_name, text value_json) {
    return json_object3(
        json_field("type", json_text("set_state")),
        json_field("state_target", json_text(state_name)),
        json_field("state_value", value_json)
    );
}

function text ui_title(text value) {
    return ui_node2("title", json_field("text", json_text(value)));
}

function text ui_text(text value) {
    return ui_node2("text", json_field("text", json_text(value)));
}

function text ui_notice(text title, text body, text tone) {
    return ui_node4(
        "notice",
        json_field("title", json_text(title)),
        json_field("text", json_text(body)),
        json_field("tone", json_text(tone))
    );
}

function text ui_loading(text value) {
    return ui_node2("loading", json_field("text", json_text(value)));
}

function text ui_empty(text title, text body) {
    return ui_node3(
        "empty",
        json_field("title", json_text(title)),
        json_field("text", json_text(body))
    );
}

function text ui_error(text title, text body) {
    return ui_node3(
        "error",
        json_field("title", json_text(title)),
        json_field("text", json_text(body))
    );
}

function text ui_button(text label, text action_json) {
    return ui_node3(
        "button",
        json_field("label", json_text(label)),
        json_field("action", action_json)
    );
}

function text ui_link(text label, text route) {
    return ui_node4(
        "link",
        json_field("label", json_text(label)),
        json_field("route", json_text(route)),
        json_field("action", ui_go(route))
    );
}

function text ui_input(text state_name, text label, text placeholder) {
    return braces(join(
        join(
            join(
                join(
                    join(
                        join(
                            join3(
                                json_field("type", json_text("input")),
                                ",",
                                json_field("state", json_text(state_name))
                            ),
                            join(",", json_field("label", json_text(label)))
                        ),
                        join(",", json_field("placeholder", json_text(placeholder)))
                    ),
                    join(",", json_field("kind", json_text("text")))
                ),
                join(",", json_field("hint", json_text("")))
            ),
            join(",", json_field("error_text", json_text("")))
        ),
        join(join(",", json_field("required", json_bool(0))), join(",", json_field("disabled", json_bool(0))))
    ));
}

function text ui_input_with_kind(text state_name, text label, text placeholder, text kind, text hint) {
    return braces(join(
        join(
            join(
                join(
                    join(
                        join(
                            join3(
                                json_field("type", json_text("input")),
                                ",",
                                json_field("state", json_text(state_name))
                            ),
                            join(",", json_field("label", json_text(label)))
                        ),
                        join(",", json_field("placeholder", json_text(placeholder)))
                    ),
                    join(",", json_field("kind", json_text(kind)))
                ),
                join(",", json_field("hint", json_text(hint)))
            ),
            join(",", json_field("error_text", json_text("")))
        ),
        join(join(",", json_field("required", json_bool(0))), join(",", json_field("disabled", json_bool(0))))
    ));
}

function text ui_input_required(text state_name, text label, text placeholder, text kind, text hint) {
    return braces(join(
        join(
            join(
                join(
                    join(
                        join(
                            join3(
                                json_field("type", json_text("input")),
                                ",",
                                json_field("state", json_text(state_name))
                            ),
                            join(",", json_field("label", json_text(label)))
                        ),
                        join(",", json_field("placeholder", json_text(placeholder)))
                    ),
                    join(",", json_field("kind", json_text(kind)))
                ),
                join(",", json_field("hint", json_text(hint)))
            ),
            join(",", json_field("error_text", json_text("")))
        ),
        join(join(",", json_field("required", json_bool(1))), join(",", json_field("disabled", json_bool(0))))
    ));
}

function text ui_stat(text label, text value) {
    return ui_node3(
        "stat",
        json_field("label", json_text(label)),
        json_field("value", json_text(value))
    );
}

function text ui_card(text title, text body_json) {
    return ui_node3(
        "card",
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_section(text title, text body_json) {
    return ui_node3(
        "section",
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_dialog(text title, text body_json) {
    return ui_node4(
        "dialog",
        json_field("title", json_text(title)),
        json_field("body", body_json),
        json_field("action", ui_action_open_dialog(title))
    );
}

function text ui_sheet(text title, text body_json) {
    return ui_node3(
        "sheet",
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_divider() {
    return json_object1(json_field("type", json_text("divider")));
}

function text ui_item(text title, text body_json) {
    return ui_node3(
        "item",
        json_field("title", json_text(title)),
        json_field("body", body_json)
    );
}

function text ui_list2(text a, text b) {
    return ui_node2("list", json_field("items", ui_items2(a, b)));
}

function text ui_list3(text a, text b, text c) {
    return ui_node2("list", json_field("items", ui_items3(a, b, c)));
}

function text ui_toolbar(text title, text left_items_json, text right_items_json) {
    return ui_node4(
        "toolbar",
        json_field("title", json_text(title)),
        json_field("left", left_items_json),
        json_field("right", right_items_json)
    );
}

function text ui_tabs2(text a, text b) {
    return ui_node2("tabs", json_field("items", ui_items2(a, b)));
}

function text ui_tabs3(text a, text b, text c) {
    return ui_node2("tabs", json_field("items", ui_items3(a, b, c)));
}

function text ui_column1(text a) {
    return ui_node2("column", json_field("items", ui_items1(a)));
}

function text ui_column2(text a, text b) {
    return ui_node2("column", json_field("items", ui_items2(a, b)));
}

function text ui_column3(text a, text b, text c) {
    return ui_node2("column", json_field("items", ui_items3(a, b, c)));
}

function text ui_column4(text a, text b, text c, text d) {
    return ui_node2("column", json_field("items", ui_items4(a, b, c, d)));
}

function text ui_column5(text a, text b, text c, text d, text e) {
    return ui_node2("column", json_field("items", ui_items5(a, b, c, d, e)));
}

function text ui_row2(text a, text b) {
    return ui_node2("row", json_field("items", ui_items2(a, b)));
}

function text ui_row3(text a, text b, text c) {
    return ui_node2("row", json_field("items", ui_items3(a, b, c)));
}

function text ui_form2(text a, text b) {
    return ui_node2("form", json_field("items", ui_items2(a, b)));
}

function text ui_form3(text a, text b, text c) {
    return ui_node2("form", json_field("items", ui_items3(a, b, c)));
}

function text ui_form_submit2(text title, text action_json, text a, text b) {
    return ui_node4(
        "form",
        json_field("title", json_text(title)),
        json_field("action", action_json),
        json_field("items", ui_items2(a, b))
    );
}

function text ui_form_submit3(text title, text action_json, text a, text b, text c) {
    return ui_node4(
        "form",
        json_field("title", json_text(title)),
        json_field("action", action_json),
        json_field("items", ui_items3(a, b, c))
    );
}

function text ui_nav2(text a, text b) {
    return ui_node2("nav", json_field("items", ui_items2(a, b)));
}

function text ui_nav3(text a, text b, text c) {
    return ui_node2("nav", json_field("items", ui_items3(a, b, c)));
}

function text ui_screen(text title, text states_json, text body_json) {
    return ui_screen_route("MainScreen", "/", title, states_json, body_json);
}

function text ui_screen_route(text screen_name, text route, text title, text states_json, text body_json) {
    return json_object6(
        json_field("type", json_text("screen")),
        json_field("screen_name", json_text(screen_name)),
        json_field("route", json_text(route)),
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

function text ui_desktop(text screen_name, text screen_json) {
    return python_json("spear_std", "ui_desktop_render", json_object2(
        json_field("screen_name", json_text(screen_name)),
        json_field("body", screen_json)
    ));
}
