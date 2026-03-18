#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <windows.h>
#include <commctrl.h>

#define APP_CLASS "SpearSetupWizardWindow"
#define APP_CLASS_W L"SpearSetupWizardWindow"
#define WM_APP_INSTALL_DONE (WM_APP + 1)
#define EMBEDDED_TRAILER_MAGIC "SPREMB01"
#define EMBEDDED_TRAILER_SIZE 24

#define IDC_TITLE 1001
#define IDC_BODY 1002
#define IDC_CHECK_PATH 1003
#define IDC_CHECK_EXAMPLES 1004
#define IDC_CHECK_EDITOR 1005
#define IDC_CHECK_TESTS 1006
#define IDC_SUMMARY 1007
#define IDC_PROGRESS 1008
#define IDC_STATUS 1009
#define IDC_BACK 1010
#define IDC_NEXT 1011
#define IDC_CANCEL 1012
#define IDC_LANG_LABEL 1013
#define IDC_LANG_COMBO 1014
#define IDC_MODE_LABEL 1015
#define IDC_MODE_INSTALL 1016
#define IDC_MODE_REPAIR 1017
#define IDC_MODE_REMOVE 1018

#define UI_BG RGB(7, 12, 22)
#define UI_PANEL RGB(17, 24, 38)
#define UI_PANEL_ALT RGB(24, 34, 52)
#define UI_PANEL_EDGE RGB(48, 63, 92)
#define UI_ACCENT RGB(77, 138, 255)
#define UI_ACCENT_SOFT RGB(35, 57, 98)
#define UI_ACCENT_GLOW RGB(119, 169, 255)
#define UI_SIDEBAR RGB(5, 9, 18)
#define UI_TEXT RGB(238, 244, 255)
#define UI_MUTED RGB(146, 162, 190)

enum {
    LANG_EN = 0,
    LANG_KO = 1
};

enum {
    OP_INSTALL = 0,
    OP_REPAIR = 1,
    OP_REMOVE = 2
};

#define LANG_JA LANG_EN
#define LANG_ZH LANG_EN

typedef struct {
    char exe_path[4096];
    char source_root[4096];
    char install_root[4096];
    char bin_dir[4096];
    char runtime_dir[4096];
    char std_dir[4096];
    char examples_dir[4096];
    char editor_dir[4096];
    char toolchain_dir[4096];
    char src_spear[4096];
    char src_spearc[4096];
    char src_setup[4096];
    char src_runtime[4096];
    char src_std[4096];
    char src_examples[4096];
    char src_editor[4096];
    char src_toolchain[4096];
    char temp_payload_root[4096];
    bool has_std;
    bool has_examples;
    bool has_editor;
    bool has_toolchain;
    bool has_gcc;
    bool has_code;
    bool existing_install;
    bool repair_mode;
    bool uninstall_mode;
} SetupContext;

typedef struct {
    bool install_path;
    bool install_examples;
    bool install_editor;
    bool run_checks;
    int language_index;
    int operation;
} SetupOptions;

typedef struct {
    SetupContext ctx;
    SetupOptions options;
    HWND hwnd;
    HWND title;
    HWND body;
    HWND check_path;
    HWND check_examples;
    HWND check_editor;
    HWND check_tests;
    HWND lang_label;
    HWND lang_combo;
    HWND mode_label;
    HWND mode_install;
    HWND mode_repair;
    HWND mode_remove;
    HWND summary;
    HWND progress;
    HWND status;
    HWND back;
    HWND next;
    HWND cancel;
    HANDLE worker;
    int page;
    bool running;
    bool ok;
    char result[8192];
} WizardState;

static HBRUSH g_brush_bg = NULL;
static HBRUSH g_brush_sidebar = NULL;
static HBRUSH g_brush_panel = NULL;
static HBRUSH g_brush_panel_alt = NULL;
static HBRUSH g_brush_accent = NULL;
static HFONT g_font_title = NULL;
static HFONT g_font_body = NULL;
static HFONT g_font_ui = NULL;
static HFONT g_font_brand = NULL;

static void fail(const char *fmt, ...) {
    char message[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    fprintf(stderr, "spear setup error: %s\n", message);
    exit(1);
}

static wchar_t *wide_from_utf8(const char *text) {
    int needed;
    wchar_t *buffer;
    if (!text) text = "";
    needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (needed <= 0) fail("failed to convert UI text");
    buffer = (wchar_t *) malloc((size_t) needed * sizeof(wchar_t));
    if (!buffer) fail("out of memory");
    if (!MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, needed)) {
        free(buffer);
        fail("failed to convert UI text");
    }
    return buffer;
}

static void set_text_utf8(HWND hwnd, const char *text) {
    wchar_t *wide = wide_from_utf8(text);
    SetWindowTextW(hwnd, wide);
    free(wide);
}

static void combo_add_utf8(HWND hwnd, const char *text) {
    wchar_t *wide = wide_from_utf8(text);
    SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM) wide);
    free(wide);
}

static void apply_control_font(HWND hwnd, HFONT font) {
    if (hwnd && font) SendMessageW(hwnd, WM_SETFONT, (WPARAM) font, TRUE);
}

