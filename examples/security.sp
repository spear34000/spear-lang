run {
    const root = "build/security-sandbox";
    const salt = "demo-salt";
    const digest = password_hash("spear", salt, 120000);

    result made = safe_make_dir(root, "secrets");
    result wrote = safe_write_text(root, "secrets/token.txt", secure_token(24));
    result read_back = safe_read_text(root, "secrets/token.txt");
    result secret_env = env_secret("SPEAR_DEMO_SECRET");

    print(is_ok(made));
    print(is_ok(wrote));
    print(text(password_verify("spear", salt, 120000, digest)));
    print(text(secure_same(digest, digest)));

    if (is_ok(read_back)) {
        print(unwrap(read_back));
    } else {
        print(error_text(read_back));
    }

    if (is_ok(secret_env)) {
        print("secret loaded");
    } else {
        print(error_text(secret_env));
    }
}
