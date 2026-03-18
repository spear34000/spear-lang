import "bridge.sp";
import "json.sp";

package std;
module csv;

function text csv_parse(text body) {
    return python_json("spear_std", "csv_parse", json_object1(json_field("text", json_text(body))));
}

function text csv_stringify(text rows_json) {
    return python_json("spear_std", "csv_stringify", json_object1(json_field("rows", rows_json)));
}
