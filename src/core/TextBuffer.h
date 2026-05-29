#pragma once
#include "Common.h"
#include <deque>
#include <vector>
#include <mutex>

// Ring buffer of typed key events.
// Thread-safe: accessed from both hook thread and worker thread.
class TextBuffer {
public:
    static constexpr int kDefaultMaxSize = 128;

    explicit TextBuffer(int maxSize = kDefaultMaxSize);

    // Add a key-down event. Command-type keys clear the buffer.
    void push(const KeyInfo& key);

    // Clear the buffer entirely.
    void clear();

    // Remove the last N elements from the buffer tail.
    void popBack(int count);

    // Extract the last wordCount words with their keys and deletion count.
    // Trailing spaces are included in both keys and backspaceCount.
    // The word-boundary space that precedes the first word is NOT included
    // in backspaceCount — it stays in the text.
    struct WordExtract {
        std::vector<KeyInfo> keys;     // keys to retype (typing order)
        int backspaceCount = 0;        // backspaces to send before retypeing
    };
    WordExtract extractLastWords(int wordCount) const;

    int  size()  const;
    bool empty() const;
    void setMaxSize(int n);

private:
    mutable std::mutex  m_mutex;
    std::deque<KeyInfo> m_buf;
    int                 m_maxSize;
    bool                m_lastWasFix   = false;
    int                 m_fixWordCount = 0;

public:
    // Used by Worker for multi-word cycling tracking (future feature).
    bool wasLastActionFix() const noexcept { return m_lastWasFix; }
    void setLastActionFix(bool v)  noexcept { m_lastWasFix = v; }
};
