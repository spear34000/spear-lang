function result load_status(text mode) {
    if (same(mode, "ready")) {
        return ok("online");
    }
    return fail("offline");
}

run {
    result current = load_status("ready");
    result fallback = load_status("sleep");

    print(is_ok(current));
    print(unwrap(current));
    print(is_ok(fallback));
    print(error_text(fallback));
}
