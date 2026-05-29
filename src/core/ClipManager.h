#pragma once
#include "Common.h"
#include <string>
#include <vector>

// Clipboard backup/restore and text access.
// Used by the fix-selection mode.
class ClipManager {
public:
    // Backup current clipboard contents (text only for now).
    void backup();
    // Restore the previously backed-up clipboard.
    void restore();

    // Get Unicode text from clipboard. Returns empty string if none.
    std::wstring getText() const;
    // Set Unicode text in clipboard.
    bool setText(const std::wstring& text);

    // Wait until the clipboard sequence number changes (clipboard was updated).
    // Returns true if change detected within timeoutMs.
    bool waitForChange(int timeoutMs = 500) const;

private:
    std::wstring  m_backup;
    bool          m_hasBackup = false;
    DWORD         m_seqBefore = 0;
};
