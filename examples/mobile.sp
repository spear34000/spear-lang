app {
    val screen = android_screen(
        "MainScreen",
        "0xFFF4F1EA",
        android_column(
            ui4(
                android_title("Spear Mobile"),
                android_body("Write simple screen code in Spear, then export a Compose screen."),
                android_card("Status", android_stat("Build speed", "Fast")),
                android_button("Open app")
            )
        )
    );

    write("build/MainScreen.kt", screen);
    show(screen);
}
