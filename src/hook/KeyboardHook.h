#pragma once
#include "Common.h"
#include <functional>

// Low-level keyboard hook (WH_KEYBOARD_LL).
// Runs in the main thread. Callback must be fast — only updates state and
// posts messages to the worker thread. Never blocks.
class KeyboardHook {
public:
    // Called for every key event (after own-injection filter).
    // Returns true if the event should be suppressed (not forwarded to OS).
    using Callback = std::function<bool(const KeyInfo& key, bool isDown)>;

    KeyboardHook() = default;
    ~KeyboardHook() { uninstall(); }

    KeyboardHook(const KeyboardHook&) = delete;
    KeyboardHook& operator=(const KeyboardHook&) = delete;

    bool install(Callback cb);
    void uninstall();
    bool installed() const noexcept { return m_hook != nullptr; }

private:
    static LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static KeyboardHook* s_instance;

    HHOOK    m_hook    = nullptr;
    Callback m_callback;
};
