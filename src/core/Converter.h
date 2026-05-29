#pragma once
#include "Common.h"
#include "LayoutManager.h"
#include <vector>
#include <string>

// Converts typed key sequences or plain text between keyboard layouts.
class Converter {
public:
    // Convert a sequence of KeyInfo (recorded in srcLayout) to new KeyInfo
    // suitable for replay in dstLayout.
    //
    // Strategy: for each key, look up the character it produces in srcLayout,
    // then find which key in dstLayout produces that character. If not found,
    // keep the original key (pass-through).
    std::vector<KeyInfo> convertKeys(
        const std::vector<KeyInfo>& keys,
        const LayoutMap& srcMap,
        const LayoutMap& dstMap,
        const LayoutManager& lm) const;

    // Convert a Unicode string from srcLayout character set to dstLayout.
    // Used for the clipboard-based fix-selection mode.
    std::wstring convertText(
        const std::wstring& text,
        const LayoutMap& srcMap,
        const LayoutMap& dstMap) const;
};
