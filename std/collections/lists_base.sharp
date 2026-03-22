package std;
module listsbase;

function num sum(numlist items) {
    var total = 0;
    each item in items {
        total = total + item;
    }
    return total;
}

function num product(numlist items) {
    var total = 1;
    each item in items {
        total = total * item;
    }
    return total;
}

function num first_num(numlist items) {
    guard(count(items) > 0, "first_num requires at least one item");
    return at(items, 0);
}

function num last_num(numlist items) {
    guard(count(items) > 0, "last_num requires at least one item");
    return at(items, count(items) - 1);
}

function text first_text(textlist items) {
    guard(count(items) > 0, "first_text requires at least one item");
    return at(items, 0);
}

function text last_text(textlist items) {
    guard(count(items) > 0, "last_text requires at least one item");
    return at(items, count(items) - 1);
}

function num contains_num(numlist items, num wanted) {
    each item in items {
        if (item == wanted) {
            return 1;
        }
    }
    return 0;
}

function num contains_text(textlist items, text wanted) {
    each item in items {
        if (same(item, wanted)) {
            return 1;
        }
    }
    return 0;
}

function text join_lines(textlist items) {
    sharp {
        var out = "";
        each item in items {
            out = join(out, item);
            out = join(out, "\n");
        }
        return out;
    }
}

function text join_with(textlist items, text separator) {
    sharp {
        var out = "";
        var first = 1;
        each item in items {
            if (first == 0) {
                out = join(out, separator);
            }
            out = join(out, item);
            first = 0;
        }
        return out;
    }
}