static void ensure_theme_resources(void) {
    if (!g_brush_bg) g_brush_bg = CreateSolidBrush(UI_BG);
    if (!g_brush_sidebar) g_brush_sidebar = CreateSolidBrush(UI_SIDEBAR);
    if (!g_brush_panel) g_brush_panel = CreateSolidBrush(UI_PANEL);
    if (!g_brush_panel_alt) g_brush_panel_alt = CreateSolidBrush(UI_PANEL_ALT);
    if (!g_brush_accent) g_brush_accent = CreateSolidBrush(UI_ACCENT);
    if (!g_font_title) {
        g_font_title = CreateFontW(-30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    }
    if (!g_font_body) {
        g_font_body = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    }
    if (!g_font_ui) {
        g_font_ui = CreateFontW(-15, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    }
    if (!g_font_brand) {
        g_font_brand = CreateFontW(-26, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    }
}

static void draw_text_utf8(HDC hdc, int x, int y, int w, int h, UINT format, COLORREF color, HFONT font, const char *text) {
    RECT rect;
    wchar_t *wide = wide_from_utf8(text);
    HFONT old_font = font ? (HFONT) SelectObject(hdc, font) : NULL;
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    rect.left = x;
    rect.top = y;
    rect.right = x + w;
    rect.bottom = y + h;
    DrawTextW(hdc, wide, -1, &rect, format);
    if (old_font) SelectObject(hdc, old_font);
    free(wide);
}

static void draw_button_face(DRAWITEMSTRUCT *dis, bool primary) {
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;
    char label[128];
    COLORREF fill = primary ? UI_ACCENT : UI_PANEL_ALT;
    COLORREF border = primary ? RGB(84, 148, 255) : UI_PANEL_EDGE;
    COLORREF text = primary ? RGB(250, 252, 255) : UI_TEXT;
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HPEN old_pen = (HPEN) SelectObject(hdc, pen);
    HBRUSH old_brush = (HBRUSH) SelectObject(hdc, brush);
    int radius = 10;

    if ((dis->itemState & ODS_SELECTED) != 0) OffsetRect(&rc, 0, 1);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);

    if ((dis->itemState & ODS_DISABLED) != 0) text = UI_MUTED;
    GetWindowTextA(dis->hwndItem, label, (int) sizeof(label));
    draw_text_utf8(hdc, rc.left, rc.top + 1, rc.right - rc.left, rc.bottom - rc.top,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE, text, g_font_ui, label);
}

static void draw_round_panel(HDC hdc, const RECT *rc, COLORREF fill, COLORREF border, int radius) {
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HPEN old_pen = (HPEN) SelectObject(hdc, pen);
    HBRUSH old_brush = (HBRUSH) SelectObject(hdc, brush);
    RoundRect(hdc, rc->left, rc->top, rc->right, rc->bottom, radius, radius);
    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
}

static void draw_step_item(HDC hdc, int y, int step, bool active, const char *label) {
    RECT chip;
    char number[8];
    chip.left = 22;
    chip.top = y;
    chip.right = 162;
    chip.bottom = y + 34;
    if (active) draw_round_panel(hdc, &chip, UI_PANEL_ALT, UI_PANEL_EDGE, 12);
    wsprintfA(number, "%d", step);
    draw_text_utf8(hdc, 36, y + 8, 18, 18, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        active ? UI_ACCENT_GLOW : UI_MUTED, g_font_ui, number);
    draw_text_utf8(hdc, 62, y + 7, 88, 20, DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        active ? UI_TEXT : UI_MUTED, g_font_ui, label);
}

static const char *lang_code(int index) {
    switch (index) {
        case LANG_KO: return "ko";
        default: return "en";
    }
}

static const char *tr(int lang, const char *key) {
    if (strcmp(key, "wizard_setup") == 0) {
        if (lang == LANG_KO) return "Spear 설치";
        if (lang == LANG_JA) return "Spear セットアップ";
        if (lang == LANG_ZH) return "Spear 安装";
        return "Spear Setup";
    }
    if (strcmp(key, "wizard_repair") == 0) {
        if (lang == LANG_KO) return "Spear 복구";
        if (lang == LANG_JA) return "Spear 修復";
        if (lang == LANG_ZH) return "Spear 修复";
        return "Spear Repair";
    }
    if (strcmp(key, "welcome_title") == 0) {
        if (lang == LANG_KO) return "Spear에 오신 것을 환영합니다";
        if (lang == LANG_JA) return "Spear へようこそ";
        if (lang == LANG_ZH) return "欢迎使用 Spear";
        return "Welcome to Spear";
    }
    if (strcmp(key, "welcome_body") == 0) {
        if (lang == LANG_KO) return "설치 중에 사용할 언어를 먼저 고른 다음, 필요한 구성 요소를 선택하세요.";
        if (lang == LANG_JA) return "最初に使用する言語を選び、その後で必要な構成要素を選択してください。";
        if (lang == LANG_ZH) return "先选择安装和运行时使用的语言，然后选择需要安装的组件。";
        return "Choose the language for installation and runtime messages, then select the components you want.";
    }
    if (strcmp(key, "language_label") == 0) {
        if (lang == LANG_KO) return "사용 언어";
        if (lang == LANG_JA) return "使用言語";
        if (lang == LANG_ZH) return "使用语言";
        return "Language";
    }
    if (strcmp(key, "components_title") == 0) {
        if (lang == LANG_KO) return "구성 요소 선택";
        if (lang == LANG_JA) return "コンポーネントの選択";
        if (lang == LANG_ZH) return "选择组件";
        return "Choose Components";
    }
    if (strcmp(key, "components_body") == 0) {
        if (lang == LANG_KO) return "설치할 항목을 고르세요. 나중에 설치 마법사를 다시 실행해 변경할 수 있습니다.";
        if (lang == LANG_JA) return "インストールする項目を選択してください。後でセットアップを再実行して変更できます。";
        if (lang == LANG_ZH) return "选择要安装的项目。之后可以重新运行安装向导进行修改。";
        return "Select what to install. You can run the setup wizard again later to change these choices.";
    }
    if (strcmp(key, "check_path") == 0) {
        if (lang == LANG_KO) return "Spear를 사용자 PATH에 추가";
        if (lang == LANG_JA) return "Spear をユーザー PATH に追加";
        if (lang == LANG_ZH) return "将 Spear 添加到用户 PATH";
        return "Add Spear to the user PATH";
    }
    if (strcmp(key, "check_examples") == 0) {
        if (lang == LANG_KO) return "기본 예제 작업공간 설치";
        if (lang == LANG_JA) return "同梱のサンプルワークスペースをインストール";
        if (lang == LANG_ZH) return "安装随附示例工作区";
        return "Install the bundled example workspace";
    }
    if (strcmp(key, "check_editor") == 0) {
        if (lang == LANG_KO) return "기본 VS Code 확장 설치";
        if (lang == LANG_JA) return "同梱の VS Code 拡張機能をインストール";
        if (lang == LANG_ZH) return "安装随附 VS Code 扩展";
        return "Install the bundled VS Code extension";
    }
    if (strcmp(key, "check_tests") == 0) {
        if (lang == LANG_KO) return "설치 후 자체 점검 실행";
        if (lang == LANG_JA) return "インストール後にセルフチェックを実行";
        if (lang == LANG_ZH) return "安装后运行自检";
        return "Run post-install self-checks";
    }
    if (strcmp(key, "ready_title") == 0) {
        if (lang == LANG_KO) return "설치 준비 완료";
        if (lang == LANG_JA) return "インストールの準備完了";
        if (lang == LANG_ZH) return "准备安装";
        return "Ready To Install";
    }
    if (strcmp(key, "ready_body") == 0) {
        if (lang == LANG_KO) return "선택한 옵션을 확인한 뒤 설치를 누르세요.";
        if (lang == LANG_JA) return "選択した内容を確認してからインストールを押してください。";
        if (lang == LANG_ZH) return "确认所选选项后，点击安装。";
        return "Review the selected options and click Install.";
    }
    if (strcmp(key, "installing_title") == 0) {
        if (lang == LANG_KO) return "설치 중";
        if (lang == LANG_JA) return "インストール中";
        if (lang == LANG_ZH) return "正在安装";
        return "Installing";
    }
    if (strcmp(key, "installing_body") == 0) {
        if (lang == LANG_KO) return "Spear를 설치하고 있습니다. 완료되면 화면이 자동으로 갱신됩니다.";
        if (lang == LANG_JA) return "Spear をインストールしています。完了すると画面が自動で更新されます。";
        if (lang == LANG_ZH) return "正在安装 Spear。完成后此窗口会自动更新。";
        return "Spear is being installed. The window will update when setup finishes.";
    }
    if (strcmp(key, "installing_status") == 0) {
        if (lang == LANG_KO) return "파일을 복사하고 설치를 구성하는 중입니다...";
        if (lang == LANG_JA) return "ファイルをコピーしてインストールを構成しています...";
        if (lang == LANG_ZH) return "正在复制文件并配置安装...";
        return "Copying files and configuring the installation...";
    }
    if (strcmp(key, "done_title") == 0) {
        if (lang == LANG_KO) return "설치 완료";
        if (lang == LANG_JA) return "完了";
        if (lang == LANG_ZH) return "已完成";
        return "Completed";
    }
    if (strcmp(key, "failed_title") == 0) {
        if (lang == LANG_KO) return "설치 실패";
        if (lang == LANG_JA) return "インストール失敗";
        if (lang == LANG_ZH) return "安装失败";
        return "Installation Failed";
    }
    if (strcmp(key, "done_body") == 0) {
        if (lang == LANG_KO) return "Spear를 사용할 준비가 되었습니다.";
        if (lang == LANG_JA) return "Spear を使用する準備ができました。";
        if (lang == LANG_ZH) return "Spear 已可以使用。";
        return "Spear is ready to use.";
    }
    if (strcmp(key, "failed_body") == 0) {
        if (lang == LANG_KO) return "설치를 완료하지 못했습니다.";
        if (lang == LANG_JA) return "インストールを完了できませんでした。";
        if (lang == LANG_ZH) return "安装未能完成。";
        return "The installer could not finish.";
    }
    if (strcmp(key, "back") == 0) {
        if (lang == LANG_KO) return "< 이전";
        if (lang == LANG_JA) return "< 戻る";
        if (lang == LANG_ZH) return "< 上一步";
        return "< Back";
    }
    if (strcmp(key, "next") == 0) {
        if (lang == LANG_KO) return "다음 >";
        if (lang == LANG_JA) return "次へ >";
        if (lang == LANG_ZH) return "下一步 >";
        return "Next >";
    }
    if (strcmp(key, "install") == 0) {
        if (lang == LANG_KO) return "설치";
        if (lang == LANG_JA) return "インストール";
        if (lang == LANG_ZH) return "安装";
        return "Install";
    }
    if (strcmp(key, "close") == 0) {
        if (lang == LANG_KO) return "닫기";
        if (lang == LANG_JA) return "閉じる";
        if (lang == LANG_ZH) return "关闭";
        return "Close";
    }
    if (strcmp(key, "cancel") == 0) {
        if (lang == LANG_KO) return "취소";
        if (lang == LANG_JA) return "キャンセル";
        if (lang == LANG_ZH) return "取消";
        return "Cancel";
    }
    if (strcmp(key, "yes") == 0) {
        if (lang == LANG_KO) return "예";
        if (lang == LANG_JA) return "はい";
        if (lang == LANG_ZH) return "是";
        return "yes";
    }
    if (strcmp(key, "no") == 0) {
        if (lang == LANG_KO) return "아니오";
        if (lang == LANG_JA) return "いいえ";
        if (lang == LANG_ZH) return "否";
        return "no";
    }
    if (strcmp(key, "found") == 0) {
        if (lang == LANG_KO) return "발견됨";
        if (lang == LANG_JA) return "検出済み";
        if (lang == LANG_ZH) return "已找到";
        return "found";
    }
    if (strcmp(key, "not_found") == 0) {
        if (lang == LANG_KO) return "없음";
        if (lang == LANG_JA) return "未検出";
        if (lang == LANG_ZH) return "未找到";
        return "not found";
    }
    return key;
}

static void fmt(char *out, size_t cap, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int written = vsnprintf(out, cap, format, args);
    va_end(args);
    if (written < 0 || (size_t) written >= cap) fail("failed to format text");
}

static void join_path(char *out, size_t cap, const char *a, const char *b) {
    fmt(out, cap, "%s\\%s", a, b);
}

static bool file_exists(const char *path) {
    DWORD attrs = GetFileAttributesA(path);
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static bool dir_exists(const char *path) {
    DWORD attrs = GetFileAttributesA(path);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static void parent_dir(char *path) {
    char *slash = strrchr(path, '\\');
    if (!slash) slash = strrchr(path, '/');
    if (!slash) fail("cannot resolve parent directory");
    *slash = '\0';
}

static void ensure_dir_recursive(const char *path) {
    char temp[4096];
    size_t len = strlen(path);
    if (len >= sizeof(temp)) fail("path too long");
    memcpy(temp, path, len + 1);
    for (size_t i = 3; temp[i]; i++) {
        if (temp[i] == '\\' || temp[i] == '/') {
            char saved = temp[i];
            temp[i] = '\0';
            if (temp[0]) CreateDirectoryA(temp, NULL);
            temp[i] = saved;
        }
    }
    CreateDirectoryA(temp, NULL);
}

static bool resolve_source_file(const char *root, const char *leaf, char *out, size_t cap) {
    char candidate[4096];
    join_path(candidate, sizeof(candidate), root, leaf);
    if (file_exists(candidate)) {
        fmt(out, cap, "%s", candidate);
        return true;
    }
    fmt(candidate, sizeof(candidate), "%s\\..\\%s", root, leaf);
    if (file_exists(candidate)) {
        fmt(out, cap, "%s", candidate);
        return true;
    }
    return false;
}

static bool resolve_source_dir(const char *root, const char *leaf, char *out, size_t cap) {
    char candidate[4096];
    join_path(candidate, sizeof(candidate), root, leaf);
    if (dir_exists(candidate)) {
        fmt(out, cap, "%s", candidate);
        return true;
    }
    fmt(candidate, sizeof(candidate), "%s\\..\\%s", root, leaf);
    if (dir_exists(candidate)) {
        fmt(out, cap, "%s", candidate);
        return true;
    }
    return false;
}

static bool resolve_tool(const char *name) {
    char buffer[4096];
    DWORD result = SearchPathA(NULL, name, NULL, (DWORD) sizeof(buffer), buffer, NULL);
    return result > 0 && result < sizeof(buffer);
}

static void broadcast_env_change(void) {
    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

static void update_user_path(const char *bin_dir, bool add_entry) {
    HKEY key;
    char current[8192] = {0};
    char next[8192] = {0};
    DWORD size = sizeof(current);
    DWORD type = REG_SZ;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &key) != ERROR_SUCCESS) {
        fail("failed to open user environment");
    }
    if (RegQueryValueExA(key, "Path", NULL, &type, (LPBYTE) current, &size) != ERROR_SUCCESS) {
        current[0] = '\0';
    }
    {
        const char *cursor = current;
        int first = 1;
        bool present = false;
        while (*cursor) {
            const char *sep = strchr(cursor, ';');
            size_t len = sep ? (size_t) (sep - cursor) : strlen(cursor);
            if (len == strlen(bin_dir) && _strnicmp(cursor, bin_dir, len) == 0) {
                present = true;
            } else if (len > 0) {
                if (!first) strncat(next, ";", sizeof(next) - strlen(next) - 1);
                strncat(next, cursor, len);
                first = 0;
            }
            if (!sep) break;
            cursor = sep + 1;
        }
        if (add_entry && !present) {
            if (next[0]) strncat(next, ";", sizeof(next) - strlen(next) - 1);
            strncat(next, bin_dir, sizeof(next) - strlen(next) - 1);
        }
    }
    if (RegSetValueExA(key, "Path", 0, REG_EXPAND_SZ, (const BYTE *) next, (DWORD) strlen(next) + 1) != ERROR_SUCCESS) {
        RegCloseKey(key);
        fail("failed to update user PATH");
    }
    RegCloseKey(key);
    broadcast_env_change();
}

static void remove_uninstall_info(void) {
    RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Spear");
}

static void set_uninstall_info(const char *install_root, const char *bin_dir) {
    HKEY key;
    char uninstall_cmd[2048];
    char icon_path[2048];
    if (RegCreateKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Spear",
            0, NULL, 0, KEY_WRITE, NULL, &key, NULL) != ERROR_SUCCESS) {
        fail("failed to register uninstall metadata");
    }
    fmt(uninstall_cmd, sizeof(uninstall_cmd), "\"%s\\spear-setup.exe\" --uninstall", bin_dir);
    fmt(icon_path, sizeof(icon_path), "%s\\spear.exe", bin_dir);
    RegSetValueExA(key, "DisplayName", 0, REG_SZ, (const BYTE *) "Spear", 6);
    RegSetValueExA(key, "Publisher", 0, REG_SZ, (const BYTE *) "Spear Project", 14);
    RegSetValueExA(key, "InstallLocation", 0, REG_SZ, (const BYTE *) install_root, (DWORD) strlen(install_root) + 1);
    RegSetValueExA(key, "DisplayVersion", 0, REG_SZ, (const BYTE *) "0.1.0", 6);
    RegSetValueExA(key, "UninstallString", 0, REG_SZ, (const BYTE *) uninstall_cmd, (DWORD) strlen(uninstall_cmd) + 1);
    RegSetValueExA(key, "DisplayIcon", 0, REG_SZ, (const BYTE *) icon_path, (DWORD) strlen(icon_path) + 1);
    RegCloseKey(key);
}

static void remove_tree_recursive(const char *path) {
    char pattern[4096];
    WIN32_FIND_DATAA entry;
    HANDLE handle;
    if (!dir_exists(path)) {
        if (file_exists(path)) DeleteFileA(path);
        return;
    }
    fmt(pattern, sizeof(pattern), "%s\\*", path);
    handle = FindFirstFileA(pattern, &entry);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            char child[4096];
            if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) continue;
            join_path(child, sizeof(child), path, entry.cFileName);
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) remove_tree_recursive(child);
            else DeleteFileA(child);
        } while (FindNextFileA(handle, &entry));
        FindClose(handle);
    }
    RemoveDirectoryA(path);
}

