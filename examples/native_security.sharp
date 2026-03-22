app {
    val root = "build/native-sandbox";

    result made = safe_make_dir(root, "secrets");
    result wrote = safe_write_text(root, "secrets/token.txt", secure_token(24));
    result read_back = safe_read_text(root, "secrets/token.txt");

    print(text(is_ok(made)));
    print(text(is_ok(wrote)));

    if (is_ok(read_back)) {
        text token = unwrap(read_back);
        print(text(secure_same(token, token)));
        print(token);
    } else {
        print(error_text(read_back));
    }
}
