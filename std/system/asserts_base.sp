package std;
module assertsbase;

function num require(num condition, text message) {
    guard(condition, message);
    return 1;
}

function text require_non_empty(text value, text message) {
    guard(size(value) > 0, message);
    return value;
}

function num require_positive(num value, text message) {
    guard(value > 0, message);
    return value;
}

function num require_non_negative(num value, text message) {
    guard(value >= 0, message);
    return value;
}

function num require_between(num value, num low, num high, text message) {
    guard(value >= low, message);
    guard(value <= high, message);
    return value;
}
