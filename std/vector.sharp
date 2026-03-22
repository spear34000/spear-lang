package std;
module vector;

function num vector_dot(numlist left, numlist right) {
    guard(count(left) == count(right), "vector_dot requires equal lengths");
    variable total = 0;
    variable i = 0;
    while (i < count(left)) {
        total = total + (at(left, i) * at(right, i));
        i = i + 1;
    }
    return total;
}

function text vector_add(numlist left, numlist right) {
    return python_json("spear_std", "vector_add", json_object2(
        json_field("left", json_array_numbers(left)),
        json_field("right", json_array_numbers(right))
    ));
}

function num vector_sum_of_squares(numlist values) {
    variable total = 0;
    variable i = 0;
    while (i < count(values)) {
        total = total + (at(values, i) * at(values, i));
        i = i + 1;
    }
    return total;
}
