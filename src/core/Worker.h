#pragma once
#include "Common.h"
#include "TextBuffer.h"
#include "LayoutManager.h"
#include "Converter.h"
#include "InputSender.h"
#include "ClipManager.h"

// Worker thread that performs text conversion and input injection.
// The hook callback is kept minimal — it posts WM_WORKER_* messages here.
class Worker {
public:
    Worker(TextBuffer& buf, LayoutManager& layouts)
        : m_buf(buf), m_layouts(layouts) {}

    ~Worker() { stop(); }

    // Start the worker thread. Returns when the thread's message queue is ready.
    bool start();
    // Signal the worker to exit and wait for it.
    void stop();

    DWORD threadId() const noexcept { return m_threadId; }

    bool enabled = true;

private:
    static DWORD WINAPI threadProc(LPVOID param);
    void run();
    void doFixSelection();

    TextBuffer&   m_buf;
    LayoutManager& m_layouts;
    Converter     m_conv;
    ClipManager   m_clip;

    HANDLE m_thread     = nullptr;
    DWORD  m_threadId   = 0;
    HANDLE m_readyEvent = nullptr;
};
