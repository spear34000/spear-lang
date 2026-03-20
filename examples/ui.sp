app {
    val home = ui_screen(
        "Spear Workspace",
        ui_states2(
            ui_state_text_value("name", "Nova"),
            ui_state_num("count", 3)
        ),
        ui_column5(
            ui_nav3(
                ui_link("Home", "/"),
                ui_link("Docs", "/docs"),
                ui_link("Settings", "/settings")
            ),
            ui_title("Build product screens with meaning first."),
            ui_notice("System", "Describe the screen as state, inputs, actions, and sections instead of raw tags.", "info"),
            ui_row2(
                ui_card(
                    "Editor",
                    ui_form3(
                        ui_input("name", "Workspace name", "Type a name"),
                        ui_button("Save", "save_workspace"),
                        ui_link("Open docs", "/docs")
                    )
                ),
                ui_card(
                    "Signals",
                    ui_column2(
                        ui_stat("Projects", "12"),
                        ui_stat("Sync", "Healthy")
                    )
                )
            ),
            ui_section(
                "Next",
                ui_column3(
                    ui_loading("State actions will attach here"),
                    ui_empty("No tasks yet", "Use this state when a collection has no records."),
                    ui_error("Action failed", "Use this state when a request or validation step fails.")
                )
            )
        )
    );

    write("build/ui.html", ui_web(home));
    write("build/HomeScreen.kt", ui_android("HomeScreen", home));
    show(ui_web(home));
}
