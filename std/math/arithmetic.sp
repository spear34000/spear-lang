package std;
module arithmetic;

function num add(num left, num right) {
    return left + right;
}

function num sub(num left, num right) {
    return left - right;
}

function num mul(num left, num right) {
    return left * right;
}

function num div_num(num left, num right) {
    guard(right != 0, "division by zero");
    return left / right;
}

function num mod(num left, num right) {
    guard(right != 0, "modulo by zero");
    return left % right;
}

function num inc(num value) {
    return value + 1;
}

function num dec(num value) {
    return value - 1;
}

function num abs_num(num value) {
    if (value < 0) {
        return 0 - value;
    }
    return value;
}

function num square_num(num value) {
    return value * value;
}

function num cube_num(num value) {
    return value * value * value;
}
