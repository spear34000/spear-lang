app {
    val home = ui_screen_route(
        "HomeScreen",
        "/workspace",
        "Spear Workspace",
        ui_states2(
            ui_state_text_value("workspace_name", "Nova"),
            ui_state_text_value("search_query", "")
        ),
        ui_column3(
            ui_toolbar(
                "Workspace",
                ui_items2(
                    ui_link("Back", "/"),
                    ui_link("Docs", "/docs")
                ),
                ui_items1(
                    ui_button("Invite", ui_action_open_dialog("Invite team"))
                )
            ),
            ui_section(
                "Workspace",
                ui_column2(
                    ui_card(
                        "Project Setup",
                        ui_form_submit3(
                            "workspace_form",
                            ui_action_submit("save_workspace"),
                            ui_input_required("workspace_name", "Workspace name", "Type a name", "text", "Shown in the sidebar and release notes."),
                            ui_input_with_kind("search_query", "Search prompt", "Search docs or files", "search", "Connected to the quick search slot."),
                            ui_button("Save changes", ui_action_submit("save_workspace"))
                        )
                    ),
                    ui_dialog(
                        "Invite team",
                        ui_column2(
                            ui_text("This overlay keeps action metadata structured."),
                            ui_button("Close", ui_action_close_dialog("Invite team"))
                        )
                    )
                )
            ),
            ui_section(
                "States",
                ui_column3(
                    ui_loading("Handlers attach to structured action slots here"),
                    ui_empty("No tasks yet", "Use this when a collection has no records."),
                    ui_error("Validation failed", "Use this when a request or input step fails.")
                )
            )
        )
    );

    write("build/ui-native.html", ui_web(home));
    show(ui_web(home));
}
