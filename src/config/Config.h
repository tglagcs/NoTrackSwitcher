#pragma once
#include "Common.h"
#include <string>

struct AppConfig {
    Hotkey hkFixSelection;  // default: Shift+Pause

    bool enabled   = true;
    bool autoStart = false;
};

class Config {
public:
    // Load config from the given path. Returns defaults if file not found.
    bool load(const std::wstring& path);
    // Save config to the given path.
    bool save(const std::wstring& path) const;

    // Get/set the config path (default: exe directory + "\\notrack.ini")
    static std::wstring defaultPath();

    AppConfig data;

private:
    std::wstring m_path;
};

// Hotkey helpers (used by Config and SettingsDlg)
std::wstring hotkeyToString(const Hotkey& hk);
Hotkey       hotkeyFromString(const std::wstring& s);
