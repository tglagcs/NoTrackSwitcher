#pragma once
#include "Common.h"
#include <functional>

// Low-level mouse hook (WH_MOUSE_LL).
// Used only to detect clicks — clears the text buffer on any mouse button event.
class MouseHook {
public:
    using Callback = std::function<void()>;

    MouseHook() = default;
    ~MouseHook() { uninstall(); }

    MouseHook(const MouseHook&) = delete;
    MouseHook& operator=(const MouseHook&) = delete;

    bool install(Callback onButtonDown);
    void uninstall();

private:
    static LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static MouseHook* s_instance;

    HHOOK    m_hook = nullptr;
    Callback m_callback;
};
