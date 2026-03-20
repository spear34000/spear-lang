app {
    val home = ui_screen_route(
        "HomeScreen",
        "/workspace",
        "Spear Workspace",
        ui_states3(
            ui_state_text_value("workspace_name", "Nova"),
            ui_state_text_value("search_query", ""),
            ui_state_num("open_tasks", 3)
        ),
        ui_column5(
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
            ui_tabs3(
                ui_link("Overview", "/workspace"),
                ui_link("Forms", "/workspace/forms"),
                ui_link("Activity", "/workspace/activity")
            ),
            ui_section(
                "Workspace",
                ui_column3(
                    ui_row2(
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
                        ui_card(
                            "Signals",
                            ui_column3(
                                ui_stat("Projects", "12"),
                                ui_stat("Sync", "Healthy"),
                                ui_list2(
                                    ui_item("Recent", ui_text("Schema locked for web, Android, and desktop outputs.")),
                                    ui_item("Queue", ui_text("3 review tasks are ready for the next action slot."))
                                )
                            )
                        )
                    ),
                    ui_dialog(
                        "Invite team",
                        ui_column2(
                            ui_text("This overlay keeps action metadata structured without forcing runtime mutation yet."),
                            ui_button("Close", ui_action_close_dialog("Invite team"))
                        )
                    ),
                    ui_sheet(
                        "Queue",
                        ui_column2(
                            ui_text("Sheets are modeled as overlays too, so web and Compose consume the same node shape."),
                            ui_button("Open backlog", ui_go("/workspace/backlog"))
                        )
                    )
                )
            ),
            ui_divider(),
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

    write("build/ui.html", ui_web(home));
    write("build/HomeScreen.kt", ui_android("HomeScreen", home));
    write("build/DesktopScreen.kt", ui_desktop("DesktopScreen", home));
    show(ui_web(home));
}