static void copy_file_to(const char *src, const char *dst) {
    char parent[4096];
    fmt(parent, sizeof(parent), "%s", dst);
    parent_dir(parent);
    ensure_dir_recursive(parent);
    if (!CopyFileA(src, dst, FALSE)) fail("failed to copy %s", src);
}

static bool should_skip_name(const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return true;
    if (_stricmp(name, "__pycache__") == 0) return true;
    if (_stricmp(name, ".gitkeep") == 0) return true;
    return false;
}

static bool should_skip_file(const char *name) {
    const char *dot = strrchr(name, '.');
    if (dot && _stricmp(dot, ".vsix") == 0) return true;
    if (dot && _stricmp(dot, ".pyc") == 0) return true;
    return false;
}

static int run_process_capture(const char *command_line, const char *working_dir, char *output, size_t output_cap);

static void copy_tree(const char *src_dir, const char *dst_dir) {
    char pattern[4096];
    WIN32_FIND_DATAA entry;
    HANDLE handle;
    ensure_dir_recursive(dst_dir);
    fmt(pattern, sizeof(pattern), "%s\\*", src_dir);
    handle = FindFirstFileA(pattern, &entry);
    if (handle == INVALID_HANDLE_VALUE) fail("failed to enumerate %s", src_dir);
    do {
        char src_path[4096];
        char dst_path[4096];
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (should_skip_name(entry.cFileName)) continue;
            join_path(src_path, sizeof(src_path), src_dir, entry.cFileName);
            join_path(dst_path, sizeof(dst_path), dst_dir, entry.cFileName);
            copy_tree(src_path, dst_path);
        } else {
            if (should_skip_file(entry.cFileName)) continue;
            join_path(src_path, sizeof(src_path), src_dir, entry.cFileName);
            join_path(dst_path, sizeof(dst_path), dst_dir, entry.cFileName);
            copy_file_to(src_path, dst_path);
        }
    } while (FindNextFileA(handle, &entry));
    FindClose(handle);
}

