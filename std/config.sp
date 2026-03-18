import "bridge.sp";
import "json.sp";

package std;
module config;

function text load_config(text path) {
    return python_json("spear_std", "config_load", json_object1(json_field("path", json_text(path))));
}
