# Spear

**English**  
Spear is a small compiled language implemented in C. It aims to stay approachable for beginners while still being structured enough for real tooling, installers, and web UI generation.

**한국어**  
Spear는 C로 구현한 작은 컴파일 언어입니다. 비전공자도 시작하기 쉽게 유지하면서, 실제 도구 체인과 설치기, 웹 UI 생성까지 다룰 수 있을 정도로 구조를 갖추는 것을 목표로 합니다.

- Source extension / 소스 확장자: `.sp`
- Memory concept / 메모리 개념: `sharp`
- Build model / 빌드 모델: Spear to C AOT translation

## Quick Start / 빠른 시작

**English**  
Small programs can start immediately because beginner mode automatically includes `std/prelude.sp`.

**한국어**  
초보자 모드에서는 `std/prelude.sp`가 자동 포함되므로 아주 작은 프로그램은 바로 시작할 수 있습니다.

```sp
run {
    print("hello");
}
```

## Core Syntax / 핵심 문법

**English**

- `const`: constant value
- `value`: inferred read-only value
- `variable`: inferred mutable value
- `fn`, `val`, `mut`, `str`, `nums`, `texts`, `show`, `app`: short aliases for faster writing
- `number`, `string`, `numbers`, `strings`: beginner-friendly type names
- `map`: native text map for keyed state
- `result`: native text result for success/error flow
- `text(number)` or `string(number)`: convert numbers to text for UI and charts
- `function`: explicit function declaration
- `run { ... }`: simplest program entry

**한국어**

- `const`: 상수
- `value`: 타입 추론이 되는 읽기 전용 값
- `variable`: 타입 추론이 되는 변경 가능한 값
- `number`, `string`, `numbers`, `strings`: 더 직관적인 타입 이름
- `text(number)` / `string(number)`: 숫자를 UI와 차트용 텍스트로 변환
- `function`: 명시적 함수 선언
- `run { ... }`: 가장 단순한 프로그램 시작점

```sp
run {
    numbers scores = pack(10, 20, 30);
    const title = "Spear";
    variable shots = 3;
    print(count(scores));
    print(title);
}
```

```sp
run {
    map state = map();
    put(state, "mode", "native");
    print(get(state, "mode", "missing"));
}
```

```sp
run {
    result job = ok("ready");
    print(is_ok(job));
    print(unwrap(job));
}
```

```sp
function result load_status(string mode) {
    if (same(mode, "ready")) {
        return ok("online");
    }
    return fail("offline");
}

run {
    result current = load_status("ready");
    print(unwrap(current));
}
```

```sp
function number add(number left, number right) {
    return left + right;
}
```

## Sharp Memory / Sharp 메모리

**English**  
`sharp { ... }` creates a temporary memory region. Text created inside the block is released together when the block ends.

`sharp text { ... }`, `sharp map { ... }`, and `sharp numbers { ... }` can also be used as value expressions. Use `keep ...;` inside the block to safely move the final value out of the temporary region.

`defer ...;` can be used inside `sharp` blocks to run cleanup or final actions right before the sharp scope closes.

**한국어**  
`sharp { ... }`는 임시 메모리 영역을 만듭니다. 블록 안에서 만든 텍스트는 블록이 끝날 때 함께 정리됩니다.

```sp
sharp {
    string label = join("sharp-", "text");
    print(label);
}
```

```sp
value label = sharp text {
    value prefix = "sharp-";
    defer print("leaving sharp");
    keep join(prefix, "text");
};
```

## Standard Library / 표준 라이브러리

**English**  
Spear ships with a structured `std/` library. For most programs, `std/prelude.sp` is the main starting surface.

**한국어**  
Spear는 구조화된 `std/` 표준 라이브러리를 함께 제공합니다. 대부분의 프로그램은 `std/prelude.sp`를 시작점으로 쓰면 됩니다.

Public entrypoints / 공개 진입점:

- `std/prelude.sp`
- `std/math.sp`
- `std/text.sp`
- `std/lists.sp`
- `std/io.sp`
- `std/paths.sp`
- `std/assert.sp`
- `std/bridge.sp`
- `std/json.sp`
- `std/map.sp`
- `std/result.sp`
- `std/fs.sp`
- `std/os.sp`
- `std/cli.sp`
- `std/net.sp`
- `std/http_server.sp`
- `std/csv.sp`
- `std/random.sp`
- `std/stats.sp`
- `std/vector.sp`
- `std/matrix.sp`
- `std/tensor.sp`
- `std/crypto.sp`
- `std/security.sp`
- `std/safe_fs.sp`
- `std/http.sp`
- `std/sqlite.sp`
- `std/logger.sp`
- `std/config.sp`
- `std/test.sp`
- `std/web.sp`
- `std/mobile.sp`
- `std/ui.sp`

Direct import / 직접 import:

```sp
import "../std/math.sp";
import "../std/web.sp";
```

Installed projects can also use:

```sp
import "std/web.sp";
```

## Web UI / 웹 UI

**English**  
Spear includes a safe HTML and UI layer with composable layout blocks, design tokens, forms, and chart primitives.

**한국어**  
Spear는 안전한 HTML/UI 레이어를 제공하며, 조합 가능한 레이아웃 블록과 디자인 토큰, 폼, 차트 primitive를 포함합니다.

