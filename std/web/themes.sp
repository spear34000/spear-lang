package std;
module webthemes;

function text theme_cinema_bg() {
    return "radial-gradient(circle at top left, #2347a8 0, rgba(35,71,168,0.18) 22%, transparent 45%),radial-gradient(circle at 85% 15%, #ff8a3d 0, rgba(255,138,61,0.16) 18%, transparent 40%),linear-gradient(180deg, #08111f 0%, #0d1728 52%, #f3efe6 52%, #f4f1ea 100%)";
}

function text theme_product_bg() {
    return "radial-gradient(circle at 20% 0%, rgba(94,197,255,0.18) 0, transparent 28%),radial-gradient(circle at 100% 10%, rgba(255,156,103,0.18) 0, transparent 30%),linear-gradient(180deg, #0a1220 0%, #111b2f 48%, #f6f3ec 48%, #f6f2ea 100%)";
}

function text theme_editorial_bg() {
    return "linear-gradient(180deg, #10151d 0%, #171f2b 46%, #f6f1e8 46%, #f5efe5 100%)";
}

function text theme_page(text background_fill) {
    return join("min-height:100vh;background:", join(background_fill, ";color:#ecf3ff"));
}
