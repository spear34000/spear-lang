import "bridge.sp";
import "crypto.sp";
import "json.sp";
import "os.sp";

package std;
module security;

function text secure_token(num bytes) {
    return python_json("spear_std", "secure_token", json_object1(json_field("bytes", json_number(bytes))));
}

function num secure_same(text left, text right) {
    return same(python_json("spear_std", "secure_compare", json_object2(
        json_field("left", json_text(left)),
        json_field("right", json_text(right))
    )), "1");
}

function text password_hash(text password, text salt, num rounds) {
    return python_json("spear_std", "pbkdf2_sha256", json_object3(
        json_field("password", json_text(password)),
        json_field("salt", json_text(salt)),
        json_field("rounds", json_number(rounds))
    ));
}

function text password_hash_default(text password, text salt) {
    return password_hash(password, salt, 120000);
}

function num password_verify(text password, text salt, num rounds, text expected_hash) {
    return same(python_json("spear_std", "pbkdf2_verify", json_object4(
        json_field("password", json_text(password)),
        json_field("salt", json_text(salt)),
        json_field("rounds", json_number(rounds)),
        json_field("expected", json_text(expected_hash))
    )), "1");
}

function result env_secret(text name) {
    text value = env_get(name, "");
    if (size(value) == 0) {
        return fail(join("missing environment variable: ", name));
    }
    return ok(value);
}

function text token_claims_expires_at(num expires_at) {
    return json_object1(
        json_field("exp", json_number(expires_at))
    );
}

function result jwt_verify(text token, text key) {
    return jwt_verify_with_leeway(token, key, 0);
}

function result jwt_verify_with_leeway(text token, text key, num leeway) {
    text raw = python_json("spear_std", "jwt_verify_hs256", json_object3(
        json_field("token", json_text(token)),
        json_field("key", json_text(key)),
        json_field("leeway", json_number(leeway))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "claims"));
    }
    return fail(json_get(raw, "error"));
}