Main web pieces / 주요 웹 구성 요소:

- Layout: `page_frame`, `container`, `section_block`, `split`, `header_bar`, `footer_note`
- Content: `heading1`, `heading2`, `paragraph`, `hero`, `metric`, `quote_block`, `badge`
- Actions: `primary_action`, `secondary_action`, `button_row`, `nav_bar`, `cta_section`
- Forms: `text_input`, `email_input`, `text_area`, `select_field`, `form_card`
- Charts: `metric_bars`, `comparison_chart`, `trend_line_card`, `timeline_feed`
- Attributes: `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, `alt_attr`
- Modifiers: `modifier`, `padding_all`, `padding_x`, `padding_y`, `background`, `foreground`, `corner_radius`, `border`, `gap_space`, `max_width`, `shadow`, `justify_center`, `justify_between`
- Compose-style blocks: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, `row_box(...) { ... }`

Examples / 예제:

- `examples/web.sp`: editorial landing page example
- `examples/charts.sp`: chart-first review board example
- `examples/platform.sp`: standard platform example for JSON, files, HTTP, SQLite, stats, and crypto
- `examples/security.sp`: security-first example for tokens, password hashing, secret env, and rooted file access
- `examples/mobile.sp`: Android Compose screen output example
- `examples/ui.sp`: single screen model rendered to web HTML and Android Compose
- `examples/map_demo.sp`: native map state example
- `examples/tool.sp`: CLI args plus map state example
- `examples/server.sp`: minimal HTML server example

Web example / 웹 예제:

```sp
run {
    const title = "Spear Signals";
    string html = page(title) {
        markup("main", style_attr("min-height:100vh;background:#f4f1ea;color:#141414")) {
            metric_bars(
                "Quality signals",
                "Keep the first pass readable and strong.",
                pack("Clarity", "Rhythm", "Freedom"),
                pack(93, 90, 88),
                pack("#141414", "#2f6fed", "#b76536")
            );
        };
    };
    write("build/signals.html", html);
}
```

## Diagnostics / 진단

**English**  
`spear check` and `spearc --check` report syntax and semantic errors without producing a native output file. The compiler also emits warnings for common issues.

**한국어**  
`spear check`와 `spearc --check`는 네이티브 출력 파일을 만들지 않고 문법 및 의미 오류를 검사합니다. 컴파일러는 자주 발생하는 문제에 대한 경고도 출력합니다.

Current checks / 현재 진단 항목:

- multi-error collection
- unused variable and unused import
- duplicate import
- unreachable code
- constant-condition warnings
- VS Code live diagnostics

## Build And Run / 빌드와 실행

```bash
make
make check
make dist
```

Project workflow / 프로젝트 흐름:

```bash
spear new my-app
cd my-app
spear check
spear build
spear serve
```

- `spear new <name>` creates a starter web project with `spear.toml` and `main.sp`
- `spear`, `spear build`, `spear serve`, and `spear check` work on the current project folder
- `import "std/web.sp";` resolves through the installed standard library, so projects can live anywhere

Run commands / 실행 명령:

```bash
build/spear.exe examples/hello.sp
build/spear.exe build examples/web.sp
build/spear.exe build examples/charts.sp
build/spear.exe build examples/map_demo.sp
build/spear.exe build examples/platform.sp
build/spear.exe build examples/tool.sp
build/spear.exe build examples/server.sp
build/spear.exe serve examples/web.sp
build/spear.exe check examples/hello.sp
```

Command behavior / 명령 동작:

- `spear file.sp`: temporary native build and run
- `spear serve file.sp`: temporary build plus local static serving
- `spear build file.sp`: writes persistent build artifacts
- `spear check file.sp`: validation only

## Installer / 설치기

**English**  
Install with `build/spear-setup.exe` or use the packaged installer artifacts in `dist/installers/`.

**한국어**  
`build/spear-setup.exe`로 설치하거나 `dist/installers/` 안의 패키지 결과물을 사용할 수 있습니다.

Installer features / 설치기 기능:

- Windows wizard UI
- English and Korean language selection
- optional PATH registration
- optional examples and VS Code extension install
- post-install self-check

Artifacts / 결과물:

- `dist/installers/SpearSetup.exe`
- `dist/installers/SpearSetup.msi`
- `vscode-spear/spear-language-0.1.0.vsix`

## VS Code Extension / VS Code 확장

**English**  
The `vscode-spear/` folder contains syntax highlighting, snippets, diagnostics, and the bundled `Spear Dark` theme.

**한국어**  
`vscode-spear/` 폴더에는 문법 강조, 스니펫, 진단 기능, 그리고 `Spear Dark` 테마가 포함되어 있습니다.

## Workspace / 작업공간 구조

- `src/`: compiler and launcher sources
- `runtime/`: bundled runtime tools, native helpers, and optional bridge scripts
  - Core std features now run native-first.
  - Python and Node are only needed when you explicitly use `pycall(...)` or `nodecall(...)`.
  - Release packages ship a minimal optional bridge set for those calls, not the old std implementation layer.
- `examples/`: sample programs
- `std/`: bundled standard library
- `build/`: generated binaries and temporary output
- `dist/installers/`: packaged installer outputs
- `vscode-spear/`: VS Code extension