static unsigned long long read_u64_le(const unsigned char *bytes) {
    return ((unsigned long long) bytes[0]) |
           ((unsigned long long) bytes[1] << 8) |
           ((unsigned long long) bytes[2] << 16) |
           ((unsigned long long) bytes[3] << 24) |
           ((unsigned long long) bytes[4] << 32) |
           ((unsigned long long) bytes[5] << 40) |
           ((unsigned long long) bytes[6] << 48) |
           ((unsigned long long) bytes[7] << 56);
}

static void make_temp_dir(char *out, size_t cap) {
    char temp_root[1024];
    char temp_name[1024];
    DWORD len = GetTempPathA((DWORD) sizeof(temp_root), temp_root);
    if (len == 0 || len >= sizeof(temp_root)) fail("TEMP is not available");
    if (!GetTempFileNameA(temp_root, "spr", 0, temp_name)) fail("failed to allocate temp directory");
    DeleteFileA(temp_name);
    if (!CreateDirectoryA(temp_name, NULL)) fail("failed to create temp directory");
    fmt(out, cap, "%s", temp_name);
}

static bool read_exact(HANDLE file, void *buffer, DWORD bytes) {
    DWORD total = 0;
    while (total < bytes) {
        DWORD chunk = 0;
        if (!ReadFile(file, (char *) buffer + total, bytes - total, &chunk, NULL)) return false;
        if (chunk == 0) return false;
        total += chunk;
    }
    return true;
}

static bool extract_embedded_payload(SetupContext *ctx) {
    HANDLE file;
    LARGE_INTEGER size;
    LARGE_INTEGER pos;
    unsigned char trailer[EMBEDDED_TRAILER_SIZE];
    unsigned long long payload_offset;
    unsigned long long payload_size;
    char temp_root[4096];
    char zip_path[4096];
    char command[12288];
    HANDLE out;
    file = CreateFileA(ctx->exe_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return false;
    if (!GetFileSizeEx(file, &size) || size.QuadPart < EMBEDDED_TRAILER_SIZE) {
        CloseHandle(file);
        return false;
    }
    pos.QuadPart = size.QuadPart - EMBEDDED_TRAILER_SIZE;
    if (!SetFilePointerEx(file, pos, NULL, FILE_BEGIN) || !read_exact(file, trailer, sizeof(trailer))) {
        CloseHandle(file);
        return false;
    }
    if (memcmp(trailer, EMBEDDED_TRAILER_MAGIC, 8) != 0) {
        CloseHandle(file);
        return false;
    }
    payload_offset = read_u64_le(trailer + 8);
    payload_size = read_u64_le(trailer + 16);
    if (payload_offset >= (unsigned long long) size.QuadPart ||
        payload_size == 0 ||
        payload_offset + payload_size > (unsigned long long) size.QuadPart - EMBEDDED_TRAILER_SIZE) {
        CloseHandle(file);
        return false;
    }
    make_temp_dir(temp_root, sizeof(temp_root));
    fmt(zip_path, sizeof(zip_path), "%s\\payload.zip", temp_root);
    out = CreateFileA(zip_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (out == INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        remove_tree_recursive(temp_root);
        fail("failed to create bundled setup payload");
    }
    pos.QuadPart = (LONGLONG) payload_offset;
    if (!SetFilePointerEx(file, pos, NULL, FILE_BEGIN)) {
        CloseHandle(out);
        CloseHandle(file);
        remove_tree_recursive(temp_root);
        fail("failed to access bundled setup payload");
    }
    while (payload_size > 0) {
        unsigned char buffer[65536];
        DWORD chunk = payload_size > sizeof(buffer) ? (DWORD) sizeof(buffer) : (DWORD) payload_size;
        DWORD written = 0;
        if (!read_exact(file, buffer, chunk) || !WriteFile(out, buffer, chunk, &written, NULL) || written != chunk) {
            CloseHandle(out);
            CloseHandle(file);
            remove_tree_recursive(temp_root);
            fail("failed to extract bundled setup payload");
        }
        payload_size -= chunk;
    }
    CloseHandle(out);
    CloseHandle(file);
    fmt(command, sizeof(command),
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \"Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('%s','%s')\"",
        zip_path, temp_root);
    if (run_process_capture(command, temp_root, NULL, 0) != 0) {
        remove_tree_recursive(temp_root);
        fail("failed to unpack bundled setup payload");
    }
    DeleteFileA(zip_path);
    fmt(ctx->temp_payload_root, sizeof(ctx->temp_payload_root), "%s", temp_root);
    fmt(ctx->source_root, sizeof(ctx->source_root), "%s", temp_root);
    return true;
}

static int run_process_capture(const char *command_line, const char *working_dir, char *output, size_t output_cap) {
    SECURITY_ATTRIBUTES sa;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD total = 0;
    char cmd[4096];
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) return -1;
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;
    fmt(cmd, sizeof(cmd), "%s", command_line);
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, working_dir, &si, &pi)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        return -1;
    }
    CloseHandle(write_pipe);
    if (output && output_cap) output[0] = '\0';
    for (;;) {
        char chunk[512];
        DWORD read = 0;
        if (!ReadFile(read_pipe, chunk, sizeof(chunk) - 1, &read, NULL) || read == 0) break;
        if (output && total + 1 < output_cap) {
            DWORD copy = read;
            if (copy > output_cap - total - 1) copy = output_cap - total - 1;
            memcpy(output + total, chunk, copy);
            total += copy;
            output[total] = '\0';
        }
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    {
        DWORD exit_code = 1;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(read_pipe);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return (int) exit_code;
    }
}

