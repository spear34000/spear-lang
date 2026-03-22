# Rust Migration

Sharp is starting a staged migration from C to Rust.

Current direction:
- Keep the existing C compiler and runtime stable
- Migrate user-facing layers first
- Move parser/frontend/runtime pieces in smaller verified slices

Current Rust workspace:
- `crates/sharp-common/`: shared path, project, and environment helpers
- `crates/sharp/`: Rust CLI entrypoint compatible with the current `sharp` flow

Current integration:
- `scripts/build_rust.cmd` builds `build/sharp-rs.exe` when Cargo is available
- `sharp.cmd` prefers `build/sharp-rs.exe` if it exists
- `spear.cmd` can also forward to the Rust CLI for compatibility

What is already mirrored in Rust:
- `sharp new <name>`
- `sharp check [file.sp|folder]`
- `sharp build [file.sp|folder]`
- `sharp serve [file.sp|folder]`
- project discovery via `sharp.toml` or legacy `spear.toml`
- fallback to bundled `sharpc.exe` / `spearc.exe`
- fallback to bundled MinGW `gcc.exe`

What still remains in C:
- compiler frontend and parser
- code generation
- setup wizard
- runtime prelude and native helpers

Recommended next migration order:
1. diagnostics and message layer
2. import/source loading
3. lexer
4. frontend scan
5. parser
6. code generation
