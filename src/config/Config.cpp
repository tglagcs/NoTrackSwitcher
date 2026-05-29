#include "Config.h"
#include <fstream>
#include <cwchar>
#include <cwctype>
#include <map>

// Portable case-insensitive wide-string comparison (replaces _wcsicmp / _wcsnicmp)
static int wcs_icmp(const wchar_t* a, const wchar_t* b) {
    while (*a && *b) {
        wchar_t ca = towlower(static_cast<wint_t>(*a));
        wchar_t cb = towlower(static_cast<wint_t>(*b));
        if (ca != cb) return static_cast<int>(ca) - static_cast<int>(cb);
        ++a; ++b;
    }
    return static_cast<int>(towlower(static_cast<wint_t>(*a))) -
           static_cast<int>(towlower(static_cast<wint_t>(*b)));
}

static int wcs_nicmp(const wchar_t* a, const wchar_t* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        wchar_t ca = towlower(static_cast<wint_t>(a[i]));
        wchar_t cb = towlower(static_cast<wint_t>(b[i]));
        if (ca != cb) return static_cast<int>(ca) - static_cast<int>(cb);
        if (!ca) return 0;
    }
    return 0;
}

// ---- Hotkey serialization -----------------------------------------------

struct VkEntry { const wchar_t* name; WORD vk; };
static const VkEntry kVkTable[] = {
    { L"Pause",     VK_PAUSE   }, { L"Break",    VK_CANCEL  },
    { L"Insert",    VK_INSERT  }, { L"Delete",   VK_DELETE  },
    { L"Home",      VK_HOME    }, { L"End",      VK_END     },
    { L"PageUp",    VK_PRIOR   }, { L"PageDown", VK_NEXT    },
    { L"F1",  VK_F1  }, { L"F2",  VK_F2  }, { L"F3",  VK_F3  },
    { L"F4",  VK_F4  }, { L"F5",  VK_F5  }, { L"F6",  VK_F6  },
    { L"F7",  VK_F7  }, { L"F8",  VK_F8  }, { L"F9",  VK_F9  },
    { L"F10", VK_F10 }, { L"F11", VK_F11 }, { L"F12", VK_F12 },
    { L"Tab",        VK_TAB      }, { L"Space",    VK_SPACE    },
    { L"Back",       VK_BACK     }, { L"Esc",      VK_ESCAPE   },
    { L"ScrollLock", VK_SCROLL   }, { L"NumLock",  VK_NUMLOCK  },
    { L"PrintScreen",VK_SNAPSHOT },
};

std::wstring hotkeyToString(const Hotkey& hk) {
    if (!hk.valid()) return L"";
    std::wstring s;
    if (hk.ctrl)  s += L"Ctrl+";
    if (hk.alt)   s += L"Alt+";
    if (hk.shift) s += L"Shift+";
    if (hk.win)   s += L"Win+";

    for (auto& e : kVkTable)
        if (e.vk == hk.vk) { s += e.name; return s; }

    if ((hk.vk >= 'A' && hk.vk <= 'Z') || (hk.vk >= '0' && hk.vk <= '9')) {
        s += static_cast<wchar_t>(hk.vk);
        return s;
    }
    // Fallback: hex (use swprintf with explicit buffer size — portable)
    wchar_t buf[16] = {};
    swprintf(buf, 16, L"0x%02X", hk.vk);
    s += buf;
    return s;
}

Hotkey hotkeyFromString(const std::wstring& raw) {
    Hotkey hk;
    if (raw.empty()) return hk;

    std::wstring s = raw;
    auto consume = [&](const std::wstring& prefix) -> bool {
        if (s.size() >= prefix.size() &&
            wcs_nicmp(s.c_str(), prefix.c_str(), prefix.size()) == 0) {
            s = s.substr(prefix.size());
            return true;
        }
        return false;
    };

    while (true) {
        if      (consume(L"Ctrl+"))  hk.ctrl  = true;
        else if (consume(L"Alt+"))   hk.alt   = true;
        else if (consume(L"Shift+")) hk.shift = true;
        else if (consume(L"Win+"))   hk.win   = true;
        else break;
    }

    for (auto& e : kVkTable)
        if (wcs_icmp(s.c_str(), e.name) == 0) { hk.vk = e.vk; return hk; }

    if (s.size() == 1) {
        wchar_t c = static_cast<wchar_t>(towupper(static_cast<wint_t>(s[0])));
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            hk.vk = static_cast<WORD>(c);
            return hk;
        }
    }

    if (s.size() > 2 && s[0] == L'0' && towupper(static_cast<wint_t>(s[1])) == L'X') {
        hk.vk = static_cast<WORD>(wcstoul(s.c_str(), nullptr, 16));
    }
    return hk;
}

// ---- INI parsing --------------------------------------------------------

static std::wstring trim(const std::wstring& s) {
    size_t a = s.find_first_not_of(L" \t\r\n");
    size_t b = s.find_last_not_of(L" \t\r\n");
    return (a == std::wstring::npos) ? L"" : s.substr(a, b - a + 1);
}

using IniMap = std::map<std::wstring, std::map<std::wstring, std::wstring>>;

static IniMap parseIni(std::wifstream& f) {
    IniMap result;
    std::wstring section, line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == L';' || line[0] == L'#') continue;
        if (line[0] == L'[') {
            size_t end = line.find(L']');
            if (end != std::wstring::npos)
                section = trim(line.substr(1, end - 1));
        } else {
            size_t eq = line.find(L'=');
            if (eq != std::wstring::npos) {
                std::wstring key = trim(line.substr(0, eq));
                std::wstring val = trim(line.substr(eq + 1));
                result[section][key] = val;
            }
        }
    }
    return result;
}

// ---- Config::load / save ------------------------------------------------

bool Config::load(const std::wstring& path) {
    m_path = path;
    data.hkFixSelection = hotkeyFromString(L"Ctrl+Shift+F");
    data.enabled        = true;
    data.autoStart      = false;

    std::wifstream f(path.c_str());
    if (!f.is_open()) return false;

    auto ini = parseIni(f);

    auto get = [&](const std::wstring& sec, const std::wstring& key,
                   const std::wstring& def) -> std::wstring {
        auto si = ini.find(sec);
        if (si == ini.end()) return def;
        auto ki = si->second.find(key);
        return (ki == si->second.end()) ? def : ki->second;
    };

    data.hkFixSelection = hotkeyFromString(get(L"hotkeys", L"fix_selection", L"Ctrl+Shift+F"));
    data.enabled        = get(L"behavior", L"enabled",    L"true")  != L"false";
    data.autoStart      = get(L"behavior", L"auto_start", L"false") == L"true";

    return true;
}

bool Config::save(const std::wstring& path) const {
    std::wofstream f(path.c_str());
    if (!f.is_open()) return false;

    f << L"[hotkeys]\n";
    f << L"fix_selection=" << hotkeyToString(data.hkFixSelection) << L"\n";

    f << L"\n[behavior]\n";
    f << L"enabled="    << (data.enabled   ? L"true" : L"false") << L"\n";
    f << L"auto_start=" << (data.autoStart ? L"true" : L"false") << L"\n";

    return f.good();
}

std::wstring Config::defaultPath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf);
    size_t last = path.find_last_of(L"\\/");
    if (last != std::wstring::npos) path = path.substr(0, last + 1);
    return path + L"notrack.ini";
}

// ---- Hotkey member functions (declared in Common.h) ---------------------
std::wstring Hotkey::toString() const        { return hotkeyToString(*this); }
Hotkey       Hotkey::fromString(const std::wstring& s) { return hotkeyFromString(s); }
