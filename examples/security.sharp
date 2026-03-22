app {
    val root = "build/security-sandbox";
    val salt = "demo-salt";
    val digest = password_hash_default("spear", salt);
    val good_token = jwt_sign_hs256(token_claims_expires_at(4102444800), "demo-key");

    result made = safe_make_dir(root, "secrets");
    result wrote = safe_write_text(root, "secrets/token.txt", secure_token(24));
    result read_back = safe_read_text(root, "secrets/token.txt");
    result path_escape = safe_read_text(root, "../escape.txt");
    result secret_env = env_secret("SPEAR_DEMO_SECRET");
    result jwt_ok = jwt_verify_with_leeway(good_token, "demo-key", 30);
    result jwt_bad = jwt_verify(good_token, "wrong-key");
    result http_bad = https_get("http://example.com", 5);

    print(text(is_ok(made)));
    print(text(is_ok(wrote)));
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

    if (is_ok(jwt_ok)) {
        print("jwt ok");
    } else {
        print(error_text(jwt_ok));
    }

    print(error_text(jwt_bad));
    print(error_text(path_escape));
    print(error_text(http_bad));
}
