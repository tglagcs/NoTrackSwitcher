#pragma once
#include "Common.h"
#include <vector>

// Sends keyboard events via SendInput.
// All events are tagged with kInjectedId so the hook ignores them.
class InputSender {
public:
    // Send N backspace key presses.
    static void sendBackspaces(int count);

    // Retype a sequence of recorded keys in the target layout.
    // Scancodes are used for hardware-level injection (layout-independent).
    static void retypeKeys(const std::vector<KeyInfo>& keys);

    // Simulate Ctrl+C (copy). Saves and restores Shift state if pressed.
    static void sendCtrlC();

    // Simulate Ctrl+V (paste).
    static void sendCtrlV();

private:
    static void addKeyEvent(std::vector<INPUT>& v, WORD vk, WORD scan,
                             DWORD flags, bool extended);
};
