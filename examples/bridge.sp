package examples;
module bridge;

run {
    print(node_json("./demo_node.cjs", "render", "{\"name\":\"Spear\"}"));
    print(python_json("demo_python", "render", "{\"name\":\"Spear\"}"));
}
