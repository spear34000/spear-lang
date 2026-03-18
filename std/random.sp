import "bridge.sp";
import "json.sp";

package std;
module random;

function text random_between(num start, num end) {
    return python_json("spear_std", "random_int", json_object2(
        json_field("start", json_number(start)),
        json_field("end", json_number(end))
    ));
}

function text random_decimal(text start, text end) {
    return python_json("spear_std", "random_float", json_object2(
        json_field("start", start),
        json_field("end", end)
    ));
}
