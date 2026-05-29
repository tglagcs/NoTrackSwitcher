#pragma once
#include "Common.h"
#include <vector>
#include <unordered_map>

// Pre-computed character table for one keyboard layout.
// Built once at startup via ToUnicodeEx for all VK codes.
struct LayoutMap {
    HKL     hkl = nullptr;
    // normal, shifted, and AltGr (ctrl+alt) mappings indexed by VK code
    wchar_t normal[256]  = {};
    wchar_t shifted[256] = {};
    wchar_t altgr[256]   = {};
};

// Manages installed keyboard layouts and provides character-level mapping.
class LayoutManager {
public:
    // Build layout tables for all currently installed layouts.
    void refresh();

    // Return the next HKL in the cycle after 'current'.
    HKL nextLayout(HKL current) const;

    // Return the HKL that most likely produced these keys
    // (the layout in which the characters are "readable" letters of that layout).
    HKL detectSourceLayout(const std::vector<KeyInfo>& keys) const;

    // Switch the foreground window's layout to 'target'.
    void switchForegroundLayout(HKL target) const;

    const std::vector<LayoutMap>& layouts() const noexcept { return m_layouts; }

    // Look up the wchar_t produced by a given VK + shift/caps in a layout.
    wchar_t charForKey(const LayoutMap& lm, WORD vk, bool shift, bool caps) const noexcept;

    // Find which VK + shift in layout 'lm' produces character 'ch'.
    // Returns false if not found.
    bool keyForChar(const LayoutMap& lm, wchar_t ch, WORD& outVk, bool& outShift) const noexcept;

    const LayoutMap* findMap(HKL hkl) const noexcept;

private:
    void buildMap(HKL hkl, LayoutMap& out) const;

    std::vector<LayoutMap> m_layouts;
};
