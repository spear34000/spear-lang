# Changelog

All notable changes to Spear will be documented in this file.

## v0.2.0 - 2026-03-18

### Language

- Added beginner-friendly declaration forms: `const`, `value`, and `variable`.
- Added clearer type aliases: `number`, `string`, `numbers`, and `strings`.
- Added `run { ... }`, `print(...)`, and `ask(...)` for simpler entry and console code.
- Added multi-error checking and broader diagnostics for unused symbols, unreachable code, constant conditions, duplicate imports, and unused imports.
- Fixed function name collisions between user modules and standard library functions by generating package/module-aware internal symbols.

### Standard Library

- Added a structured `std/` library with public entrypoints for math, text, lists, io, paths, assert, bridge, and web.
- Added `std/prelude.sp` as the default beginner import surface.
- Expanded reusable web building blocks for layout, content, actions, and modifier-based styling.

### Web UI

- Added Compose-style web DSL blocks: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, and `row_box(...) { ... }`.
- Added modifier helpers for spacing, sizing, borders, colors, alignment, and flex layout.
- Added HTML attribute helpers such as `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, and `alt_attr`.
- Updated examples to demonstrate declarative, component-style page composition.

### Tooling

- Improved VS Code syntax highlighting and added the bundled `Spear Dark` theme.
- Improved live diagnostics in the VS Code extension and compiler path resolution for installed builds.
- Added installer language selection for English and Korean, with localized runtime and launcher error messages.
- Fixed Windows build and launcher issues in `Makefile` and `spear.cmd`.

### Release Artifacts

- Published Windows installer artifacts: `SpearSetup.exe` and `SpearSetup.msi`.
- Published VS Code extension package: `spear-language-0.2.0.vsix`.
