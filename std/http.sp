import "bridge.sp";
import "json.sp";

package std;
module http;

function text https_request_json(text method, text url, text body, text content_type, num timeout_seconds) {
    return python_json("spear_std", "safe_http_request", json_object5(
        json_field("method", json_text(method)),
        json_field("url", json_text(url)),
        json_field("body", json_text(body)),
        json_field("content_type", json_text(content_type)),
        json_field("timeout", json_number(timeout_seconds))
    ));
}

function result https_get(text url, num timeout_seconds) {
    text raw = https_request_json("GET", url, "", "", timeout_seconds);
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "body"));
    }
    return fail(json_get(raw, "error"));
}

function result https_post_json(text url, text body_json, num timeout_seconds) {
    text raw = https_request_json("POST", url, body_json, "application/json; charset=utf-8", timeout_seconds);
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "body"));
    }
    return fail(json_get(raw, "error"));
}