static void run_self_check(const SetupContext *ctx, const SetupOptions *options, char *out, size_t cap) {
    char spearc[4096];
    char spear[4096];
    char bundled_gcc[4096];
    char check_file[4096];
    char temp_source[4096];
    char command[8192];
    char output[4096];
    bool can_run_launcher_check;
    join_path(spearc, sizeof(spearc), ctx->bin_dir, "spearc.exe");
    join_path(spear, sizeof(spear), ctx->bin_dir, "spear.exe");
    if (options->install_examples && dir_exists(ctx->examples_dir)) {
        join_path(check_file, sizeof(check_file), ctx->examples_dir, "hello.sp");
    } else {
        join_path(temp_source, sizeof(temp_source), ctx->install_root, "self-check.sp");
        {
            FILE *fp = fopen(temp_source, "wb");
            if (!fp) fail("failed to create self-check source");
            fputs("spear launch() {\n    say(1);\n}\n", fp);
            fclose(fp);
        }
        fmt(check_file, sizeof(check_file), "%s", temp_source);
    }
    fmt(command, sizeof(command), "\"%s\" --check \"%s\"", spearc, check_file);
    if (run_process_capture(command, ctx->install_root, output, sizeof(output)) != 0) {
        fmt(out, cap, "Installed, but compiler self-check failed.\r\n\r\n%s", output[0] ? output : "No details.");
        if (!(options->install_examples && dir_exists(ctx->examples_dir))) DeleteFileA(check_file);
        return;
    }
    join_path(bundled_gcc, sizeof(bundled_gcc), ctx->toolchain_dir, "mingw64\\bin\\gcc.exe");
    can_run_launcher_check = ctx->has_gcc || file_exists(bundled_gcc);
    if (can_run_launcher_check) {
        fmt(command, sizeof(command), "\"%s\" check \"%s\"", spear, check_file);
        if (run_process_capture(command, ctx->install_root, output, sizeof(output)) != 0) {
            fmt(out, cap, "Installed, but launcher self-check failed.\r\n\r\n%s", output[0] ? output : "No details.");
            if (!(options->install_examples && dir_exists(ctx->examples_dir))) DeleteFileA(check_file);
            return;
        }
    }
    if (!(options->install_examples && dir_exists(ctx->examples_dir))) DeleteFileA(check_file);
    fmt(out, cap, can_run_launcher_check ? "Installation finished and self-check passed." : "Installation finished. GCC was not found, so launcher validation was skipped.");
}

static void discover_context(SetupContext *ctx) {
    char local_app[4096];
    char user_profile[4096];
    DWORD len = GetModuleFileNameA(NULL, ctx->exe_path, (DWORD) sizeof(ctx->exe_path));
    if (len == 0 || len >= sizeof(ctx->exe_path)) fail("cannot resolve installer path");
    fmt(ctx->source_root, sizeof(ctx->source_root), "%s", ctx->exe_path);
    parent_dir(ctx->source_root);
    extract_embedded_payload(ctx);
    if (GetEnvironmentVariableA("LOCALAPPDATA", local_app, sizeof(local_app)) == 0) fail("LOCALAPPDATA is not available");
    if (GetEnvironmentVariableA("USERPROFILE", user_profile, sizeof(user_profile)) == 0) fail("USERPROFILE is not available");
    join_path(ctx->install_root, sizeof(ctx->install_root), local_app, "Programs\\Spear");
    join_path(ctx->bin_dir, sizeof(ctx->bin_dir), ctx->install_root, "bin");
    join_path(ctx->runtime_dir, sizeof(ctx->runtime_dir), ctx->install_root, "runtime");
    join_path(ctx->std_dir, sizeof(ctx->std_dir), ctx->install_root, "std");
    join_path(ctx->examples_dir, sizeof(ctx->examples_dir), ctx->install_root, "examples");
    join_path(ctx->toolchain_dir, sizeof(ctx->toolchain_dir), ctx->install_root, "toolchain");
    fmt(ctx->editor_dir, sizeof(ctx->editor_dir), "%s\\.vscode\\extensions\\spear-language-local", user_profile);
    {
        char existing_launcher[4096];
        join_path(existing_launcher, sizeof(existing_launcher), ctx->bin_dir, "spear.exe");
        ctx->existing_install = file_exists(existing_launcher);
    }
    ctx->repair_mode = strstr(GetCommandLineA(), "--repair") != NULL;
    ctx->uninstall_mode = strstr(GetCommandLineA(), "--uninstall") != NULL;
    if (!resolve_source_file(ctx->source_root, "spear.exe", ctx->src_spear, sizeof(ctx->src_spear)) ||
        !resolve_source_file(ctx->source_root, "spearc.exe", ctx->src_spearc, sizeof(ctx->src_spearc)) ||
        !resolve_source_file(ctx->source_root, "spear-setup.exe", ctx->src_setup, sizeof(ctx->src_setup)) ||
        !resolve_source_dir(ctx->source_root, "runtime", ctx->src_runtime, sizeof(ctx->src_runtime)) ||
        !resolve_source_dir(ctx->source_root, "std", ctx->src_std, sizeof(ctx->src_std))) {
        fail("installer must run with spear.exe, spearc.exe, spear-setup.exe, runtime, and std available");
    }
    ctx->has_std = true;
    ctx->has_examples = resolve_source_dir(ctx->source_root, "examples", ctx->src_examples, sizeof(ctx->src_examples));
    ctx->has_editor = resolve_source_dir(ctx->source_root, "vscode-spear", ctx->src_editor, sizeof(ctx->src_editor));
    ctx->has_toolchain = resolve_source_dir(ctx->source_root, "toolchain", ctx->src_toolchain, sizeof(ctx->src_toolchain));
    ctx->has_gcc = resolve_tool("gcc.exe");
    ctx->has_code = resolve_tool("code.cmd") || resolve_tool("code.exe");
}

static void perform_uninstall(const SetupContext *ctx) {
    int answer;
    char message[4096];
    fmt(message, sizeof(message),
        "Remove Spear from this user profile?\n\nInstall root:\n%s\n\nThis removes the launcher, compiler, runtime, examples, and the local VS Code extension folder.",
        ctx->install_root);
    answer = MessageBoxA(NULL, message, "Spear Uninstall", MB_ICONQUESTION | MB_OKCANCEL | MB_SETFOREGROUND);
    if (answer != IDOK) {
        printf("Spear uninstall cancelled.\n");
        return;
    }
    update_user_path(ctx->bin_dir, false);
    remove_uninstall_info();
    remove_tree_recursive(ctx->install_root);
    remove_tree_recursive(ctx->editor_dir);
    printf("Spear uninstalled from %s\n", ctx->install_root);
}

static void perform_uninstall_silent(const SetupContext *ctx) {
    update_user_path(ctx->bin_dir, false);
    remove_uninstall_info();
    remove_tree_recursive(ctx->install_root);
    remove_tree_recursive(ctx->editor_dir);
}

static void set_text(HWND hwnd, const char *text) {
    set_text_utf8(hwnd, text);
}

static void read_options(WizardState *state) {
    state->options.install_path = SendMessageA(state->check_path, BM_GETCHECK, 0, 0) == BST_CHECKED;
    state->options.install_examples = SendMessageA(state->check_examples, BM_GETCHECK, 0, 0) == BST_CHECKED;
    state->options.install_editor = SendMessageA(state->check_editor, BM_GETCHECK, 0, 0) == BST_CHECKED;
    state->options.run_checks = SendMessageA(state->check_tests, BM_GETCHECK, 0, 0) == BST_CHECKED;
    state->options.language_index = (int) SendMessageW(state->lang_combo, CB_GETCURSEL, 0, 0);
    if (SendMessageA(state->mode_remove, BM_GETCHECK, 0, 0) == BST_CHECKED) state->options.operation = OP_REMOVE;
    else if (SendMessageA(state->mode_repair, BM_GETCHECK, 0, 0) == BST_CHECKED) state->options.operation = OP_REPAIR;
    else state->options.operation = OP_INSTALL;
    if (state->options.language_index < 0) state->options.language_index = LANG_EN;
}

