#include "InputSender.h"

void InputSender::addKeyEvent(std::vector<INPUT>& v, WORD vk, WORD scan,
                               DWORD flags, bool extended) {
    INPUT in    = {};
    in.type     = INPUT_KEYBOARD;
    in.ki.wVk   = vk;
    in.ki.wScan = scan;
    in.ki.dwFlags       = flags;
    in.ki.dwExtraInfo   = kInjectedId;
    if (extended) in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    v.push_back(in);
}

void InputSender::sendBackspaces(int count) {
    if (count <= 0) return;
    std::vector<INPUT> inputs;
    inputs.reserve(count * 2);
    for (int i = 0; i < count; ++i) {
        addKeyEvent(inputs, VK_BACK, 0x0E, 0, false);
        addKeyEvent(inputs, VK_BACK, 0x0E, KEYEVENTF_KEYUP, false);
    }
    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void InputSender::retypeKeys(const std::vector<KeyInfo>& keys) {
    if (keys.empty()) return;
    std::vector<INPUT> inputs;
    inputs.reserve(keys.size() * 4);

    for (const auto& k : keys) {
        if (k.type == KeyType::Space) {
            WORD sc = k.scan ? k.scan : static_cast<WORD>(MapVirtualKey(k.vk, MAPVK_VK_TO_VSC));
            addKeyEvent(inputs, 0, sc, KEYEVENTF_SCANCODE, k.extended);
            addKeyEvent(inputs, 0, sc, KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, k.extended);
            continue;
        }

        if (k.shift) addKeyEvent(inputs, VK_SHIFT, 0x2A, 0, false);

        WORD sc = k.scan ? k.scan : static_cast<WORD>(MapVirtualKey(k.vk, MAPVK_VK_TO_VSC));
        if (sc) {
            addKeyEvent(inputs, 0, sc, KEYEVENTF_SCANCODE, k.extended);
            addKeyEvent(inputs, 0, sc, KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, k.extended);
        } else {
            // Fallback to VK if no scancode available
            addKeyEvent(inputs, k.vk, 0, 0, k.extended);
            addKeyEvent(inputs, k.vk, 0, KEYEVENTF_KEYUP, k.extended);
        }

        if (k.shift) addKeyEvent(inputs, VK_SHIFT, 0x2A, KEYEVENTF_KEYUP, false);
    }

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void InputSender::sendCtrlC() {
    bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    std::vector<INPUT> inputs;
    inputs.reserve(8);

    if (shiftDown) addKeyEvent(inputs, VK_SHIFT, 0x2A, KEYEVENTF_KEYUP, false);
    addKeyEvent(inputs, VK_CONTROL, 0x1D, 0, false);
    addKeyEvent(inputs, 'C', 0x2E, 0, false);
    addKeyEvent(inputs, 'C', 0x2E, KEYEVENTF_KEYUP, false);
    addKeyEvent(inputs, VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, false);
    if (shiftDown) addKeyEvent(inputs, VK_SHIFT, 0x2A, 0, false);

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void InputSender::sendCtrlV() {
    std::vector<INPUT> inputs;
    inputs.reserve(4);
    addKeyEvent(inputs, VK_CONTROL, 0x1D, 0, false);
    addKeyEvent(inputs, 'V', 0x2F, 0, false);
    addKeyEvent(inputs, 'V', 0x2F, KEYEVENTF_KEYUP, false);
    addKeyEvent(inputs, VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, false);
    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}
