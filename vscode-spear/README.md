# Sharp Language Extension

Syntax highlighting, snippets, language configuration, and live syntax diagnostics for `sharp` source files.

- File extension: `.sp`
- Language id: `spear` (compatibility id for `.sp`)
- Core memory concept: `sharp`
- Real-time syntax check: `build/sharpc.exe --check-stdin <current-file>`
- The editor check path does not create temporary `.sp` files next to your source
- Optional setting: `spear.compilerPath`
- The Windows setup wizard can install this extension as an unpacked local extension

Open this folder as a VS Code extension project or package it with `vsce`.
