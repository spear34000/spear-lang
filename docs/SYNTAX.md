# Sharp Syntax

**English**  
This file is a compact syntax guide for Sharp source files (`.sharp`).

**한국어**  
이 문서는 Sharp 소스 파일(`.sharp`)을 위한 빠른 문법 가이드입니다.

## 1. Smallest Program

```sharp
run {
    print("hello");
}
```

- `run { ... }`: simplest program entry
- `print(...)` / `show(...)`: output text

## 2. Values and Variables

```sharp
run {
    const title = "Sharp";
    value score = 10;
    variable shots = 3;

    print(title);
    print(string(score));
    print(string(shots));
}
```

- `const`: constant
- `value`: inferred read-only value
- `variable`: inferred mutable value

Short aliases:

- `val` = `value`
- `mut` = `variable`
- `show` = `print`
- `app` = `run`

## 3. Types

```sharp
run {
    number count = 3;
    string title = "Sharp";
    numbers scores = pack(10, 20, 30);
    strings names = pack("A", "B");
}
```

Main built-in surface types:

- `number`
- `string`
- `numbers`
- `strings`
- `map`
- `result`

Also accepted:

- `num`
- `text`
- `numlist`
- `textlist`
- `str`
- `nums`
- `texts`

## 4. Functions

```sharp
function number add(number left, number right) {
    return left + right;
}

function string greet(string name) {
    return join("hello ", name);
}

run {
    print(string(add(3, 4)));
    print(greet("Sharp"));
}
```

- `function`: explicit function declaration
- `fn`: short alias

## 5. Conditions and Loops

```sharp
run {
    variable shots = 3;

    if (shots > 0) {
        print("ready");
    } else {
        print("empty");
    }

    while (shots > 0) {
        print(string(shots));
        shots = shots - 1;
    }
}
```

Also supported:

- `for (...) { ... }`
- `each item in list { ... }`
- `break`
- `continue`

## 6. Lists

```sharp
run {
    numbers scores = pack(4, 5, 6);
    print(string(count(scores)));
    print(string(at(scores, 0)));
}
```

Common functions:

- `pack(...)`
- `count(list)`
- `at(list, index)`
- `push(list, value)`

## 7. Maps

```sharp
run {
    map state = map();
    put(state, "mode", "sharp");
    print(get(state, "mode", "missing"));
    print(string(count(state)));
}
```

Common functions:

- `map()`
- `put(map, key, value)`
- `get(map, key, fallback)`
- `has(map, key)`
- `drop(map, key)`
- `count(map)`

## 8. Results

```sharp
function result load_name() {
    return ok("Sharp");
}

run {
    result current = load_name();
    if (is_ok(current)) {
        print(unwrap(current));
    } else {
        print(error_text(current));
    }
}
```

Common functions:

- `ok(text)`
- `fail(text)`
- `is_ok(result)`
- `unwrap(result)`
- `error_text(result)`

## 9. Sharp Blocks

```sharp
run {
    value label = sharp text {
        value prefix = "sharp-";
        defer print("leaving sharp");
        emit prefix;
        emit "note";
    };

    print(label);
}
```

Sharp ideas:

- `sharp { ... }`: temporary scope
- `sharp text { ... }`: build a string value
- `sharp map { ... }`
- `sharp numbers { ... }`
- `keep ...;`: move final value out
- `emit ...;`: append text in `sharp text`
- `defer ...;`: run cleanup on exit

## 10. Imports

Installed or project-local code can import standard modules like this:

```sharp
import "std/prelude.sharp";
import "std/web.sharp";
```

Workspace-relative imports also work:

```sharp
import "../std/math.sharp";
```

## 11. Web Output

```sharp
import "std/web.sharp";

run {
    const title = "Sharp Web";
    string html = landing_page(title, theme_product_bg()) {
        centered(box(glass_panel_mod()) {
            markup("h1") { "Sharp"; };
            markup("p") { "Readable UI blocks"; };
        });
    };
    write("build/app.html", html);
}
```

Useful web modules:

- `std/web.sharp`
- `std/ui.sharp`
- `std/mobile.sharp`

## 12. Shared UI Model

```sharp
import "std/ui.sharp";

run {
    string screen = ui_screen(
        "HomeScreen",
        "/",
        "Home",
        ui_column2(
            ui_title("Sharp"),
            ui_text("One screen, multiple outputs")
        )
    );

    write("build/ui.json", screen);
}
```

Core UI ideas:

- semantic screen model
- shared layout/content/input nodes
- web / android / desktop output

## 13. Interop

```sharp
import "std/interop.sharp";

run {
    value py = pip_module("requests");
    value js = npm_module("dayjs");
    print("interop ready");
}
```

CLI:

```bash
sharp add pip requests
sharp add npm dayjs
```

See also:

- [`INTEROP.md`](./INTEROP.md)

## 14. Project Files

Typical project:

```text
my-app/
  sharp.toml
  main.sharp
```

Typical commands:

```bash
sharp new my-app
sharp check
sharp build
sharp serve
```

## 15. Notes

- Primary extension: `.sharp`
- `sharp` without arguments shows usage
- `sharp .` runs the current project folder
- Older `.sp` files may still work through compatibility paths, but `.sharp` is the default surface
