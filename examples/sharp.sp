run {
    value label = sharp text {
        value prefix = "sharp-";
        keep join(prefix, "note");
    };

    map payload = sharp map {
        map next = map();
        put(next, "title", label);
        put(next, "mode", "composed");
        keep next;
    };

    numbers doubled = sharp numbers {
        numbers items = pack(2, 4, 6);
        keep items;
    };

    print(label);
    print(get(payload, "title", "missing"));
    print(count(doubled));
}
