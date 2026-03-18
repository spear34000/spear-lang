import "math.sp";

package examples;
module hello;

spear launch() {
    numlist scores = pack(add(4, 5), 12, 20);
    textlist words = pack("spear", "edge");
    var shots = 3;

    say(count(scores));
    say(at(scores, 0));
    say(sharpen(at(words, 0)));
    each score in scores {
        say(score);
    }

    for (var i = 0; i < count(words); i = i + 1) {
        if (same(at(words, i), "edge")) {
            continue;
        }
        say(at(words, i));
    }

    while (shots > 0) {
        say(shots);
        if (shots == 2) {
            break;
        }
        shots = shots - 1;
    }
}
