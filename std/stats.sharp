package std;
module stats;

function num stats_sum(numlist values) {
    variable total = 0;
    variable i = 0;
    while (i < count(values)) {
        total = total + at(values, i);
        i = i + 1;
    }
    return total;
}

function num stats_mean(numlist values) {
    guard(count(values) > 0, "stats_mean requires at least one value");
    return stats_sum(values) / count(values);
}

function num stats_min(numlist values) {
    guard(count(values) > 0, "stats_min requires at least one value");
    variable best = at(values, 0);
    variable i = 1;
    while (i < count(values)) {
        if (at(values, i) < best) {
            best = at(values, i);
        }
        i = i + 1;
    }
    return best;
}

function num stats_max(numlist values) {
    guard(count(values) > 0, "stats_max requires at least one value");
    variable best = at(values, 0);
    variable i = 1;
    while (i < count(values)) {
        if (at(values, i) > best) {
            best = at(values, i);
        }
        i = i + 1;
    }
    return best;
}

function num stats_range(numlist values) {
    return stats_max(values) - stats_min(values);
}
