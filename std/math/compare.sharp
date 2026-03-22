package std;
module compare;

function num min_num(num left, num right) {
    if (left < right) {
        return left;
    }
    return right;
}

function num max_num(num left, num right) {
    if (left > right) {
        return left;
    }
    return right;
}

function num clamp_num(num value, num low, num high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

function num between_num(num value, num low, num high) {
    if (value < low) {
        return 0;
    }
    if (value > high) {
        return 0;
    }
    return 1;
}

function num is_even(num value) {
    return mod(value, 2) == 0;
}

function num is_odd(num value) {
    return mod(value, 2) != 0;
}

function num sign_num(num value) {
    if (value < 0) {
        return -1;
    }
    if (value > 0) {
        return 1;
    }
    return 0;
}
