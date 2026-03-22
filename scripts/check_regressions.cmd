@echo off
setlocal

if not exist build mkdir build

gcc -O2 -Wall -Wextra -std=c11 -o build\sharpc-c.exe src\spearc.c || exit /b 1
call scripts\build_rust.cmd || exit /b 1

powershell -NoProfile -Command "$lines = @('spear launch() {','    for (var i = 0; i < 3; i = i + 1) {','        sharp {','            if (i == 0) {','                continue;','            }','            if (i == 1) {','                break;','            }','        }','    }','}'); [System.IO.File]::WriteAllText('build\\regression_loop.sp', [string]::Join([Environment]::NewLine, $lines))" || exit /b 1

build\spearc.exe build\regression_loop.sp -o build\regression_loop.c >NUL 2>&1 || exit /b 1
findstr /C:"spear_scope_leave(&_scope_1);" build\regression_loop.c >NUL || exit /b 1
findstr /C:"continue;" build\regression_loop.c >NUL || exit /b 1
findstr /C:"break;" build\regression_loop.c >NUL || exit /b 1

powershell -NoProfile -Command "$helper = [string]::Join([Environment]::NewLine, @('package regression;','module helper;','','function num unique_regression_value() {','    return 7;','}')); [System.IO.File]::WriteAllText('build\\regression_helper.sp', $helper); $src = [string]::Join([Environment]::NewLine, @('import \"regression_helper.sp\";','','spear launch() {','    let bad = ;','}')); [System.IO.File]::WriteAllText('build\\regression_import_line_src.sp', $src)" || exit /b 1
cmd /c "type build\\regression_import_line_src.sp | build\\spearc.exe --check-stdin build\\regression_import_line.sp > build\\regression_import_line.log 2>&1"
findstr /C:"spearc error [line 4:" build\regression_import_line.log >NUL || exit /b 1

powershell -NoProfile -Command "$lines = @('view regression_card(text title) {','    return markup(\"section\") {','        markup(\"h1\") {','            escape(title);','        };','        row {','            action(\"/start\", \"Open\");','            action(\"#docs\", \"Docs\");','        };','    };','}','', 'spear launch() {','    text html = page(\"Demo\") {','        column {','            regression_card(\"Declarative UI\");','        };','    };','    write(\"build/ui.html\", html);','}'); [System.IO.File]::WriteAllText('build\\regression_ui.sp', [string]::Join([Environment]::NewLine, $lines))" || exit /b 1
build\spearc.exe --check build\regression_ui.sp >NUL 2>&1 || exit /b 1

cmd /c "if exist build\\regression_app rmdir /s /q build\\regression_app"
build\spear.exe new build\regression_app >NUL 2>&1 || exit /b 1
build\spear.exe check build\regression_app >NUL 2>&1 || exit /b 1
build\spear.exe build build\regression_app >NUL 2>&1 || exit /b 1

del /Q build\regression_loop.sp build\regression_loop.c build\regression_import_line.log build\regression_import_line_src.sp build\regression_helper.sp build\regression_ui.sp 2>NUL
cmd /c "if exist build\\regression_app rmdir /s /q build\\regression_app"
exit /b 0
