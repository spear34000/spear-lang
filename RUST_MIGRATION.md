# Rust Migration

Sharp is starting a staged migration from C to Rust.

Current direction:
- Keep the existing C compiler and runtime stable
- Migrate user-facing layers first
- Move parser/frontend/runtime pieces in smaller verified slices

Current Rust workspace:
- `crates/sharp-common/`: shared path, project, and environment helpers
- `crates/sharp/`: Rust CLI entrypoint compatible with the current `sharp` flow
- `crates/sharpc/`: Rust compiler entrypoint wrapper in front of the legacy C backend

Current integration:
- `scripts/build_rust.cmd` builds `build/sharp.exe` and `build/sharpc.exe`
- `sharp.cmd` and `spear.cmd` use the Rust CLI by default
- the legacy C compiler is kept as `sharpc-c.exe` and packaged under `runtime/`

What is already mirrored in Rust:
- `sharp new <name>`
- `sharp check [file.sharp|folder]`
- `sharp build [file.sharp|folder]`
- `sharp serve [file.sharp|folder]`
- `sharpc ...` user-facing compiler entrypoint
- project discovery via `sharp.toml` or legacy `spear.toml`
- fallback to bundled `sharpc.exe` / `spearc.exe`
- fallback to bundled MinGW `gcc.exe`

What still remains in C:
- compiler frontend and parser backend implementation
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

