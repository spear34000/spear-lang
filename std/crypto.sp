import "bridge.sp";
import "json.sp";

package std;
module crypto;

function text sha256(text body) {
    return python_json("spear_std", "sha256_hex", json_object1(json_field("text", json_text(body))));
}

function text hmac_sha256(text key, text body) {
    return python_json("spear_std", "hmac_sha256", json_object2(
        json_field("key", json_text(key)),
        json_field("text", json_text(body))
    ));
}

function text jwt_sign_hs256(text claims_json, text key) {
    return python_json("spear_std", "jwt_hs256", json_object2(
        json_field("claims", claims_json),
        json_field("key", json_text(key))
    ));
}

function text jwt_decode(text token) {
    return python_json("spear_std", "jwt_decode", json_object1(json_field("token", json_text(token))));
}
