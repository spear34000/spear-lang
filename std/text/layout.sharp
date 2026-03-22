package std;
module layout;

function text line(text value) {
    return suffix(value, newline());
}

function text repeat(text value, num times) {
    sharp {
        var out = "";
        for (var i = 0; i < times; i = i + 1) {
            out = join(out, value);
        }
        return out;
    }
}

function text indent(text value, num depth) {
    sharp {
        text pad = repeat("    ", depth);
        return prefix(pad, value);
    }
}

function num is_empty_text(text value) {
    return size(value) == 0;
}

function num not_empty_text(text value) {
    return size(value) != 0;
}

function text bullet_line(text value) {
    return line(prefix("- ", value));
}
