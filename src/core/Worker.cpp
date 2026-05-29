#include "Worker.h"

bool Worker::start() {
    m_readyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    m_thread = CreateThread(nullptr, 0, threadProc, this, 0, &m_threadId);
    if (!m_thread) { CloseHandle(m_readyEvent); m_readyEvent = nullptr; return false; }
    SetThreadPriority(m_thread, THREAD_PRIORITY_ABOVE_NORMAL);
    WaitForSingleObject(m_readyEvent, 3000);
    CloseHandle(m_readyEvent);
    m_readyEvent = nullptr;
    return true;
}

void Worker::stop() {
    if (m_threadId)
        PostThreadMessage(m_threadId, WM_WORKER_QUIT, 0, 0);
    if (m_thread) {
        WaitForSingleObject(m_thread, 3000);
        CloseHandle(m_thread);
        m_thread   = nullptr;
        m_threadId = 0;
    }
}

DWORD WINAPI Worker::threadProc(LPVOID param) {
    static_cast<Worker*>(param)->run();
    return 0;
}

void Worker::run() {
    MSG dummy;
    PeekMessage(&dummy, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    if (m_readyEvent) SetEvent(m_readyEvent);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
            case WM_WORKER_FIX_SELECTION: if (enabled) doFixSelection(); break;
            case WM_WORKER_CLEAR_BUFFER:  m_buf.clear();                 break;
            case WM_WORKER_QUIT:          return;
        }
    }
}

void Worker::doFixSelection() {
    static constexpr int kSwitchDelayMs = 30;
    // Get the foreground window's current layout
    HWND hwnd   = GetForegroundWindow();
    HKL  srcHkl = nullptr;
    if (hwnd) {
        DWORD tid = GetWindowThreadProcessId(hwnd, nullptr);
        srcHkl = GetKeyboardLayout(tid);
    }
    HKL dstHkl = m_layouts.nextLayout(srcHkl);
    if (srcHkl == dstHkl) return;

    const LayoutMap* srcMap = m_layouts.findMap(srcHkl);
    const LayoutMap* dstMap = m_layouts.findMap(dstHkl);
    if (!srcMap || !dstMap) return;

    // Record clipboard sequence number before Ctrl+C
    m_clip.backup();

    // Copy selection to clipboard
    InputSender::sendCtrlC();

    // Wait for clipboard update (up to 500 ms)
    if (!m_clip.waitForChange(500)) {
        m_clip.restore();
        return;
    }

    std::wstring text = m_clip.getText();
    if (text.empty()) {
        m_clip.restore();
        return;
    }

    // Character-level conversion using pre-computed layout tables
    std::wstring converted = m_conv.convertText(text, *srcMap, *dstMap);
    m_clip.setText(converted);

    // Paste
    InputSender::sendCtrlV();

    // Switch layout after paste
    Sleep(kSwitchDelayMs);
    m_layouts.switchForegroundLayout(dstHkl);

    // Restore original clipboard (includes its own delay)
    m_clip.restore();

    m_buf.clear();
}
