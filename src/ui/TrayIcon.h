#pragma once
#include "Common.h"
#include <shellapi.h>
#include <functional>
#include <string>

// System tray icon with context menu.
class TrayIcon {
public:
    ~TrayIcon() { destroy(); }

    // Create the tray icon. hwndOwner receives WM_TRAY_NOTIFY.
    bool create(HWND hwndOwner, HINSTANCE hInst);
    void destroy();

    // Update icon and tooltip to reflect enabled/disabled state.
    void setEnabled(bool enabled);
    // Update the tooltip text.
    void setTooltip(const std::wstring& tip);

    // Called from WndProc when WM_TRAY_NOTIFY is received.
    // Returns true if handled.
    bool handleNotify(WPARAM wParam, LPARAM lParam);

    // Callbacks
    std::function<void()> onEnable;
    std::function<void()> onSettings;
    std::function<void()> onExit;

private:
    void showContextMenu();

    NOTIFYICONDATAW m_nid    = {};
    HWND            m_hwnd   = nullptr;
    HINSTANCE       m_hInst  = nullptr;
    HICON           m_hIconOn  = nullptr;
    HICON           m_hIconOff = nullptr;
    bool            m_enabled  = true;
    bool            m_created  = false;
};
