#include "MouseHook.h"

MouseHook* MouseHook::s_instance = nullptr;

bool MouseHook::install(Callback cb) {
    if (m_hook) return true;
    m_callback = std::move(cb);
    s_instance = this;
    m_hook = SetWindowsHookEx(WH_MOUSE_LL, hookProc, nullptr, 0);
    return m_hook != nullptr;
}

void MouseHook::uninstall() {
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
    if (s_instance == this) s_instance = nullptr;
}

LRESULT CALLBACK MouseHook::hookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && s_instance && s_instance->m_callback) {
        switch (wParam) {
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
                s_instance->m_callback();
                break;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
