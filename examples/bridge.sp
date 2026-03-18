package examples;
module bridge;

spear launch() {
    say(nodecall("./demo_node.cjs", "render", "{\"name\":\"Spear\"}"));
    say(pycall("demo_python", "render", "{\"name\":\"Spear\"}"));
}
