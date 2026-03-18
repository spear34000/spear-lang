# spear

`spear` is a small compiled language implemented in C.

- Source extension: `.sp`
- Core memory concept: `sharp`
- Build model: AOT translation from `spear` to C

## Sharp Memory

`sharp { ... }` creates a temporary memory region.

- `text` values created inside the block live in that region.
- Leaving the block releases those temporary strings together.
- You do not manage `malloc` and `free` directly in user code.

## Easier Spear

Recent syntax improvements focus on making the language easier to write:

- `let` infers scalar variable types from the initializer
- `pack(...)` can create lists with initial values
- user functions make larger programs practical
- `const` blocks accidental reassignment
- `guard(...)` makes fail-fast contracts cheap
- `escape(...)`, `markup(...)`, and `page(...)` make safe HTML output simpler
- `view` makes UI components read like components instead of plain text helpers
- `each item in list { ... }` cuts down list boilerplate
- `var` is the short mutable binding form
- `for`, `break`, and `continue` cover the standard control-flow baseline
- `import "file.sp"` lets a program split across local Spear files
- `module`, `package`, and `function` make multi-file code read like a real language
- `nodecall(...)` and `pycall(...)` connect Spear to Node/npm and Python/pip code through JSON payloads
- `write(path, content)` makes page/app output generation practical
- checked arithmetic traps division-by-zero and 64-bit overflow with source locations
- list bounds and `guard(...)` failures report `line:column` runtime locations

Rule of thumb:

- use `escape(...)` on untrusted text
- use `markup(...)` to wrap already-safe HTML fragments
- use `page(...)` to emit a full HTML document

## Example

```sp
import "math.sp";

package examples;
module hello;

spear launch() {
    numlist scores = pack(add(4, 5), 12, 20);
    textlist words = pack("spear", "edge");
    var shots = 3;

    say(count(scores));
    say(at(scores, 0));
    say(sharpen(at(words, 0)));
    for (var i = 0; i < count(words); i = i + 1) {
        say(at(words, i));
    }

    while (shots > 0) {
        say(shots);
        shots = shots - 1;
    }
}
```

UI example:

```sp
view hero(text title, text body) {
    return markup("section") {
        markup("h1") {
            escape(title);
        };
        markup("p") {
            escape(body);
        };
        row {
            action("/start", "Open App");
            action("#docs", "Docs");
        };
    };
}

spear launch() {
    const let title = "Spear UI";
    text html = page(title) {
        column {
            hero(title, "safe, fast, reliable");
        };
    };
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    say(html);
}
```

## Language Features

- `spear <name>() { ... }` entry function
- `function num <name>(...)`, `function text <name>(...)`, `view <name>(...)` functions
- `let` and `var` type inference for scalar values
- `const` immutable bindings
- `num`, `text`, `numlist`, `textlist`
- `import`, `module`, `package`
- `say(...)`
- `write(path, content)`
- `nodecall(pkg, fn, payload)`, `pycall(module, fn, payload)`
- `join(a, b)`, `read()`, `size(text)`, `same(a, b)`
- `guard(condition, message)`
- `escape(text)`, `markup(tag, html)`, `page(title, body)`
- block UI builders: `markup(tag) { ... }`, `page(title) { ... }`, `column { ... }`, `row { ... }`
- `stack(a, b)`, `inline(a, b)`, `action(href, label)`
- `pack(...)`, `push(list, value)`, `count(list)`, `at(list, index)`
- `each item in list { ... }`
- `sharp { ... }`
- `if`, `else`, `for`, `while`, `break`, `continue`, `return`, `throw`

## Build

```bash
make
make example
make check
make dist
```

`make check` also runs `scripts/check_regressions.cmd` to cover loop-scope cleanup, imported-source line mapping, and declarative UI block syntax.

Windows installer artifacts can be rebuilt with `scripts/build_installers.cmd` and are copied to `dist/installers/`.

Run like Python:

```bash
build/spear.exe examples/hello.sp
build/spear.exe build examples/web.sp
build/spear.exe serve examples/web.sp
build/spear.exe check examples/hello.sp
```

- `spear file.sp`: runs from a temporary native build and does not leave `.c` or `.exe` output behind
- `spear serve file.sp`: also uses a temporary native build, but your Spear program can still write app output like `build/spear-ui.html`
- `spear build file.sp`: writes persistent `build/<name>.c` and `build/<name>.exe`
- `spear check file.sp`: validates syntax, imports, and top-level structure without generating native output
- editor diagnostics use `spearc --check-stdin <file.sp>`, so unsaved checks do not leave temporary `.sp` files beside your source

`spear.exe` hides raw GCC output from the terminal.

- Spear compile failures show a Spear-style message and write details to `build/*.spearc.log`
- native backend failures write details to `build/*.native.log`
- runtime failures print `spear runtime error: ...`

Bridge example:

```sp
spear launch() {
    say(nodecall("./demo_node.cjs", "render", "{\"name\":\"Spear\"}"));
    say(pycall("demo_python", "render", "{\"name\":\"Spear\"}"));
}
```

`nodecall` and `pycall` take a JSON string, call an external function, and return text.
That gives Spear a safe extension story without turning the whole language into unchecked native FFI.

For a global command, install with `build/spear-setup.exe`.
The setup executable now behaves like a small install wizard:

- real Windows wizard UI with `Back`, `Next`, `Install`, and `Close`
- welcome step
- tool detection for `gcc`, `node`, `python`, and VS Code
- optional user `PATH` registration
- optional bundled example workspace install
- optional unpacked VS Code extension install
- runtime bridge + editor asset copy into `%LOCALAPPDATA%\Programs\Spear`
- uninstall metadata + install manifest registration
- optional post-install self-check
- completion step with next commands

After installation:

```bash
spear examples/hello.sp
spear build examples/web.sp
spear serve examples/web.sp
spear check examples/hello.sp
```

Manual build:

```bash
make spear
make setup
make dist
build/spear.exe examples/hello.sp
```

## Workspace

- `src/`: compiler implementation
- `runtime/`: Node and Python bridge scripts for external ecosystem access
- `examples/`: sample Spear programs
- `examples/bridge.sp`: bridge example for Node and Python calls
- `examples/web.sp`: safe HTML/UI-flavored output example
- `build/`: generated C and binaries
- `build/dist/`: installer-ready package layout
- `build/spear.exe`: Python-like Spear launcher
- `build/spear-setup.exe`: Windows installer
- `dist/installers/`: packaged `SpearSetup.exe` and `SpearSetup.msi`
- `vscode-spear/`: VS Code extension files

## VS Code Extension

The `vscode-spear/` folder contains:

- syntax highlighting
- snippets
- language configuration
- real-time syntax diagnostics via `spearc --check`
- logo asset

Open that folder as an extension project or package it with `vsce`.
