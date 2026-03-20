import "../std/net.sp";

app {
    show(tcp_request("127.0.0.1", 46071, "ping"));
    show(udp_send("127.0.0.1", 46072, "pong"));
}
