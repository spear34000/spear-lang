import "bridge.sp";
import "json.sp";

package std;
module logger;

function text log_line(text level, text message) {
    return python_json("spear_std", "logger_line", json_object2(
        json_field("level", json_text(level)),
        json_field("message", json_text(message))
    ));
}

function text info_log(text message) {
    return log_line("INFO", message);
}

function text warn_log(text message) {
    return log_line("WARN", message);
}

function text error_log(text message) {
    return log_line("ERROR", message);
}
