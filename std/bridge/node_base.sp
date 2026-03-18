package std;
module nodelib;

function text node_json(text script, text fn_name, text payload) {
    return nodecall(script, fn_name, payload);
}

function text node_empty(text script, text fn_name) {
    return node_json(script, fn_name, "{}");
}
