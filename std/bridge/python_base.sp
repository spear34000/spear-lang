package std;
module pythonlib;

function text python_json(text module_name, text fn_name, text payload) {
    return pycall(module_name, fn_name, payload);
}

function text python_text(text module_name, text fn_name, text payload_text) {
    return pycall(module_name, fn_name, payload_text);
}

function text python_empty(text module_name, text fn_name) {
    return python_json(module_name, fn_name, "{}");
}
