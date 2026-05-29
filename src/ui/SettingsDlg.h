#pragma once
#include "Common.h"
#include "../config/Config.h"

class SettingsDlg {
public:
    explicit SettingsDlg(AppConfig& cfg) : m_cfg(cfg) {}

    bool show(HINSTANCE hInst, HWND hwndParent);

    // Returns the HWND of the currently open dialog, or nullptr.
    static HWND currentHwnd();

private:
    static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR handleMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    void onInit(HWND hDlg);
    void onApply(HWND hDlg);

    AppConfig& m_cfg;
    AppConfig  m_edit;
    HWND       m_hDlg = nullptr;
    HBRUSH     m_bgBrush   = nullptr;
    HBRUSH     m_editBrush = nullptr;
};
