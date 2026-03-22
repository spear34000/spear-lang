app {
    result res = https_get("https://example.com", 5);
    print(text(is_ok(res)));
    if (is_ok(res)) {
        print(text(size(unwrap(res))));
    } else {
        print(error_text(res));
    }
}
