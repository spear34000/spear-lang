# Changelog

All notable changes to Sharp are documented here.  
Sharp??二쇱슂 蹂寃??ы빆? ??臾몄꽌??湲곕줉?⑸땲??

## v0.1.0 - 2026-03-18

### Language / ?몄뼱

- Added beginner-friendly declaration forms: `const`, `value`, and `variable`.  
  珥덈낫??移쒗솕?곸씤 ?좎뼵 ?뺤떇 `const`, `value`, `variable`??異붽??덉뒿?덈떎.
- Added clearer type aliases: `number`, `string`, `numbers`, and `strings`.  
  ??吏곴??곸씤 ???蹂꾩묶 `number`, `string`, `numbers`, `strings`瑜?異붽??덉뒿?덈떎.
- Added `run { ... }`, `print(...)`, and `ask(...)` for simpler entry and console code.  
  ???ъ슫 ?쒖옉?먭낵 肄섏넄 肄붾뱶瑜??꾪빐 `run { ... }`, `print(...)`, `ask(...)`瑜?異붽??덉뒿?덈떎.
- Added multi-error checking and broader diagnostics for unused symbols, unreachable code, constant conditions, duplicate imports, and unused imports.  
  ?щ윭 ?ㅻ쪟 ?곗냽 寃?ъ? 誘몄궗???щ낵, ?꾨떖 遺덇? 肄붾뱶, ?곸닔 議곌굔, 以묐났 import, 誘몄궗??import 吏꾨떒??異붽??덉뒿?덈떎.
- Fixed function name collisions between user modules and standard library functions by generating package/module-aware internal symbols.  
  ?⑦궎吏/紐⑤뱢 ?뺣낫瑜?諛섏쁺???대? ?щ낵 ?앹꽦???듯빐 ?ъ슜??紐⑤뱢怨??쒖? ?쇱씠釉뚮윭由??⑥닔 ?대쫫 異⑸룎???닿껐?덉뒿?덈떎.

### Standard Library / ?쒖? ?쇱씠釉뚮윭由?
- Added a structured `std/` library with public entrypoints for math, text, lists, io, paths, assert, bridge, and web.  
  math, text, lists, io, paths, assert, bridge, web 怨듦컻 吏꾩엯?먯쓣 媛뽰텣 援ъ“?붾맂 `std/` ?쇱씠釉뚮윭由щ? 異붽??덉뒿?덈떎.
- Added `std/prelude.sharp` as the default beginner import surface.  
  珥덈낫?먯슜 湲곕낯 import ?쒕㈃?쇰줈 `std/prelude.sharp`瑜?異붽??덉뒿?덈떎.
- Expanded reusable web building blocks for layout, content, actions, and modifier-based styling.  
  ?덉씠?꾩썐, 肄섑뀗痢? ?≪뀡, modifier 湲곕컲 ?ㅽ??쇰쭅????鍮뚮뵫 釉붾줉???뺤옣?덉뒿?덈떎.

### Web UI / ??UI

- Added Compose-style web DSL blocks: `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, and `row_box(...) { ... }`.  
  Compose ?ㅽ?????DSL 釉붾줉 `box(...) { ... }`, `surface_box(...) { ... }`, `column_box(...) { ... }`, `row_box(...) { ... }`瑜?異붽??덉뒿?덈떎.
- Added modifier helpers for spacing, sizing, borders, colors, alignment, and flex layout.  
  媛꾧꺽, ?ш린, ?뚮몢由? ?됱긽, ?뺣젹, flex ?덉씠?꾩썐??modifier helper瑜?異붽??덉뒿?덈떎.
- Added HTML attribute helpers such as `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, and `alt_attr`.  
  `attr`, `attrs`, `class_name`, `style_attr`, `href_attr`, `src_attr`, `alt_attr` 媛숈? HTML ?띿꽦 helper瑜?異붽??덉뒿?덈떎.
- Updated examples to demonstrate declarative, component-style page composition.  
  ?좎뼵??而댄룷?뚰듃 ?ㅽ????섏씠吏 援ъ꽦??蹂댁뿬二쇰룄濡??덉젣瑜?媛깆떊?덉뒿?덈떎.

### Tooling / ?꾧뎄

- Improved VS Code syntax highlighting and added the bundled `Sharp Dark` theme.  
  VS Code 臾몃쾿 媛뺤“瑜?媛쒖꽑?섍퀬 踰덈뱾 `Sharp Dark` ?뚮쭏瑜?異붽??덉뒿?덈떎.
- Improved live diagnostics in the VS Code extension and compiler path resolution for installed builds.  
  VS Code ?뺤옣???ㅼ떆媛?吏꾨떒怨??ㅼ튂??鍮뚮뱶??而댄뙆?쇰윭 寃쎈줈 ?댁꽍??媛쒖꽑?덉뒿?덈떎.
- Added installer language selection for English and Korean, with localized runtime and launcher error messages.  
  ?곸뼱/?쒓뎅???ㅼ튂 ?몄뼱 ?좏깮怨??꾩??붾맂 ?고???諛??곗쿂 ?ㅻ쪟 硫붿떆吏瑜?異붽??덉뒿?덈떎.
- Fixed Windows build and launcher issues in `Makefile` and `Sharp.cmd`.  
  `Makefile`怨?`Sharp.cmd`??Windows 鍮뚮뱶 諛??곗쿂 臾몄젣瑜??섏젙?덉뒿?덈떎.

### Release Artifacts / 由대━利??곗텧臾?
- Published Windows installer artifacts: `SharpSetup.exe` and `SharpSetup.msi`.  
  Windows ?ㅼ튂 ?뚯씪 `SharpSetup.exe`, `SharpSetup.msi`瑜?諛고룷?덉뒿?덈떎.
- Published VS Code extension package: `Sharp-language-0.1.0.vsix`.  
  VS Code ?뺤옣 ?⑦궎吏 `Sharp-language-0.1.0.vsix`瑜?諛고룷?덉뒿?덈떎.


