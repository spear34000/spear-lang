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

- `value` means "make a new read-only value"
- `variable` means "make a new mutable value"
- `const name = ...;` now works directly without `const let`
- beginner-friendly aliases work too: `number`, `string`, `numbers`, `strings`, `mutable`
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
- beginner aliases like `run { ... }`, `print(...)`, and `ask(...)` reduce boilerplate

## Standard Library

Spear now ships with a structured `std/` library.

Public entrypoints:

- `std/prelude.sp`: recommended starter import for app code
- `std/math.sp`: numeric helpers
- `std/text.sp`: text composition helpers
- `std/lists.sp`: collection helpers
- `std/io.sp`: console and file wrappers
- `std/paths.sp`: path and build-output helpers
- `std/assert.sp`: reusable validation helpers
- `std/bridge.sp`: Node/Python bridge wrappers
- `std/web.sp`: reusable `view` helpers for UI output

Internal organization:

- `std/math/`: arithmetic and comparison helpers
- `std/text/`: text builders and layout helpers
- `std/collections/`: list-focused utilities
- `std/io/`: console and file helpers
- `std/system/`: path and assertion helpers
- `std/bridge/`: bridge-specific wrappers
- `std/web/`: page, content, and action views

Web building blocks now cover more of a real page:

- layout: `page_shell`, `page_frame`, `container`, `section_block`, `article_block`, `panel`, `card`, `split`, `stack_block`, `header_bar`, `footer_note`
- content: `heading1`, `heading2`, `heading3`, `paragraph`, `lead`, `hero`, `intro`, `note`, `success_note`, `warning_note`, `item`, `bullet_list`, `number_list`, `quote_block`, `code_line`, `metric`, `empty_state`, `faq_item`, `badge`
- actions: `actions`, `action_bar`, `primary_action`, `secondary_action`, `button_row`, `link_row`, `nav_bar`, `cta_section`, `docs_cta`
- attribute helpers: `attr`, `attrs`, `class_name`, `id_name`, `style_attr`, `href_attr`, `src_attr`, `alt_attr`
- compose-like modifiers: `modifier`, `padding_all`, `padding_x`, `padding_y`, `margin_all`, `margin_x`, `margin_y`, `gap_space`, `background`, `foreground`, `corner_radius`, `border`, `fill_max_width`, `fill_max_height`, `width_fixed`, `max_width`, `min_width`, `height_fixed`, `min_height`, `center_x`, `align_center`, `justify_center`, `justify_between`, `shadow`, `font_size`, `font_weight`, `flex_col`, `flex_row`, `wrap`
- compose-like blocks: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, `row_box(...) { ... }`, `label_text`, `button_link`, `image_block`, `spacer`

Beginner mode automatically includes `std/prelude.sp`, so the smallest programs can start immediately:

```sp
run {
    print("hello");
}
```

Clear data shapes:

- `number` or `num`: one integer value
- `string` or `text`: one text value
- `numbers` or `numlist`: a list of integer values
- `strings` or `textlist`: a list of text values
- `value` or `let`: inferred read-only binding
- `variable`, `mutable`, or `var`: inferred mutable binding
- `const`: explicit constant binding, with or without a type

Clear declaration rules:

- `value name = ...;`
  - inferred type, read-only
- `variable name = ...;`
  - inferred type, mutable
- `const name = ...;`
  - inferred type, constant
- `number name = ...;`
  - explicit number
- `string name = ...;`
  - explicit string
- `numbers name = ...;`
  - explicit number list
- `strings name = ...;`
  - explicit string list
- `const number name = ...;`
  - explicit constant with a fixed type

Clear function rules:

- `function number add(number left, number right) { ... }`
  - number-returning function
- `function string title_line(string title) { ... }`
  - string-returning function
- `function view hero(string title, string body) { ... }`
  - reusable UI/view function
- `run { ... }`
  - simplest app entry point
- `spear launch() { ... }`
  - named entry point when you want a formal app function

That means beginner code can read like this:

```sp
run {
    numbers scores = pack(10, 20, 30);
    const title = "Spear";
    variable shots = 3;
    print(count(scores));
    print(title);
}
```

Use direct modules only when you want a narrower surface:

```sp
import "../std/math.sp";
import "../std/web.sp";
```

Rule of thumb:

- use `escape(...)` on untrusted text
- use `markup(...)` to wrap already-safe HTML fragments
- use `page(...)` to emit a full HTML document

## Example

```sp
package examples;
module hello;

run {
    numbers scores = pack(add(4, 5), 12, 20);
    strings words = pack("spear", "edge");
    variable shots = clamp_num(3, 0, 10);

    print(count(scores));
    print(at(scores, 0));
    print(prefix("sharp-", at(words, 0)));
    print(sum(scores));
    print(size(repeat("=", 5)));
    for (var i = 0; i < count(words); i = i + 1) {
        print(at(words, i));
    }

    while (shots > 0) {
        print(shots);
        shots = dec(shots);
    }
}
```

UI example:

```sp
function view hero_block(string title, string body) {
    return surface_box(corner_radius("24px")) {
        column_box(gap_space("14px")) {
            badge("Starter");
            hero(title, body);
            button_row("/start", "Open App", "#docs", "Docs");
        };
    };
}

run {
    const title = "Spear UI";
    string html = page_frame(
        title,
        centered(header_bar(title, "safe, fast, reliable")),
        centered(column_box(modifier(max_width("960px"), gap_space("24px"))) {
            nav_bar("/", "Home", "#docs", "Docs", "#about", "About");
            hero_block(title, "safe, fast, reliable");
            cta_section("Start", "Open the app or read the docs.", "/start", "Open App");
        }),
        footer_note("Generated with Spear")
    );
    guard(size(html) > 0, "body must not be empty");
    write("build/spear-ui.html", html);
    print(html);
}
```

## Language Features

- `spear <name>() { ... }` entry function
- `function number <name>(...)`, `function string <name>(...)`, `function view <name>(...)` functions
- `run { ... }` beginner entry block
- `value`, `variable`, and `const` declaration forms
- `number`, `string`, `numbers`, `strings`, `mutable` beginner aliases
- `const` immutable bindings
- `num`, `text`, `numlist`, `textlist` also still work
- `import`, `module`, `package`
- `say(...)`, `print(...)`
- `write(path, content)`
- `nodecall(pkg, fn, payload)`, `pycall(module, fn, payload)`
- `read()`, `ask(prompt)`
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
- `std/`: bundled standard library with public entrypoints + internal submodules
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
