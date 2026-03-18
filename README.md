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
- `write(path, content)` makes page/app output generation practical

Rule of thumb:

- use `escape(...)` on untrusted text
- use `markup(...)` to wrap already-safe HTML fragments
- use `page(...)` to emit a full HTML document

## Example

```sp
num add(num left, num right) {
    return left + right;
}

text sharpen(text base) {
    sharp {
        let prefix = "sharp-";
        return join(prefix, base);
    }
}

spear launch() {
    numlist scores = pack(add(4, 5), 12, 20);
    textlist words = pack("spear", "edge");
    let shots = 3;

    say(count(scores));
    say(at(scores, 0));
    say(sharpen(at(words, 0)));

    while (shots > 0) {
        say(shots);
        shots = shots - 1;
    }
}
```

UI example:

```sp
view hero(text title, text body) {
    return stack(
        markup("h1", escape(title)),
        stack(
            markup("p", escape(body)),
            action("/start", "Open App")
        )
    );
}

spear launch() {
    const let title = "Spear UI";
    let body = hero(title, "safe, fast, reliable");
    guard(size(body) > 0, "body must not be empty");
    text html = page(title, body);
    write("build/spear-ui.html", html);
    say(html);
}
```

## Language Features

- `spear <name>() { ... }` entry function
- `num <name>(...)`, `text <name>(...)`, `view <name>(...)` functions
- `let` type inference for scalar values
- `const` immutable bindings
- `num`, `text`, `numlist`, `textlist`
- `say(...)`
- `write(path, content)`
- `join(a, b)`, `read()`, `size(text)`, `same(a, b)`
- `guard(condition, message)`
- `escape(text)`, `markup(tag, html)`, `page(title, body)`
- `stack(a, b)`, `inline(a, b)`, `action(href, label)`
- `pack(...)`, `push(list, value)`, `count(list)`, `at(list, index)`
- `each item in list { ... }`
- `sharp { ... }`
- `if`, `else`, `while`, `return`

## Build

```bash
make
make example
```

Run like Python:

```bash
spear examples/hello.sp
spear build examples/web.sp
```

On Windows this is provided by [spear.cmd](/c:/Users/User/Desktop/spear/spear.cmd#L1).

Manual build:

```bash
gcc -O3 -Wall -Wextra -std=c11 -o build/spearc.exe src/spearc.c
build/spearc.exe examples/hello.sp -o build/hello_sp.c
gcc -O3 -Wall -Wextra -std=c11 -o build/hello_sp.exe build/hello_sp.c
build/hello_sp.exe
```

## Workspace

- `src/`: compiler implementation
- `examples/`: sample Spear programs
- `examples/web.sp`: safe HTML/UI-flavored output example
- `build/`: generated C and binaries
- `vscode-spear/`: VS Code extension files

## VS Code Extension

The `vscode-spear/` folder contains:

- syntax highlighting
- snippets
- language configuration
- logo asset

Open that folder as an extension project or package it with `vsce`.
