# Spear

**English**  
Spear is a small compiled language implemented in C. It is designed to feel easy for beginners while still being structured enough for real tools, installers, and UI generation.

**한국어**  
Spear는 C로 만든 작은 컴파일 언어입니다. 비전공자도 쉽게 읽고 쓸 수 있게 만들면서도, 실제 도구 제작, 설치기, UI 생성까지 다룰 수 있도록 구조를 갖추는 것을 목표로 합니다.

- Source extension / 소스 확장자: `.sp`
- Core memory concept / 핵심 메모리 개념: `sharp`
- Build model / 빌드 모델: AOT translation from Spear to C / Spear를 C로 미리 변환한 뒤 빌드

## Quick Start / 빠른 시작

**English**  
Beginner mode automatically includes `std/prelude.sp`, so very small programs can start immediately.

**한국어**  
초보자 모드는 `std/prelude.sp`를 자동 포함하므로 아주 작은 프로그램은 바로 시작할 수 있습니다.

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
- `number`, `string`, `numbers`, `strings`: clearer beginner type names
- `text(number)` or `string(number)`: turn a number into text for UI and charts
- `function`: explicit function declaration
- `run { ... }`: simplest program entry

**한국어**

- `const`: 상수 값
- `value`: 타입 추론되는 읽기 전용 값
- `variable`: 타입 추론되는 변경 가능한 값
- `number`, `string`, `numbers`, `strings`: 더 직관적인 초보자용 타입 이름
- `text(number)` 또는 `string(number)`: 숫자를 UI/차트용 문자열로 바꾸기
- `function`: 명시적인 함수 선언
- `run { ... }`: 가장 쉬운 프로그램 시작점

Example / 예시:

```sp
run {
    numbers scores = pack(10, 20, 30);
    const title = "Spear";
    variable shots = 3;
    print(count(scores));
    print(title);
}
```

Function style / 함수 스타일:

```sp
function number add(number left, number right) {
    return left + right;
}
```

## Sharp Memory / Sharp 메모리

**English**  
`sharp { ... }` creates a temporary memory region. Text created inside the block is released together when the block ends.

**한국어**  
`sharp { ... }`는 임시 메모리 영역을 만듭니다. 블록 안에서 만든 텍스트는 블록이 끝날 때 함께 정리됩니다.

```sp
sharp {
    string label = join("sharp-", "text");
    print(label);
}
```

## Standard Library / 표준 라이브러리

**English**  
Spear ships with a structured `std/` library. For most programs, `std/prelude.sp` is the main starting surface.

**한국어**  
Spear에는 구조화된 `std/` 표준 라이브러리가 포함됩니다. 대부분의 프로그램은 `std/prelude.sp`를 시작점으로 사용하면 됩니다.

Public entrypoints / 공개 진입점:

- `std/prelude.sp`: default starter surface / 기본 시작 표면
- `std/math.sp`: numeric helpers / 숫자 유틸리티
- `std/text.sp`: text helpers / 문자열 유틸리티
- `std/lists.sp`: list helpers / 리스트 유틸리티
- `std/io.sp`: console and file helpers / 콘솔 및 파일 유틸리티
- `std/paths.sp`: path helpers / 경로 유틸리티
- `std/assert.sp`: validation helpers / 검증 유틸리티
- `std/bridge.sp`: Node/Python bridge helpers / Node/Python 연동 유틸리티
- `std/web.sp`: reusable web UI helpers / 재사용 가능한 웹 UI 유틸리티

Direct module import / 직접 모듈 import:

```sp
import "../std/math.sp";
import "../std/web.sp";
```

## Web UI / 웹 UI

**English**  
Spear includes a safe HTML and UI layer with Compose-style layout blocks and modifier chaining.

**한국어**  
Spear는 안전한 HTML/UI 레이어를 포함하며, Compose 스타일의 레이아웃 블록과 modifier 체인을 지원합니다.

Main web pieces / 주요 웹 구성요소:

