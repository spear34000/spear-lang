# Sharp

**English**  
Sharp is a small compiled language with Rust frontends and a native backend. It is designed to stay simple for beginners while still supporting real tooling, UI generation, installers, interop, and native builds.

**한국어**  
Sharp는 Rust 프론트엔드와 네이티브 백엔드를 사용하는 작은 컴파일 언어입니다. 비전공자도 시작하기 쉽게 유지하면서, 실제 도구 체인, UI 생성, 설치기, 외부 생태계 연동, 네이티브 빌드를 지원하는 것을 목표로 합니다.

- Source extension: `.sp`
- Core memory concept: `sharp`
- Build model: Sharp to native executable through generated C/native backend
- Interop guide: [`docs/INTEROP.md`](docs/INTEROP.md)

## Quick Start

**English**  
Small programs can start immediately because beginner mode automatically includes `std/prelude.sp`.

**한국어**  
초보자 모드에서는 `std/prelude.sp`가 자동 포함되므로 아주 작은 프로그램은 바로 시작할 수 있습니다.

```sp
run {
    print("hello");
}
```

## Core Syntax

**English**

- `const`: constant value
- `value`: inferred read-only value
- `variable`: inferred mutable value
- `fn`, `val`, `mut`, `str`, `nums`, `texts`, `show`, `app`: short aliases
- `number`, `string`, `numbers`, `strings`: beginner-friendly type names
- `map`: native keyed text state
- `result`: native success/error flow value
- `text(number)` or `string(number)`: convert numbers to text
- `function`: explicit function declaration
- `run { ... }`: simplest program entry

**한국어**

- `const`: 상수
- `value`: 타입 추론되는 읽기 전용 값
- `variable`: 타입 추론되는 변경 가능한 값
- `fn`, `val`, `mut`, `str`, `nums`, `texts`, `show`, `app`: 짧은 별칭
- `number`, `string`, `numbers`, `strings`: 직관적인 타입 이름
- `map`: 기본 키-값 상태 타입
- `result`: 성공/실패 흐름을 위한 기본 결과 타입
- `text(number)` / `string(number)`: 숫자를 문자열로 변환
- `function`: 명시적 함수 선언
- `run { ... }`: 가장 단순한 진입점

```sp
run {
    numbers scores = pack(10, 20, 30);
    const title = "Sharp";
    variable shots = 3;
    print(count(scores));
    print(title);
}
```

## Sharp Memory

**English**  
`sharp { ... }` creates a temporary memory region. `sharp text { ... }`, `sharp map { ... }`, and `sharp numbers { ... }` also work as value expressions. Use `keep ...;` to move the final value out, `defer ...;` for cleanup, and `emit ...;` to build text incrementally.

**한국어**  
`sharp { ... }`는 임시 메모리 영역을 만듭니다. `sharp text { ... }`, `sharp map { ... }`, `sharp numbers { ... }`는 값 표현식으로도 쓸 수 있습니다. 최종 값은 `keep ...;`로 꺼내고, 정리 작업은 `defer ...;`, 텍스트 누적은 `emit ...;`로 처리합니다.

```sp
value label = sharp text {
    value prefix = "sharp-";
    defer print("leaving sharp");
    emit prefix;
    emit "text";
};
```

## Commands

```bash
sharp new my-app
sharp check examples/hello.sp
sharp build examples/web.sp
sharp serve examples/web.sp
sharp add pip requests
sharp add npm dayjs
```

## Project Workflow

```bash
sharp new my-app
cd my-app
sharp check
sharp build
sharp serve
```

A project uses `sharp.toml` and usually starts from `main.sp`.

## Standard Library

Public entrypoints:

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
- `std/interop.sp`

Direct import:

```sp
import "../std/math.sp";
import "../std/web.sp";
```

Installed projects can also use:

```sp
import "std/web.sp";
```

## UI and App Output

**English**  
Sharp includes a shared UI model that can render to web HTML, Android Compose, and desktop Compose output.

**한국어**  
Sharp는 공통 UI 모델을 제공하며, 같은 선언에서 웹 HTML, Android Compose, Desktop Compose 출력을 만들 수 있습니다.

Key UI ideas:

- semantic screen model
- shared layout/content/input/state nodes
- structured action metadata
- web, android, desktop output from one screen spec

Example files:

- [`examples/ui.sp`](examples/ui.sp)
- [`examples/mobile.sp`](examples/mobile.sp)
- [`examples/web.sp`](examples/web.sp)
- [`examples/charts.sp`](examples/charts.sp)

## Interop

**English**  
Sharp can vendor Python and Node packages into a project and generate wrapper modules for them.

**한국어**  
Sharp는 Python, Node 패키지를 프로젝트 안에 vendor 형태로 설치하고, 바로 import 가능한 wrapper 모듈을 생성할 수 있습니다.

```bash
sharp add pip requests
sharp add npm dayjs
sharp add npm axios
```

For details, see [`docs/INTEROP.md`](docs/INTEROP.md).

Common generated helpers:

- `requests.get_text(url)`
- `requests.get_json(url)`
- `requests.post_json(url, body_json)`
- `requests.status_code(url)`
- `dayjs.format_now(pattern)`
- `dayjs.add_days(iso, days, pattern)`
- `dayjs.from_iso(iso, pattern)`
- `axios.get_text(url)`
- `axios.get_json(url)`
- `axios.post_json(url, body_json)`
- `axios.status_code(url)`

## Diagnostics

`sharp check` reports syntax and semantic errors without producing a native output file. The toolchain also reports warnings such as:

- unused variable
- unused import
- duplicate import
- unreachable code
- constant condition
- live VS Code diagnostics

## Native-First Runtime

**English**  
Sharp is packaged as a native-first toolchain. Most core runtime paths now work without requiring a separate Python installation.

**한국어**  
Sharp는 native-first 툴체인으로 패키징되어 있으며, 주요 런타임 경로는 별도 Python 설치 없이 동작하도록 정리되어 있습니다.

Optional bridges still exist for explicit `pycall(...)` and `nodecall(...)` workflows.

## Examples

- `examples/hello.sp`: smallest program
- `examples/web.sp`: web example
- `examples/charts.sp`: chart-first example
- `examples/ui.sp`: shared UI model example
- `examples/mobile.sp`: Android Compose output
- `examples/platform.sp`: platform standard library sample
- `examples/security.sp`: security-first sample
- `examples/server.sp`: minimal HTTP server
- `examples/interop.sp`: Python/Node interop sample

## Build From Source

```bash
make
make check
make dist
```

Rust workspace metadata is defined in [`Cargo.toml`](Cargo.toml), while the legacy native backend still lives in the repository for compatibility and migration.
