package std;
module tensor;

function num tensor3_index(num y_size, num z_size, num x, num y, num z) {
    return (((x * y_size) + y) * z_size) + z;
}

function num tensor3_at(numlist values, num y_size, num z_size, num x, num y, num z) {
    return at(values, tensor3_index(y_size, z_size, x, y, z));
}
