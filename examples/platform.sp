run {
    const payload = json_object2(
        json_field("name", json_text("Spear")),
        json_field("kind", json_text("platform"))
    );

    numlist values = pack(8, 12, 20, 24);
    numlist left = pack(1, 2, 3);
    numlist right = pack(4, 5, 6);

    print(info_log("platform example"));
    print(json_parse(payload));
    print(sha256("spear"));
    print(text(stats_mean(values)));
    print(text(vector_dot(left, right)));
    print(text(matrix_row_sum(pack(1, 2, 3, 4), 2, 1)));
    print(text(tensor3_at(pack(1, 2, 3, 4, 5, 6, 7, 8), 2, 2, 1, 0, 1)));
    print(random_between(10, 99));
    print(result_value(ok_result(json_text("ready"))));
    print(write_file_text("build/platform.json", json_parse(payload)));
    print(path_exists("build/platform.json"));
    print(env_get("USERNAME", "unknown"));
    print(process_run("cmd /c echo spear"));
    print(http_get("https://example.com"));
    print(sqlite_execute("build/platform.db", "create table if not exists logs(message text)"));
    print(sqlite_execute("build/platform.db", "insert into logs(message) values ('hello from spear')"));
    print(sqlite_query("build/platform.db", "select message from logs"));
}
