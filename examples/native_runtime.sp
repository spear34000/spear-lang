run {
    value csv_rows = csv_parse("name,score\nnova,12\nspear,21\n");
    value csv_text = csv_stringify(csv_rows);
    value log_line_text = info_log("native runtime");
    value process_json = process_run("cmd /c echo spear-native");
    value env_json = env_all();
    value random_num = random_between(100, 999);

    write("build/native-config.json", json_parse(json_object2(
        json_field("mode", json_text("native")),
        json_field("kind", json_text("runtime"))
    )));

    print(log_line_text);
    print(csv_text);
    print(random_num);
    print(json_get(process_json, "stdout"));
    print(json_get(env_json, "USERNAME"));
    print(load_config("build/native-config.json"));
}
