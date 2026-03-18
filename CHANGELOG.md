# Changelog

All notable changes to Spear are documented here.  
Spear의 주요 변경 사항은 이 문서에 기록됩니다.

## v0.2.0 - 2026-03-18

### Language / 언어

- Added beginner-friendly declaration forms: `const`, `value`, and `variable`.  
  초보자 친화적인 선언 형식 `const`, `value`, `variable`을 추가했습니다.
- Added clearer type aliases: `number`, `string`, `numbers`, and `strings`.  
  더 직관적인 타입 별칭 `number`, `string`, `numbers`, `strings`를 추가했습니다.
- Added `run { ... }`, `print(...)`, and `ask(...)` for simpler entry and console code.  
  더 쉬운 시작점과 콘솔 코드를 위해 `run { ... }`, `print(...)`, `ask(...)`를 추가했습니다.
- Added multi-error checking and broader diagnostics for unused symbols, unreachable code, constant conditions, duplicate imports, and unused imports.  
  여러 오류 연속 검사와 미사용 심볼, 도달 불가 코드, 상수 조건, 중복 import, 미사용 import 진단을 추가했습니다.
- Fixed function name collisions between user modules and standard library functions by generating package/module-aware internal symbols.  
  패키지/모듈 정보를 반영한 내부 심볼 생성을 통해 사용자 모듈과 표준 라이브러리 함수 이름 충돌을 해결했습니다.

### Standard Library / 표준 라이브러리

- Added a structured `std/` library with public entrypoints for math, text, lists, io, paths, assert, bridge, and web.  
  math, text, lists, io, paths, assert, bridge, web 공개 진입점을 갖춘 구조화된 `std/` 라이브러리를 추가했습니다.
- Added `std/prelude.sp` as the default beginner import surface.  
  초보자용 기본 import 표면으로 `std/prelude.sp`를 추가했습니다.
- Expanded reusable web building blocks for layout, content, actions, and modifier-based styling.  
  레이아웃, 콘텐츠, 액션, modifier 기반 스타일링용 웹 빌딩 블록을 확장했습니다.

### Web UI / 웹 UI

- Added Compose-style web DSL blocks: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, and `row_box(...) { ... }`.  
  Compose 스타일 웹 DSL 블록 `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, `row_box(...) { ... }`를 추가했습니다.
- Added modifier helpers for spacing, sizing, borders, colors, alignment, and flex layout.  
  간격, 크기, 테두리, 색상, 정렬, flex 레이아웃용 modifier helper를 추가했습니다.
- Added HTML attribute helpers such as `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, and `alt_attr`.  
  `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, `alt_attr` 같은 HTML 속성 helper를 추가했습니다.
- Updated examples to demonstrate declarative, component-style page composition.  
  선언형 컴포넌트 스타일 페이지 구성을 보여주도록 예제를 갱신했습니다.

### Tooling / 도구

- Improved VS Code syntax highlighting and added the bundled `Spear Dark` theme.  
  VS Code 문법 강조를 개선하고 번들 `Spear Dark` 테마를 추가했습니다.
- Improved live diagnostics in the VS Code extension and compiler path resolution for installed builds.  
  VS Code 확장의 실시간 진단과 설치형 빌드용 컴파일러 경로 해석을 개선했습니다.
- Added installer language selection for English and Korean, with localized runtime and launcher error messages.  
  영어/한국어 설치 언어 선택과 현지화된 런타임 및 런처 오류 메시지를 추가했습니다.
- Fixed Windows build and launcher issues in `Makefile` and `spear.cmd`.  
  `Makefile`과 `spear.cmd`의 Windows 빌드 및 런처 문제를 수정했습니다.

### Release Artifacts / 릴리즈 산출물

- Published Windows installer artifacts: `SpearSetup.exe` and `SpearSetup.msi`.  
  Windows 설치 파일 `SpearSetup.exe`, `SpearSetup.msi`를 배포했습니다.
- Published VS Code extension package: `spear-language-0.2.0.vsix`.  
  VS Code 확장 패키지 `spear-language-0.2.0.vsix`를 배포했습니다.
