num add(num left, num right) {
    return left + right;
}

text sharpen(text base) {
    sharp {
        let prefix = "sharp-";
        return join(prefix, base);
    }
}

spear launch() {
    numlist scores = pack(add(4, 5), 12, 20);
    textlist words = pack("spear", "edge");
    let shots = 3;

    say(count(scores));
    say(at(scores, 0));
    say(sharpen(at(words, 0)));
    each score in scores {
        say(score);
    }

    while (shots > 0) {
        say(shots);
        shots = shots - 1;
    }
}
