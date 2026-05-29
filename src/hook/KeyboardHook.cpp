#include "KeyboardHook.h"

KeyboardHook* KeyboardHook::s_instance = nullptr;

bool KeyboardHook::install(Callback cb) {
    if (m_hook) return true;
    m_callback = std::move(cb);
    s_instance = this;
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookProc, nullptr, 0);
    return m_hook != nullptr;
}

void KeyboardHook::uninstall() {
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
    if (s_instance == this) s_instance = nullptr;
}

LRESULT CALLBACK KeyboardHook::hookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !s_instance)
        return CallNextHookEx(nullptr, nCode, wParam, lParam);

    auto* ks = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

    // Skip events injected by ourselves
    if (ks->dwExtraInfo == kInjectedId)
        return CallNextHookEx(nullptr, nCode, wParam, lParam);

    const bool isDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    const bool isUp   = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

    if (!isDown && !isUp)
        return CallNextHookEx(nullptr, nCode, wParam, lParam);

    // Build key info
    KeyInfo ki;
    ki.vk       = static_cast<WORD>(ks->vkCode);
    ki.scan     = static_cast<WORD>(ks->scanCode);
    ki.extended = (ks->flags & LLKHF_EXTENDED) != 0;
    ki.shift    = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
    ki.caps     = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    // Get layout of the foreground window's thread
    HWND  hwnd = GetForegroundWindow();
    DWORD tid  = hwnd ? GetWindowThreadProcessId(hwnd, nullptr) : 0;
    ki.layout  = GetKeyboardLayout(tid);

    // Classify key type using VK codes (no ToUnicodeEx in hook — avoids dead-key side effects)
    WORD vk = ki.vk;
    const bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool altDown  = (GetKeyState(VK_MENU)    & 0x8000) != 0;

    if (vk == VK_SPACE || vk == VK_RETURN || vk == VK_TAB) {
        ki.type = KeyType::Space;
    } else if (ctrlDown || altDown ||
               vk == VK_BACK || vk == VK_DELETE || vk == VK_ESCAPE ||
               vk == VK_LEFT || vk == VK_RIGHT || vk == VK_UP || vk == VK_DOWN ||
               vk == VK_HOME || vk == VK_END || vk == VK_PRIOR || vk == VK_NEXT ||
               vk == VK_INSERT || vk == VK_SNAPSHOT || vk == VK_SCROLL || vk == VK_PAUSE ||
               vk == VK_CANCEL || vk == VK_NUMLOCK || vk == VK_CAPITAL ||
               (vk >= VK_F1 && vk <= VK_F24) ||
               vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
               vk == VK_LSHIFT || vk == VK_RSHIFT ||
               vk == VK_LCONTROL || vk == VK_RCONTROL ||
               vk == VK_LMENU || vk == VK_RMENU ||
               vk == VK_LWIN || vk == VK_RWIN || vk == VK_APPS) {
        ki.type = KeyType::Command;
    } else if ((vk >= 'A' && vk <= 'Z')) {
        // VK_A–VK_Z produce letters in all Latin/Cyrillic layouts
        ki.type = KeyType::Letter;
    } else {
        // Digits, OEM keys, numpad — treat as Custom (kept during fix)
        ki.type = KeyType::Custom;
    }

    bool suppress = false;
    if (s_instance->m_callback)
        suppress = s_instance->m_callback(ki, isDown);

    if (suppress) return 1;
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
