package std;
module test;

function num assert_true(num condition, text message) {
    guard(condition, message);
    return 1;
}

function num assert_num_eq(num left, num right, text message) {
    guard(left == right, message);
    return 1;
}

function num assert_text_eq(text left, text right, text message) {
    guard(same(left, right), message);
    return 1;
}
