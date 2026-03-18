import "bridge.sp";
import "json.sp";

package std;
module net;

function text http_get(text url) {
    return python_json("spear_std", "http_request", json_object2(
        json_field("method", json_text("GET")),
        json_field("url", json_text(url))
    ));
}

function text http_post(text url, text body) {
    return python_json("spear_std", "http_request", json_object3(
        json_field("method", json_text("POST")),
        json_field("url", json_text(url)),
        json_field("body", json_text(body))
    ));
}

function text tcp_request(text host, num port, text message) {
    return python_json("spear_std", "tcp_request", json_object3(
        json_field("host", json_text(host)),
        json_field("port", json_number(port)),
        json_field("message", json_text(message))
    ));
}

function text udp_send(text host, num port, text message) {
    return python_json("spear_std", "udp_send", json_object3(
        json_field("host", json_text(host)),
        json_field("port", json_number(port)),
        json_field("message", json_text(message))
    ));
}
