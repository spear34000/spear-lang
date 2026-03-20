run {
    value label = sharp text {
        value prefix = "sharp-";
        defer print("leaving sharp text");
        keep join(prefix, "note");
    };

    map payload = sharp map {
        map next = map();
        put(next, "title", label);
        put(next, "mode", "composed");
        defer print("closing sharp map");
        keep next;
    };

    numbers doubled = sharp numbers {
        numbers items = pack(2, 4, 6);
        defer print("closing sharp numbers");
        keep items;
    };

    print(label);
    print(get(payload, "title", "missing"));
    print(count(doubled));
}
