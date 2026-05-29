#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vector>
#include <deque>
#include <string>
#include <cstdint>
#include <functional>

// Tag own injected key events so the hook can ignore them
static constexpr ULONG_PTR kInjectedId = 0x4E545331UL; // 'NTS1'

// Worker thread messages (PostThreadMessage)
static constexpr UINT WM_WORKER_FIX_WORD      = WM_APP + 1;
static constexpr UINT WM_WORKER_FIX_SELECTION = WM_APP + 2;
static constexpr UINT WM_WORKER_CLEAR_BUFFER  = WM_APP + 3;
static constexpr UINT WM_WORKER_QUIT          = WM_APP + 4;
static constexpr UINT WM_WORKER_RELOAD_CONFIG = WM_APP + 5;

// Tray icon callback message
static constexpr UINT WM_TRAY_NOTIFY = WM_APP + 10;

enum class KeyType : uint8_t {
    Letter,   // printable letter — converted when switching layout
    Space,    // word boundary: space, enter, tab
    Custom,   // punctuation — layout-dependent, kept as-is
    Command,  // control key (Ctrl, Alt, Del, etc.) — clears buffer
};

struct KeyInfo {
    WORD    vk       = 0;
    WORD    scan     = 0;
    bool    extended = false;
    bool    shift    = false;
    bool    caps     = false;
    HKL     layout   = nullptr;
    KeyType type     = KeyType::Command;
};

struct Hotkey {
    WORD vk    = 0;
    bool shift = false;
    bool ctrl  = false;
    bool alt   = false;
    bool win   = false;

    bool valid() const noexcept { return vk != 0; }
    bool matches(WORD vkCode, bool s, bool c, bool a, bool w) const noexcept {
        return valid() && vk == vkCode && shift == s && ctrl == c && alt == a && win == w;
    }

    std::wstring toString() const;
    static Hotkey fromString(const std::wstring& s);
};
