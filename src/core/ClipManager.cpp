#include "ClipManager.h"

void ClipManager::backup() {
    m_hasBackup = false;
    m_seqBefore = GetClipboardSequenceNumber();
    m_backup    = getText();
    m_hasBackup = true;
}

void ClipManager::restore() {
    if (!m_hasBackup) return;
    // Small delay to let the paste complete before we overwrite
    Sleep(220);
    setText(m_backup);
    m_hasBackup = false;
}

std::wstring ClipManager::getText() const {
    std::wstring result;
    if (!OpenClipboard(nullptr)) return result;

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        const wchar_t* ptr = static_cast<const wchar_t*>(GlobalLock(hData));
        if (ptr) {
            result = ptr;
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
    return result;
}

bool ClipManager::setText(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();

    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) { CloseClipboard(); return false; }

    wchar_t* ptr = static_cast<wchar_t*>(GlobalLock(hMem));
    if (ptr) {
        memcpy(ptr, text.c_str(), bytes);
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    } else {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
}

bool ClipManager::waitForChange(int timeoutMs) const {
    DWORD before = m_seqBefore;
    int   elapsed = 0;
    while (elapsed < timeoutMs) {
        if (GetClipboardSequenceNumber() != before) return true;
        Sleep(10);
        elapsed += 10;
    }
    return false;
}