static void write_options(WizardState *state) {
    SendMessageA(state->check_path, BM_SETCHECK, state->options.install_path ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(state->check_examples, BM_SETCHECK, state->options.install_examples ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(state->check_editor, BM_SETCHECK, state->options.install_editor ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(state->check_tests, BM_SETCHECK, state->options.run_checks ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(state->lang_combo, CB_SETCURSEL, state->options.language_index, 0);
    SendMessageA(state->mode_install, BM_SETCHECK, state->options.operation == OP_INSTALL ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(state->mode_repair, BM_SETCHECK, state->options.operation == OP_REPAIR ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageA(state->mode_remove, BM_SETCHECK, state->options.operation == OP_REMOVE ? BST_CHECKED : BST_UNCHECKED, 0);
    EnableWindow(state->check_examples, state->ctx.has_examples ? TRUE : FALSE);
    EnableWindow(state->check_editor, state->ctx.has_editor ? TRUE : FALSE);
    EnableWindow(state->mode_repair, state->ctx.existing_install ? TRUE : FALSE);
    EnableWindow(state->mode_remove, state->ctx.existing_install ? TRUE : FALSE);
}

static void write_language_config(const SetupContext *ctx, int lang) {
    char path[4096];
    FILE *fp;
    join_path(path, sizeof(path), ctx->install_root, "spear-lang.txt");
    fp = fopen(path, "wb");
    if (!fp) fail("failed to write language configuration");
    fputs(lang_code(lang), fp);
    fclose(fp);
}

static void build_summary(WizardState *state, char *out, size_t cap) {
    read_options(state);
    {
        int lang = state->options.language_index;
    fmt(out, cap,
        "Install location:\r\n%s\r\n\r\n"
        "Language:\r\n%s\r\n\r\n"
        "Core:\r\n- spear.exe\r\n- spearc.exe\r\n- spear-setup.exe\r\n- runtime bridge files\r\n\r\n"
        "Options:\r\n- PATH registration: %s\r\n- Example workspace: %s\r\n- VS Code extension: %s\r\n- Post-install self-check: %s\r\n\r\n"
        "Detected:\r\n- system gcc: %s\r\n- bundled toolchain: %s\r\n- VS Code CLI: %s",
        state->ctx.install_root,
        lang == LANG_KO ? "한국어" : "English",
        state->options.install_path ? tr(lang, "yes") : tr(lang, "no"),
        state->options.install_examples ? tr(lang, "yes") : tr(lang, "no"),
        state->options.install_editor ? tr(lang, "yes") : tr(lang, "no"),
        state->options.run_checks ? tr(lang, "yes") : tr(lang, "no"),
        state->ctx.has_gcc ? tr(lang, "found") : tr(lang, "not_found"),
        state->ctx.has_toolchain ? tr(lang, "found") : tr(lang, "not_found"),
        state->ctx.has_code ? tr(lang, "found") : tr(lang, "not_found"));
    }
}

static void show_page(WizardState *state) {
    char text[8192];
    read_options(state);
    MoveWindow(state->title, 220, 34, 420, 38, TRUE);
    MoveWindow(state->body, 220, 76, 420, 46, TRUE);
    MoveWindow(state->lang_label, 236, 172, 120, 24, TRUE);
    MoveWindow(state->lang_combo, 380, 168, 210, 260, TRUE);
    MoveWindow(state->mode_label, 236, 214, 120, 24, TRUE);
    MoveWindow(state->mode_install, 380, 212, 210, 22, TRUE);
    MoveWindow(state->mode_repair, 380, 242, 210, 22, TRUE);
    MoveWindow(state->mode_remove, 380, 272, 210, 22, TRUE);
    MoveWindow(state->check_path, 236, 172, 354, 22, TRUE);
    MoveWindow(state->check_examples, 236, 206, 354, 22, TRUE);
    MoveWindow(state->check_editor, 236, 240, 354, 22, TRUE);
    MoveWindow(state->check_tests, 236, 274, 354, 22, TRUE);
    MoveWindow(state->summary, 220, 162, 390, 184, TRUE);
    MoveWindow(state->progress, 236, 186, 354, 18, TRUE);
    MoveWindow(state->status, 236, 214, 354, 132, TRUE);
    set_text(state->back, tr(state->options.language_index, "back"));
    set_text(state->cancel, tr(state->options.language_index, "cancel"));
    ShowWindow(state->check_path, SW_HIDE);
    ShowWindow(state->check_examples, SW_HIDE);
    ShowWindow(state->check_editor, SW_HIDE);
    ShowWindow(state->check_tests, SW_HIDE);
    ShowWindow(state->lang_label, SW_HIDE);
    ShowWindow(state->lang_combo, SW_HIDE);
    ShowWindow(state->mode_label, SW_HIDE);
    ShowWindow(state->mode_install, SW_HIDE);
    ShowWindow(state->mode_repair, SW_HIDE);
    ShowWindow(state->mode_remove, SW_HIDE);
    ShowWindow(state->summary, SW_HIDE);
    ShowWindow(state->progress, SW_HIDE);
    ShowWindow(state->status, SW_HIDE);

    if (state->page == 0) {
        set_text(state->title, state->ctx.repair_mode ? tr(state->options.language_index, "wizard_repair") : tr(state->options.language_index, "wizard_setup"));
        set_text(state->body, tr(state->options.language_index, "welcome_body"));
        set_text(state->lang_label, tr(state->options.language_index, "language_label"));
        set_text(state->mode_label, state->options.language_index == LANG_KO ? "설치 작업" : "Action");
        set_text(state->mode_install, state->options.language_index == LANG_KO ? (state->ctx.existing_install ? "Spear 다시 설치" : "Spear 설치") : (state->ctx.existing_install ? "Reinstall Spear" : "Install Spear"));
        set_text(state->mode_repair, state->options.language_index == LANG_KO ? "현재 설치 복구" : "Repair current install");
        set_text(state->mode_remove, state->options.language_index == LANG_KO ? "Spear 제거" : "Remove Spear");
        write_options(state);
        ShowWindow(state->lang_label, SW_SHOW);
        ShowWindow(state->lang_combo, SW_SHOW);
        ShowWindow(state->mode_label, SW_SHOW);
        ShowWindow(state->mode_install, SW_SHOW);
        ShowWindow(state->mode_repair, SW_SHOW);
        ShowWindow(state->mode_remove, SW_SHOW);
        EnableWindow(state->back, FALSE);
        EnableWindow(state->next, TRUE);
        set_text(state->next, tr(state->options.language_index, "next"));
    } else if (state->page == 1) {
        set_text(state->title, tr(state->options.language_index, "components_title"));
        set_text(state->body, tr(state->options.language_index, "components_body"));
        set_text(state->check_path, tr(state->options.language_index, "check_path"));
        set_text(state->check_examples, tr(state->options.language_index, "check_examples"));
        set_text(state->check_editor, tr(state->options.language_index, "check_editor"));
        set_text(state->check_tests, tr(state->options.language_index, "check_tests"));
        write_options(state);
        EnableWindow(state->check_path, state->options.operation != OP_REMOVE);
        EnableWindow(state->check_examples, state->ctx.has_examples && state->options.operation != OP_REMOVE);
        EnableWindow(state->check_editor, state->ctx.has_editor && state->options.operation != OP_REMOVE);
        EnableWindow(state->check_tests, state->options.operation != OP_REMOVE);
        ShowWindow(state->check_path, SW_SHOW);
        ShowWindow(state->check_examples, SW_SHOW);
        ShowWindow(state->check_editor, SW_SHOW);
        ShowWindow(state->check_tests, SW_SHOW);
        EnableWindow(state->back, TRUE);
        EnableWindow(state->next, TRUE);
        set_text(state->next, tr(state->options.language_index, "next"));
    } else if (state->page == 2) {
        set_text(state->title, tr(state->options.language_index, "ready_title"));
        set_text(state->body, tr(state->options.language_index, "ready_body"));
        build_summary(state, text, sizeof(text));
        set_text(state->summary, text);
        ShowWindow(state->summary, SW_SHOW);
        EnableWindow(state->back, TRUE);
        EnableWindow(state->next, TRUE);
        set_text(state->next, tr(state->options.language_index, "install"));
    } else if (state->page == 3) {
        set_text(state->title, tr(state->options.language_index, "installing_title"));
        set_text(state->body, tr(state->options.language_index, "installing_body"));
        set_text(state->status, tr(state->options.language_index, "installing_status"));
        ShowWindow(state->progress, SW_SHOW);
        ShowWindow(state->status, SW_SHOW);
        EnableWindow(state->back, FALSE);
        EnableWindow(state->next, FALSE);
    } else {
        set_text(state->title, state->ok ? tr(state->options.language_index, "done_title") : tr(state->options.language_index, "failed_title"));
        set_text(state->body, state->ok ? tr(state->options.language_index, "done_body") : tr(state->options.language_index, "failed_body"));
        set_text(state->status, state->result);
        ShowWindow(state->status, SW_SHOW);
        EnableWindow(state->back, FALSE);
        EnableWindow(state->next, TRUE);
        set_text(state->next, tr(state->options.language_index, "close"));
    }
}

static DWORD WINAPI install_worker(LPVOID param) {
    WizardState *state = (WizardState *) param;
    char dst[4096];
    char summary[4096];
    state->ok = false;
    state->result[0] = '\0';
    if (state->options.operation == OP_REMOVE) {
        perform_uninstall_silent(&state->ctx);
        fmt(state->result, sizeof(state->result), "Spear was removed from:\r\n%s", state->ctx.install_root);
        state->ok = true;
        PostMessageA(state->hwnd, WM_APP_INSTALL_DONE, 0, 0);
        return 0;
    }
    remove_tree_recursive(state->ctx.runtime_dir);
    remove_tree_recursive(state->ctx.std_dir);
    remove_tree_recursive(state->ctx.examples_dir);
    remove_tree_recursive(state->ctx.editor_dir);
    remove_tree_recursive(state->ctx.toolchain_dir);
    ensure_dir_recursive(state->ctx.bin_dir);
    ensure_dir_recursive(state->ctx.runtime_dir);
    ensure_dir_recursive(state->ctx.std_dir);
    ensure_dir_recursive(state->ctx.toolchain_dir);
    join_path(dst, sizeof(dst), state->ctx.bin_dir, "spear.exe");
    copy_file_to(state->ctx.src_spear, dst);
    join_path(dst, sizeof(dst), state->ctx.bin_dir, "spearc.exe");
    copy_file_to(state->ctx.src_spearc, dst);
    join_path(dst, sizeof(dst), state->ctx.bin_dir, "spear-setup.exe");
    copy_file_to(state->ctx.src_setup, dst);
    copy_tree(state->ctx.src_runtime, state->ctx.runtime_dir);
    copy_tree(state->ctx.src_std, state->ctx.std_dir);
    if (state->ctx.has_toolchain) copy_tree(state->ctx.src_toolchain, state->ctx.toolchain_dir);
    write_language_config(&state->ctx, state->options.language_index);
    if (state->options.install_examples && state->ctx.has_examples) copy_tree(state->ctx.src_examples, state->ctx.examples_dir);
    if (state->options.install_editor && state->ctx.has_editor) copy_tree(state->ctx.src_editor, state->ctx.editor_dir);
    if (state->options.install_path) update_user_path(state->ctx.bin_dir, true);
    set_uninstall_info(state->ctx.install_root, state->ctx.bin_dir);
    if (state->options.run_checks) run_self_check(&state->ctx, &state->options, summary, sizeof(summary));
    else fmt(summary, sizeof(summary), state->options.operation == OP_REPAIR ? "Repair finished." : "Installation finished.");
    fmt(state->result, sizeof(state->result),
        "%s\r\n\r\nInstall root:\r\n%s\r\n\r\nStandard library:\r\n%s\r\n\r\nExamples:\r\n%s\r\n\r\nVS Code extension:\r\n%s",
        summary,
        state->ctx.install_root,
        state->ctx.std_dir,
        (state->options.install_examples && dir_exists(state->ctx.examples_dir)) ? state->ctx.examples_dir : "not installed",
        (state->options.install_editor && dir_exists(state->ctx.editor_dir)) ? state->ctx.editor_dir : "not installed");
    state->ok = true;
    PostMessageA(state->hwnd, WM_APP_INSTALL_DONE, 0, 0);
    return 0;
}

static void start_install(WizardState *state) {
    read_options(state);
    state->page = 3;
    state->running = true;
    show_page(state);
    SendMessageA(state->progress, PBM_SETMARQUEE, TRUE, 0);
    state->worker = CreateThread(NULL, 0, install_worker, state, 0, NULL);
    if (!state->worker) {
        state->running = false;
        state->ok = false;
        fmt(state->result, sizeof(state->result), "Failed to start installer worker thread.");
        state->page = 4;
        show_page(state);
    }
}

static LRESULT CALLBACK wizard_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WizardState *state = (WizardState *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    switch (msg) {
        case WM_NCCREATE:
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCTW *) lParam)->lpCreateParams);
            return TRUE;
        case WM_CREATE:
            state = (WizardState *) ((CREATESTRUCTW *) lParam)->lpCreateParams;
            state->hwnd = hwnd;
            ensure_theme_resources();
            state->title = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 220, 34, 420, 38, hwnd, (HMENU) IDC_TITLE, NULL, NULL);
            state->body = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 220, 76, 420, 46, hwnd, (HMENU) IDC_BODY, NULL, NULL);
            state->lang_label = CreateWindowW(L"STATIC", L"", WS_CHILD, 32, 126, 120, 22, hwnd, (HMENU) IDC_LANG_LABEL, NULL, NULL);
            state->lang_combo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST, 160, 122, 220, 240, hwnd, (HMENU) IDC_LANG_COMBO, NULL, NULL);
            state->mode_label = CreateWindowW(L"STATIC", L"", WS_CHILD, 32, 158, 120, 22, hwnd, (HMENU) IDC_MODE_LABEL, NULL, NULL);
            state->mode_install = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTORADIOBUTTON, 160, 156, 300, 22, hwnd, (HMENU) IDC_MODE_INSTALL, NULL, NULL);
            state->mode_repair = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTORADIOBUTTON, 160, 184, 300, 22, hwnd, (HMENU) IDC_MODE_REPAIR, NULL, NULL);
            state->mode_remove = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTORADIOBUTTON, 160, 212, 300, 22, hwnd, (HMENU) IDC_MODE_REMOVE, NULL, NULL);
            combo_add_utf8(state->lang_combo, "English");
            combo_add_utf8(state->lang_combo, "한국어");
            SendMessageW(state->lang_combo, CB_SETCURSEL, LANG_EN, 0);
            state->check_path = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX, 32, 132, 520, 22, hwnd, (HMENU) IDC_CHECK_PATH, NULL, NULL);
            state->check_examples = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX, 32, 162, 520, 22, hwnd, (HMENU) IDC_CHECK_EXAMPLES, NULL, NULL);
            state->check_editor = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX, 32, 192, 520, 22, hwnd, (HMENU) IDC_CHECK_EDITOR, NULL, NULL);
            state->check_tests = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX, 32, 222, 520, 22, hwnd, (HMENU) IDC_CHECK_TESTS, NULL, NULL);
            state->summary = CreateWindowW(L"EDIT", L"", WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 24, 132, 560, 190, hwnd, (HMENU) IDC_SUMMARY, NULL, NULL);
            state->progress = CreateWindowExW(0, PROGRESS_CLASSW, L"", WS_CHILD | PBS_MARQUEE, 24, 144, 560, 18, hwnd, (HMENU) IDC_PROGRESS, NULL, NULL);
            state->status = CreateWindowW(L"EDIT", L"", WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 24, 176, 560, 146, hwnd, (HMENU) IDC_STATUS, NULL, NULL);
            state->back = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 332, 376, 90, 32, hwnd, (HMENU) IDC_BACK, NULL, NULL);
            state->next = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_DEFPUSHBUTTON, 432, 376, 90, 32, hwnd, (HMENU) IDC_NEXT, NULL, NULL);
            state->cancel = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 532, 376, 90, 32, hwnd, (HMENU) IDC_CANCEL, NULL, NULL);
            apply_control_font(state->title, g_font_title);
            apply_control_font(state->body, g_font_body);
            apply_control_font(state->lang_label, g_font_ui);
            apply_control_font(state->lang_combo, g_font_ui);
            apply_control_font(state->mode_label, g_font_ui);
            apply_control_font(state->mode_install, g_font_ui);
            apply_control_font(state->mode_repair, g_font_ui);
            apply_control_font(state->mode_remove, g_font_ui);
            apply_control_font(state->check_path, g_font_ui);
            apply_control_font(state->check_examples, g_font_ui);
            apply_control_font(state->check_editor, g_font_ui);
            apply_control_font(state->check_tests, g_font_ui);
            apply_control_font(state->summary, g_font_ui);
            apply_control_font(state->status, g_font_ui);
            apply_control_font(state->back, g_font_ui);
            apply_control_font(state->next, g_font_ui);
            apply_control_font(state->cancel, g_font_ui);
            SendMessageA(state->progress, PBM_SETBARCOLOR, 0, (LPARAM) UI_ACCENT);
            SendMessageA(state->progress, PBM_SETBKCOLOR, 0, (LPARAM) UI_ACCENT_SOFT);
            show_page(state);
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_LANG_COMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        read_options(state);
                        {
                            wchar_t *window_title = wide_from_utf8(state->ctx.repair_mode ? tr(state->options.language_index, "wizard_repair") : tr(state->options.language_index, "wizard_setup"));
                            SetWindowTextW(hwnd, window_title);
                            free(window_title);
                        }
                        show_page(state);
                    }
                    return 0;
                case IDC_MODE_INSTALL:
                case IDC_MODE_REPAIR:
                case IDC_MODE_REMOVE:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        read_options(state);
                        show_page(state);
                    }
                    return 0;
                case IDC_BACK:
                    if (state->page > 0 && state->page < 3) {
                        state->page--;
                        show_page(state);
                    }
                    return 0;
                case IDC_NEXT:
                    if (state->page == 0) {
                        state->page = 1;
                        show_page(state);
                    } else if (state->page == 1) {
                        state->page = 2;
                        show_page(state);
                    } else if (state->page == 2) {
                        start_install(state);
                    } else if (state->page >= 4) {
                        DestroyWindow(hwnd);
                    }
                    return 0;
                case IDC_CANCEL:
                    if (!state->running) DestroyWindow(hwnd);
                    return 0;
            }
            return 0;
        case WM_DRAWITEM:
            {
                DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
                if (wParam == IDC_BACK || wParam == IDC_CANCEL) {
                    draw_button_face(dis, false);
                    return TRUE;
                }
                if (wParam == IDC_NEXT) {
                    draw_button_face(dis, true);
                    return TRUE;
                }
            }
            break;
        case WM_CTLCOLOREDIT:
            SetTextColor((HDC) wParam, UI_TEXT);
            SetBkColor((HDC) wParam, UI_PANEL_ALT);
            return (LRESULT) g_brush_panel_alt;
        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC) wParam, UI_TEXT);
            if (state && (((HWND) lParam) == state->title || ((HWND) lParam) == state->body)) {
                SetBkColor((HDC) wParam, UI_BG);
                return (LRESULT) g_brush_bg;
            }
            if (state && (((HWND) lParam) == state->summary || ((HWND) lParam) == state->status)) {
                SetBkColor((HDC) wParam, UI_PANEL_ALT);
                return (LRESULT) g_brush_panel_alt;
            }
            SetBkColor((HDC) wParam, UI_PANEL);
            return (LRESULT) g_brush_panel;
        case WM_CTLCOLORBTN:
            SetTextColor((HDC) wParam, UI_TEXT);
            if (state && ((((HWND) lParam) == state->back) || (((HWND) lParam) == state->next) || (((HWND) lParam) == state->cancel))) {
                SetBkColor((HDC) wParam, UI_BG);
                return (LRESULT) g_brush_bg;
            }
            SetBkColor((HDC) wParam, UI_PANEL);
            return (LRESULT) g_brush_panel;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                RECT rc;
                RECT sidebar;
                RECT content;
                RECT card;
                RECT accent_bar;
                RECT glow;
                RECT top_line;
                HDC hdc = BeginPaint(hwnd, &ps);
                GetClientRect(hwnd, &rc);
                FillRect(hdc, &rc, g_brush_bg);
                sidebar = rc;
                sidebar.right = 184;
                FillRect(hdc, &sidebar, g_brush_sidebar);
                accent_bar = sidebar;
                accent_bar.left = 0;
                accent_bar.top = 0;
                accent_bar.right = 6;
                FillRect(hdc, &accent_bar, g_brush_accent);
                glow.left = 184;
                glow.top = 0;
                glow.right = rc.right;
                glow.bottom = 108;
                FillRect(hdc, &glow, g_brush_bg);
                top_line = glow;
                top_line.top = 22;
                top_line.bottom = 24;
                top_line.left = 220;
                top_line.right = rc.right - 28;
                FillRect(hdc, &top_line, g_brush_accent);
                content = rc;
                content.left = 184;
                FillRect(hdc, &content, g_brush_bg);
                card.left = 220;
                card.top = 150;
                card.right = rc.right - 24;
                card.bottom = rc.bottom - 64;
                draw_round_panel(hdc, &card, UI_PANEL, UI_PANEL_EDGE, 16);
                draw_text_utf8(hdc, 28, 28, 120, 34, DT_LEFT | DT_VCENTER | DT_SINGLELINE, UI_TEXT, g_font_brand, "Spear");
                draw_text_utf8(hdc, 28, 61, 120, 18, DT_LEFT | DT_VCENTER | DT_SINGLELINE, UI_MUTED, g_font_ui, "Premium installer");
                draw_text_utf8(hdc, 28, rc.bottom - 78, 120, 18, DT_LEFT | DT_VCENTER | DT_SINGLELINE, UI_MUTED, g_font_ui, "v0.1.0");
                draw_step_item(hdc, 124, 1, state && state->page == 0, "Welcome");
                draw_step_item(hdc, 164, 2, state && state->page == 1, "Options");
                draw_step_item(hdc, 204, 3, state && state->page == 2, "Review");
                draw_step_item(hdc, 244, 4, state && state->page == 3, "Install");
                draw_step_item(hdc, 284, 5, state && state->page >= 4, "Finish");
                EndPaint(hwnd, &ps);
            }
            return 0;
        case WM_APP_INSTALL_DONE:
            state->running = false;
            if (state->worker) {
                CloseHandle(state->worker);
                state->worker = NULL;
            }
            SendMessageA(state->progress, PBM_SETMARQUEE, FALSE, 0);
            state->page = 4;
            show_page(state);
            return 0;
        case WM_CLOSE:
            if (!state || !state->running) DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void run_wizard(SetupContext *ctx) {
    WNDCLASSW wc;
    MSG msg;
    INITCOMMONCONTROLSEX icc;
    WizardState state;
    ZeroMemory(&state, sizeof(state));
    state.ctx = *ctx;
    state.options.install_path = true;
    state.options.install_examples = ctx->has_examples;
    state.options.install_editor = ctx->has_editor;
    state.options.run_checks = true;
    state.options.language_index = LANG_EN;
    state.options.operation = ctx->repair_mode ? OP_REPAIR : (ctx->existing_install ? OP_REPAIR : OP_INSTALL);
    ZeroMemory(&icc, sizeof(icc));
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = wizard_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = APP_CLASS_W;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR) IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    RegisterClassW(&wc);
    {
        wchar_t *window_title = wide_from_utf8(ctx->repair_mode ? tr(LANG_EN, "wizard_repair") : tr(LANG_EN, "wizard_setup"));
        CreateWindowExW(WS_EX_DLGMODALFRAME, APP_CLASS_W, window_title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 680, 460, NULL, NULL, GetModuleHandleA(NULL), &state);
        free(window_title);
    }
    ShowWindow(state.hwnd, SW_SHOW);
    UpdateWindow(state.hwnd);
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

int main(void) {
    SetupContext ctx;
    ZeroMemory(&ctx, sizeof(ctx));
    discover_context(&ctx);
    if (ctx.uninstall_mode) {
        perform_uninstall(&ctx);
        if (ctx.temp_payload_root[0]) remove_tree_recursive(ctx.temp_payload_root);
        return 0;
    }
    run_wizard(&ctx);
    if (ctx.temp_payload_root[0]) remove_tree_recursive(ctx.temp_payload_root);
    return 0;
}
