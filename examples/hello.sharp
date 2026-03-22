package examples;
module hello;

run {
    numbers scores = pack(add(4, 5), 12, 20);
    strings words = pack("spear", "edge");
    variable shots = clamp_num(3, 0, 10);

    print(count(scores));
    print(at(scores, 0));
    print(prefix("sharp-", at(words, 0)));
    print(sum(scores));
    print(size(repeat("=", 5)));
    each score in scores {
        print(score);
    }

    for (var i = 0; i < count(words); i = i + 1) {
        if (same(at(words, i), "edge")) {
            continue;
        }
        print(at(words, i));
    }

    while (shots > 0) {
        print(shots);
        if (shots == 2) {
            break;
        }
        shots = sub(shots, 1);
    }
}
