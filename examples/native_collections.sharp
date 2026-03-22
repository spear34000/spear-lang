run {
    value base = json_parse(json_object2(
        json_field("name", json_text("Spear")),
        json_field("status", json_text("ready"))
    ));
    value changed = map_set_text(base, "mode", "native");
    value removed = map_remove(changed, "status");
    value result_ok = ok_result(json_text("steady"));

    print(json_keys(changed));
    print(map_get(changed, "mode", "missing"));
    print(text(map_has(removed, "status")));
    print(result_value(result_ok));
    print(text(result_is_ok(result_ok)));
    print(vector_add(pack(1, 2, 3), pack(4, 5, 6)));
}
