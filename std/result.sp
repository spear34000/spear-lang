import "bridge.sp";
import "json.sp";
import "text.sp";

package std;
module results;

function text ok_result(text value_json) {
    return json_object2(
        json_field("ok", "true"),
        json_field("value", value_json)
    );
}

function text error_result(text message) {
    return json_object2(
        json_field("ok", "false"),
        json_field("error", json_text(message))
    );
}

function num result_is_ok(text body) {
    return same(python_json("spear_std", "result_is_ok", json_object1(json_field("text", json_text(body)))), "1");
}

function text result_value(text body) {
    return python_json("spear_std", "result_value", json_object1(json_field("text", json_text(body))));
}

function text result_error(text body) {
    return python_json("spear_std", "result_error", json_object1(json_field("text", json_text(body))));
}
