#include "TrayIcon.h"
#include "resource.h"
#include <shellapi.h>
#include <cwchar>

static constexpr UINT kTrayId = 1;

// Portable replacement for wcscpy_s — copies up to N-1 wide chars + null
static void tip_copy(wchar_t* dst, size_t dstCount, const wchar_t* src) {
    wcsncpy(dst, src, dstCount - 1);
    dst[dstCount - 1] = L'\0';
}
#define COPY_TIP(dst, src) tip_copy(dst, sizeof(dst)/sizeof(dst[0]), src)

bool TrayIcon::create(HWND hwndOwner, HINSTANCE hInst) {
    m_hwnd  = hwndOwner;
    m_hInst = hInst;

    m_hIconOn  = static_cast<HICON>(LoadImageW(hInst, MAKEINTRESOURCEW(IDI_TRAY),
                   IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    m_hIconOff = static_cast<HICON>(LoadImageW(hInst, MAKEINTRESOURCEW(IDI_TRAY_DISABLED),
                   IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));

    if (!m_hIconOn)
        m_hIconOn  = static_cast<HICON>(LoadImageW(nullptr, IDI_APPLICATION,
                       IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
    if (!m_hIconOff)
        m_hIconOff = static_cast<HICON>(LoadImageW(nullptr, IDI_INFORMATION,
                       IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));

    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize           = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd             = m_hwnd;
    m_nid.uID              = kTrayId;
    m_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAY_NOTIFY;
    m_nid.hIcon            = m_hIconOn;
    COPY_TIP(m_nid.szTip, L"NoTrack Switcher");

    m_created = Shell_NotifyIconW(NIM_ADD, &m_nid) != FALSE;
    return m_created;
}

void TrayIcon::destroy() {
    if (m_created) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_created = false;
    }
}

void TrayIcon::setEnabled(bool enabled) {
    m_enabled    = enabled;
    m_nid.hIcon  = enabled ? m_hIconOn : m_hIconOff;
    m_nid.uFlags = NIF_ICON | NIF_TIP;
    COPY_TIP(m_nid.szTip, enabled ? L"NoTrack Switcher" : L"NoTrack Switcher (disabled)");
    if (m_created) Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::setTooltip(const std::wstring& tip) {
    m_nid.uFlags = NIF_TIP;
    COPY_TIP(m_nid.szTip, tip.c_str());
    if (m_created) Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

bool TrayIcon::handleNotify(WPARAM /*wParam*/, LPARAM lParam) {
    switch (LOWORD(lParam)) {
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            showContextMenu();
            return true;
        case WM_LBUTTONDBLCLK:
            if (onSettings) onSettings();
            return true;
    }
    return false;
}

void TrayIcon::showContextMenu() {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    AppendMenuW(hMenu, MF_STRING, IDM_TRAY_ENABLE,
                m_enabled ? L"Disable" : L"Enable");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_TRAY_SETTINGS, L"Settings...");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_TRAY_EXIT, L"Exit");

    SetForegroundWindow(m_hwnd);

    POINT pt;
    GetCursorPos(&pt);
    UINT cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                               pt.x, pt.y, 0, m_hwnd, nullptr);
    DestroyMenu(hMenu);

    switch (cmd) {
        case IDM_TRAY_ENABLE:   if (onEnable)   onEnable();   break;
        case IDM_TRAY_SETTINGS: if (onSettings) onSettings(); break;
        case IDM_TRAY_EXIT:     if (onExit)     onExit();     break;
    }

    PostMessage(m_hwnd, WM_NULL, 0, 0);
}
