import "bridge.sp";
import "json.sp";

package std;
module safefs;

function result safe_make_dir(text root, text relative_path) {
    text raw = python_json("spear_std", "fs_safe_mkdir", json_object2(
        json_field("root", json_text(root)),
        json_field("path", json_text(relative_path))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "path"));
    }
    return fail(json_get(raw, "error"));
}

function result safe_write_text(text root, text relative_path, text content) {
    text raw = python_json("spear_std", "fs_safe_write", json_object3(
        json_field("root", json_text(root)),
        json_field("path", json_text(relative_path)),
        json_field("content", json_text(content))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "path"));
    }
    return fail(json_get(raw, "error"));
}

function result safe_read_text(text root, text relative_path) {
    text raw = python_json("spear_std", "fs_safe_read", json_object2(
        json_field("root", json_text(root)),
        json_field("path", json_text(relative_path))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "text"));
    }
    return fail(json_get(raw, "error"));
}

function result safe_list_dir(text root, text relative_path) {
    text raw = python_json("spear_std", "fs_safe_list", json_object2(
        json_field("root", json_text(root)),
        json_field("path", json_text(relative_path))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "items"));
    }
    return fail(json_get(raw, "error"));
}

function result safe_remove_path(text root, text relative_path) {
    text raw = python_json("spear_std", "fs_safe_remove", json_object2(
        json_field("root", json_text(root)),
        json_field("path", json_text(relative_path))
    ));
    if (same(json_get(raw, "ok"), "true")) {
        return ok(json_get(raw, "path"));
    }
    return fail(json_get(raw, "error"));
}
