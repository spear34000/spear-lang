package std;
module matrix;

function num matrix_index(num columns, num row_index, num column_index) {
    return (row_index * columns) + column_index;
}

function num matrix_at(numlist values, num columns, num row_index, num column_index) {
    return at(values, matrix_index(columns, row_index, column_index));
}

function num matrix_row_sum(numlist values, num columns, num row_index) {
    variable total = 0;
    variable column_index = 0;
    while (column_index < columns) {
        total = total + matrix_at(values, columns, row_index, column_index);
        column_index = column_index + 1;
    }
    return total;
}
