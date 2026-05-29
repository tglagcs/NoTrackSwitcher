#include "Converter.h"

std::vector<KeyInfo> Converter::convertKeys(
    const std::vector<KeyInfo>& keys,
    const LayoutMap& srcMap,
    const LayoutMap& dstMap,
    const LayoutManager& lm) const
{
    std::vector<KeyInfo> result;
    result.reserve(keys.size());

    for (const auto& k : keys) {
        if (k.type == KeyType::Space) {
            result.push_back(k);
            continue;
        }

        // Determine the character this key produced in the source layout
        wchar_t ch = lm.charForKey(srcMap, k.vk, k.shift, k.caps);
        if (!ch) {
            // No character mapping — keep the key as-is (punct, etc.)
            result.push_back(k);
            continue;
        }

        // Find which key produces that character in the destination layout
        WORD  dstVk    = 0;
        bool  dstShift = false;
        if (lm.keyForChar(dstMap, ch, dstVk, dstShift)) {
            KeyInfo out  = k;
            out.vk       = dstVk;
            out.scan     = static_cast<WORD>(MapVirtualKeyEx(dstVk, MAPVK_VK_TO_VSC, dstMap.hkl));
            out.shift    = dstShift;
            out.layout   = dstMap.hkl;
            result.push_back(out);
        } else {
            // Character not in dst layout — keep original
            result.push_back(k);
        }
    }
    return result;
}

std::wstring Converter::convertText(
    const std::wstring& text,
    const LayoutMap& srcMap,
    const LayoutMap& dstMap) const
{
    std::wstring result;
    result.reserve(text.size());

    for (wchar_t ch : text) {
        // Find VK that produces ch in srcMap
        bool found = false;
        for (int vk = 0; vk < 256 && !found; ++vk) {
            if (srcMap.normal[vk] == ch || srcMap.shifted[vk] == ch) {
                // Find the corresponding char in dstMap at same VK
                wchar_t dstCh = (srcMap.shifted[vk] == ch) ? dstMap.shifted[vk] : dstMap.normal[vk];
                if (dstCh) { result += dstCh; found = true; }
            }
        }
        if (!found) result += ch;  // pass through (digit, space, etc.)
    }
    return result;
}
