run {
    map state = map();
    put(state, "mode", "native");
    put(state, "status", "ready");

    print(get(state, "mode", "missing"));
    print(count(state));
    print(has(state, "status"));

    drop(state, "status");
    print(count(state));
}
