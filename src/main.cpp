#include "Common.h"
#include "hook/KeyboardHook.h"
#include "hook/MouseHook.h"
#include "core/TextBuffer.h"
#include "core/LayoutManager.h"
#include "core/Worker.h"
#include "config/Config.h"
#include "ui/TrayIcon.h"
#include "ui/SettingsDlg.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "psapi.lib")
#endif

// Forward declarations
static LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);
static void applyAutostart(bool enable);

// Global objects (linked to each other via lambdas below)
static Config        g_config;
static TextBuffer    g_buf;
static LayoutManager g_layouts;
static Worker*       g_worker   = nullptr;
static KeyboardHook  g_kbHook;
static MouseHook     g_msHook;
static TrayIcon      g_tray;
static HWND          g_hwndMain = nullptr;
static HINSTANCE     g_hInst    = nullptr;

// ---- Keyboard hook callback -----------------------------------------------
static bool onKey(const KeyInfo& key, bool isDown) {
    if (!g_config.data.enabled) return false;
    if (!isDown) return false;

    bool shift = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
    bool ctrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    bool alt   = (GetKeyState(VK_MENU)    & 0x8000) != 0;
    bool win   = (GetKeyState(VK_LWIN)    & 0x8000) ||
                 (GetKeyState(VK_RWIN)    & 0x8000);

    if (g_config.data.hkFixSelection.matches(key.vk, shift, ctrl, alt, win)) {
        if (g_worker)
            PostThreadMessage(g_worker->threadId(), WM_WORKER_FIX_SELECTION, 0, 0);
        return true;
    }

    g_buf.push(key);
    return false;
}

// ---- Window procedure (hidden message-only window) ------------------------
static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAY_NOTIFY:
            g_tray.handleNotify(wParam, lParam);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ---- Entry point ----------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    g_hInst = hInst;

    // Single instance guard
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"NoTrackSwitcher_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 0;
    }

    // Load config
    std::wstring cfgPath = Config::defaultPath();
    g_config.load(cfgPath);

    // Build layout tables
    g_layouts.refresh();

    // Create hidden message-only window (needed for tray icon and message pump)
    WNDCLASSEXW wc    = {};
    wc.cbSize         = sizeof(wc);
    wc.lpfnWndProc    = mainWndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = L"NoTrackSwitcherWnd";
    RegisterClassExW(&wc);

    g_hwndMain = CreateWindowExW(0, L"NoTrackSwitcherWnd", L"NoTrack Switcher",
                                  0, 0, 0, 0, 0,
                                  HWND_MESSAGE, nullptr, hInst, nullptr);

    // Create and start worker thread
    g_worker = new Worker(g_buf, g_layouts);
    g_worker->enabled = g_config.data.enabled;
    g_worker->start();

    // Install keyboard and mouse hooks
    g_kbHook.install(onKey);
    g_msHook.install([&]() {
        // Mouse click → clear text buffer
        g_buf.clear();
    });

    // Create tray icon
    g_tray.create(g_hwndMain, hInst);

    g_tray.onEnable = [&]() {
        g_config.data.enabled = !g_config.data.enabled;
        g_worker->enabled     =  g_config.data.enabled;
        g_tray.setEnabled(g_config.data.enabled);
        g_config.save(cfgPath);
    };

    g_tray.onSettings = [&]() {
        HWND existing = SettingsDlg::currentHwnd();
        if (existing) { SetForegroundWindow(existing); return; }

        SettingsDlg dlg(g_config.data);
        if (dlg.show(hInst, nullptr)) {
            g_worker->enabled = g_config.data.enabled;
            g_tray.setEnabled(g_config.data.enabled);
            g_layouts.refresh();
            applyAutostart(g_config.data.autoStart);
            g_config.save(cfgPath);
        }
    };

    g_tray.onExit = [&]() {
        DestroyWindow(g_hwndMain);
    };

    // Message loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup
    g_kbHook.uninstall();
    g_msHook.uninstall();
    g_tray.destroy();
    if (g_worker) { delete g_worker; g_worker = nullptr; }
    CloseHandle(hMutex);
    return 0;
}

// ---- Autostart via registry -----------------------------------------------
static void applyAutostart(bool enable) {
    HKEY hKey = nullptr;
    RegOpenKeyExW(HKEY_CURRENT_USER,
                  L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  0, KEY_SET_VALUE, &hKey);
    if (!hKey) return;

    if (enable) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring val = std::wstring(L"\"") + path + L"\"";
        RegSetValueExW(hKey, L"NoTrackSwitcher", 0, REG_SZ,
                       reinterpret_cast<const BYTE*>(val.c_str()),
                       static_cast<DWORD>((val.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(hKey, L"NoTrackSwitcher");
    }
    RegCloseKey(hKey);
}
