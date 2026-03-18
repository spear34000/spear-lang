package std;
module builders;

function text empty() {
    return "";
}

function text space() {
    return " ";
}

function text newline() {
    return "\n";
}

function text tab() {
    return "\t";
}

function text comma_space() {
    return ", ";
}

function text colon_space() {
    return ": ";
}

function text prefix(text left, text value) {
    return join(left, value);
}

function text suffix(text value, text right) {
    return join(value, right);
}

function text surround(text left, text value, text right) {
    sharp {
        text merged = join(left, value);
        return join(merged, right);
    }
}

function text quote(text value) {
    return surround("\"", value, "\"");
}

function text parens(text value) {
    return surround("(", value, ")");
}

function text brackets(text value) {
    return surround("[", value, "]");
}

function text braces(text value) {
    return surround("{", value, "}");
}

function text join3(text first, text second, text third) {
    sharp {
        text left = join(first, second);
        return join(left, third);
    }
}

function text join4(text first, text second, text third, text fourth) {
    sharp {
        text left = join3(first, second, third);
        return join(left, fourth);
    }
}

function text label_value(text label, text value) {
    sharp {
        text left = join(label, colon_space());
        return join(left, value);
    }
}
