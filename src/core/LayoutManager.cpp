#include "LayoutManager.h"
#include <algorithm>

void LayoutManager::refresh() {
    m_layouts.clear();

    int count = GetKeyboardLayoutList(0, nullptr);
    if (count <= 0) return;

    std::vector<HKL> hkls(count);
    GetKeyboardLayoutList(count, hkls.data());

    m_layouts.reserve(count);
    for (HKL hkl : hkls) {
        LayoutMap lm;
        lm.hkl = hkl;
        buildMap(hkl, lm);
        m_layouts.push_back(lm);
    }
}

void LayoutManager::buildMap(HKL hkl, LayoutMap& out) const {
    // Activating the layout in ToUnicodeEx requires a key state buffer.
    // We cycle through all VK codes and query each combination.
    BYTE kstate[256] = {};

    auto query = [&](WORD vk, const BYTE* ks) -> wchar_t {
        UINT scan = MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC, hkl);
        wchar_t ch = 0;
        // Try up to twice to skip dead-key state
        for (int attempt = 0; attempt < 2; ++attempt) {
            wchar_t tmp[4] = {};
            int r = ToUnicodeEx(vk, scan, ks, tmp, 4, 0, hkl);
            if (r == 1) { ch = tmp[0]; break; }
            if (r == -1) {
                // Dead key — press Space to flush the dead-key buffer, then retry
                BYTE empty[256] = {};
                wchar_t sp[4] = {};
                ToUnicodeEx(VK_SPACE, MapVirtualKeyEx(VK_SPACE, MAPVK_VK_TO_VSC, hkl),
                            empty, sp, 4, 0, hkl);
                // result is ignored; on retry we'll get the combined char or 0
            } else {
                break;
            }
        }
        return ch;
    };

    for (int vk = 0; vk < 256; ++vk) {
        // Normal
        memset(kstate, 0, sizeof(kstate));
        out.normal[vk] = query(static_cast<WORD>(vk), kstate);

        // Shifted
        memset(kstate, 0, sizeof(kstate));
        kstate[VK_SHIFT] = 0x80;
        out.shifted[vk] = query(static_cast<WORD>(vk), kstate);

        // AltGr (Ctrl+Alt) — common on European layouts
        memset(kstate, 0, sizeof(kstate));
        kstate[VK_CONTROL] = 0x80;
        kstate[VK_MENU]    = 0x80;
        out.altgr[vk] = query(static_cast<WORD>(vk), kstate);
    }
}

HKL LayoutManager::nextLayout(HKL current) const {
    if (m_layouts.empty()) return current;
    for (size_t i = 0; i < m_layouts.size(); ++i) {
        if (m_layouts[i].hkl == current) {
            return m_layouts[(i + 1) % m_layouts.size()].hkl;
        }
    }
    return m_layouts[0].hkl;
}

HKL LayoutManager::detectSourceLayout(const std::vector<KeyInfo>& keys) const {
    if (keys.empty() || m_layouts.empty()) return nullptr;

    // Use the layout stored in the first key (recorded at type time)
    HKL candidate = keys.front().layout;
    // Verify it's in our list
    for (auto& lm : m_layouts)
        if (lm.hkl == candidate) return candidate;

    return m_layouts.front().hkl;
}

void LayoutManager::switchForegroundLayout(HKL target) const {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;
    PostMessage(hwnd, WM_INPUTLANGCHANGEREQUEST, 0, reinterpret_cast<LPARAM>(target));
}

wchar_t LayoutManager::charForKey(const LayoutMap& lm, WORD vk, bool shift, bool caps) const noexcept {
    // CapsLock flips the case of alphabetic keys
    bool effectiveShift = shift;
    if (caps && vk >= 'A' && vk <= 'Z') effectiveShift = !effectiveShift;

    wchar_t ch = effectiveShift ? lm.shifted[vk] : lm.normal[vk];
    return ch;
}

bool LayoutManager::keyForChar(const LayoutMap& lm, wchar_t ch, WORD& outVk, bool& outShift) const noexcept {
    if (!ch) return false;
    for (int vk = 0; vk < 256; ++vk) {
        if (lm.normal[vk] == ch)  { outVk = static_cast<WORD>(vk); outShift = false; return true; }
        if (lm.shifted[vk] == ch) { outVk = static_cast<WORD>(vk); outShift = true;  return true; }
    }
    return false;
}

const LayoutMap* LayoutManager::findMap(HKL hkl) const noexcept {
    for (auto& lm : m_layouts)
        if (lm.hkl == hkl) return &lm;
    return nullptr;
}
