package std;
module console;

function text read_line() {
    return read();
}

function text prompt_line(text prompt) {
    say(prompt);
    return read();
}

function text prompt_non_empty(text prompt) {
    sharp {
        text value = prompt_line(prompt);
        guard(size(value) > 0, "expected non-empty input");
        return value;
    }
}
