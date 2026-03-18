import "../std/cli.sp";
import "../std/map.sp";
import "../std/json.sp";

run {
    text state = map_empty();
    state = map_set_text(state, "mode", "tool");
    state = map_set_json(state, "args", json_number(args_count()));

    print(state);
    print(map_get(state, "mode", "unknown"));
    print(text(args_count()));
    if (args_count() > 0) {
        print(arg_text(0));
    } else {
        print("no args");
    }
}
