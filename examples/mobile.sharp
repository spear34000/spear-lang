app {
    val screen = mobile_screen(
        "MainScreen",
        "/mobile",
        "Spear Mobile",
        ui_states1(
            ui_state_text_value("status_text", "Fast")
        ),
        mobile_column3(
            mobile_title("Spear Mobile"),
            mobile_text("Write simple screen code in Spear, then export shared UI to Android Compose."),
            mobile_card(
                "Status",
                mobile_stat("Build speed", "Fast")
            )
        )
    );

    write("build/MainScreen.kt", mobile_android("MainScreen", screen));
    write("build/DesktopMain.kt", mobile_desktop("DesktopMain", screen));
    show(mobile_android("MainScreen", screen));
}
