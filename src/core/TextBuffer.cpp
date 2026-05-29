#include "TextBuffer.h"
#include <algorithm>

TextBuffer::TextBuffer(int maxSize) : m_maxSize(maxSize) {}

void TextBuffer::push(const KeyInfo& key) {
    std::lock_guard lock(m_mutex);

    if (key.type == KeyType::Command) {
        m_buf.clear();
        m_lastWasFix   = false;
        m_fixWordCount = 0;
        return;
    }

    m_buf.push_back(key);
    if (static_cast<int>(m_buf.size()) > m_maxSize)
        m_buf.pop_front();

    // Any new printable input resets the fix-cycle counter
    if (key.type == KeyType::Letter || key.type == KeyType::Custom) {
        m_lastWasFix   = false;
        m_fixWordCount = 0;
    }
}

void TextBuffer::clear() {
    std::lock_guard lock(m_mutex);
    m_buf.clear();
    m_lastWasFix   = false;
    m_fixWordCount = 0;
}

void TextBuffer::popBack(int count) {
    std::lock_guard lock(m_mutex);
    count = std::min(count, static_cast<int>(m_buf.size()));
    while (count-- > 0) m_buf.pop_back();
}

// Walk backward collecting up to wordCount words.
//
// Rules:
//  - Trailing spaces (after the last letter) are deleted AND retyped.
//  - The first word-boundary space (between words N and N+1 from the end)
//    is NOT deleted — it stays in the text. Only the letters are deleted.
//  - For wordCount > 1, the inter-word spaces BETWEEN the words being fixed
//    ARE deleted and retyped.
//
// Example (wordCount=1): buffer = [h,e,l,l,o, ,w,o,r,l,d]
//   → keys=[w,o,r,l,d], backspaceCount=5   (space before "world" stays)
//
// Example (wordCount=1): buffer = [w,o,r,l,d, , ]
//   → keys=[' ',' ','w','o','r','l','d'], backspaceCount=7 (trailing spaces included)
TextBuffer::WordExtract TextBuffer::extractLastWords(int wordCount) const {
    std::lock_guard lock(m_mutex);
    WordExtract result;
    if (m_buf.empty() || wordCount <= 0) return result;

    std::vector<KeyInfo> collected;
    int idx = static_cast<int>(m_buf.size()) - 1;

    // Phase 1: collect trailing space keys (delete+retype them)
    while (idx >= 0 && m_buf[idx].type == KeyType::Space) {
        collected.push_back(m_buf[idx]);
        result.backspaceCount++;
        idx--;
    }

    // Phase 2: collect wordCount words
    int wordsComplete = 0;
    while (idx >= 0 && wordsComplete < wordCount) {
        const auto& k = m_buf[idx];

        if (k.type == KeyType::Space) {
            // This space marks the end of a word we just finished collecting
            wordsComplete++;
            if (wordsComplete < wordCount) {
                // Space between two words being fixed — delete and retype
                collected.push_back(k);
                result.backspaceCount++;
            }
            // else: outer boundary — do NOT include; it stays in the text
            idx--;
        } else {
            collected.push_back(k);
            result.backspaceCount++;
            idx--;
        }
    }

    // Keys were gathered right-to-left; restore typing order
    std::reverse(collected.begin(), collected.end());
    result.keys = std::move(collected);
    return result;
}

int TextBuffer::size() const {
    std::lock_guard lock(m_mutex);
    return static_cast<int>(m_buf.size());
}

bool TextBuffer::empty() const {
    std::lock_guard lock(m_mutex);
    return m_buf.empty();
}

void TextBuffer::setMaxSize(int n) {
    std::lock_guard lock(m_mutex);
    m_maxSize = n;
    while (static_cast<int>(m_buf.size()) > m_maxSize)
        m_buf.pop_front();
}