- Layout / 레이아웃: `page_frame`, `container`, `section_block`, `split`, `header_bar`, `footer_note`
- Content / 콘텐츠: `heading1`, `heading2`, `paragraph`, `hero`, `metric`, `quote_block`, `badge`
- Actions / 액션: `primary_action`, `secondary_action`, `button_row`, `nav_bar`, `cta_section`
- Attributes / 속성: `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, `alt_attr`
- Modifiers / modifier: `modifier`, `padding_all`, `padding_x`, `padding_y`, `background`, `foreground`, `corner_radius`, `border`, `gap_space`, `max_width`, `shadow`, `justify_center`, `justify_between`
- Compose-style blocks / Compose 스타일 블록: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, `row_box(...) { ... }`

UI example / UI 예시:

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

## Diagnostics / 진단

**English**  
`spear check` and `spearc --check` report syntax and semantic errors without generating a native output file. The compiler also emits warnings for several common issues.

**한국어**  
`spear check`와 `spearc --check`는 네이티브 출력 파일을 만들지 않고 문법 및 의미 오류를 검사합니다. 컴파일러는 자주 발생하는 문제에 대한 경고도 출력합니다.

Current checks / 현재 진단 항목:

- multi-error collection / 여러 오류 연속 수집
- unused variable and unused import / 미사용 변수 및 미사용 import
- duplicate import / 중복 import
- unreachable code / 도달 불가 코드
- constant-condition warnings / 상수 조건 경고
- VS Code live diagnostics / VS Code 실시간 진단

## Build And Run / 빌드와 실행

```bash
make
make check
make dist
```

Run commands / 실행 명령:

```bash
build/spear.exe examples/hello.sp
build/spear.exe build examples/web.sp
build/spear.exe serve examples/web.sp
build/spear.exe check examples/hello.sp
```

Command behavior / 명령 동작:

- `spear file.sp`: temporary native build only / 임시 네이티브 빌드만 수행
- `spear serve file.sp`: temporary build plus app output / 임시 빌드 후 앱 출력 가능
- `spear build file.sp`: writes persistent build artifacts / `build/`에 결과물 생성
- `spear check file.sp`: syntax and structure validation only / 문법과 구조만 검사

## Installer / 설치기

**English**  
Install with `build/spear-setup.exe` or use the packaged installer artifacts in `dist/installers/`.

**한국어**  
`build/spear-setup.exe`로 설치하거나 `dist/installers/`에 있는 패키지된 설치 파일을 사용할 수 있습니다.

Installer features / 설치기 기능:

- Windows wizard UI / Windows 마법사 UI
- English and Korean language selection / 영어와 한국어 언어 선택
- optional PATH registration / 선택적 PATH 등록
- optional examples and VS Code extension install / 예제와 VS Code 확장 선택 설치
- post-install self-check / 설치 후 자체 점검

Artifacts / 산출물:

- `dist/installers/SpearSetup.exe`
- `dist/installers/SpearSetup.msi`
- `vscode-spear/spear-language-0.1.0.vsix`

## VS Code Extension / VS Code 확장

**English**  
The `vscode-spear/` folder contains syntax highlighting, snippets, diagnostics, and the bundled `Spear Dark` theme.

**한국어**  
`vscode-spear/` 폴더에는 문법 강조, 스니펫, 진단 기능, 그리고 번들된 `Spear Dark` 테마가 포함되어 있습니다.

## Workspace / 작업공간 구조

- `src/`: compiler and launcher sources / 컴파일러 및 런처 소스
- `runtime/`: Node and Python bridge scripts / Node 및 Python 브리지 스크립트
- `examples/`: sample programs / 예제 프로그램
- `std/`: bundled standard library / 번들 표준 라이브러리
- `build/`: generated binaries and temporary output / 생성된 바이너리와 임시 출력
- `dist/installers/`: packaged installer outputs / 패키지된 설치기 출력
- `vscode-spear/`: VS Code extension / VS Code 확장
