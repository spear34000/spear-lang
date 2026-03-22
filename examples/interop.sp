import "../std/interop.sp";
import "../std/json.sp";

run {
    value py = pip_module("demo_python");
    value js = npm_module("./demo_node.cjs");

    print(py_call(py, "render", json_object1(json_field("name", json_text("Sharp")))));
    print(js_call(js, "render", json_object1(json_field("name", json_text("Sharp")))));
}
