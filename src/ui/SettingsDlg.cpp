#include "SettingsDlg.h"
#include "../resources/resource.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>

#ifdef _MSC_VER
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#endif

static SettingsDlg* s_dlg = nullptr;

static constexpr COLORREF kBg   = RGB(30,  30,  30);
static constexpr COLORREF kEdit = RGB(45,  45,  45);
static constexpr COLORREF kText = RGB(220, 220, 220);

static WORD hkToControl(const Hotkey& hk) {
    if (!hk.valid()) return 0;
    WORD mods = 0;
    if (hk.ctrl)  mods |= HOTKEYF_CONTROL;
    if (hk.alt)   mods |= HOTKEYF_ALT;
    if (hk.shift) mods |= HOTKEYF_SHIFT;
    if (hk.win)   mods |= HOTKEYF_EXT;
    return MAKEWORD(hk.vk, mods);
}

static Hotkey hkFromControl(WORD val) {
    Hotkey hk;
    hk.vk    = LOBYTE(val);
    BYTE mods = HIBYTE(val);
    hk.ctrl   = (mods & HOTKEYF_CONTROL) != 0;
    hk.alt    = (mods & HOTKEYF_ALT)     != 0;
    hk.shift  = (mods & HOTKEYF_SHIFT)   != 0;
    hk.win    = (mods & HOTKEYF_EXT)     != 0;
    return hk;
}

HWND SettingsDlg::currentHwnd() {
    return s_dlg ? s_dlg->m_hDlg : nullptr;
}

bool SettingsDlg::show(HINSTANCE hInst, HWND hwndParent) {
    m_edit = m_cfg;
    s_dlg  = this;

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_HOTKEY_CLASS;
    InitCommonControlsEx(&icc);

    INT_PTR ret = DialogBoxParamW(hInst, MAKEINTRESOURCEW(IDD_SETTINGS),
                                   hwndParent, dlgProc, 0);
    if (m_bgBrush)   { DeleteObject(m_bgBrush);   m_bgBrush   = nullptr; }
    if (m_editBrush) { DeleteObject(m_editBrush); m_editBrush = nullptr; }
    s_dlg = nullptr;
    return ret == IDOK;
}

INT_PTR CALLBACK SettingsDlg::dlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    return s_dlg ? s_dlg->handleMsg(hDlg, msg, wParam, lParam) : FALSE;
}

INT_PTR SettingsDlg::handleMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            m_hDlg = hDlg;
            onInit(hDlg);
            return TRUE;

        case WM_CTLCOLORDLG:
            return reinterpret_cast<INT_PTR>(m_bgBrush);

        case WM_CTLCOLORSTATIC: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, kText);
            SetBkColor(hdc, kBg);
            return reinterpret_cast<INT_PTR>(m_bgBrush);
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, kText);
            SetBkColor(hdc, kEdit);
            return reinterpret_cast<INT_PTR>(m_editBrush);
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    onApply(hDlg);
                    m_cfg = m_edit;
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            return FALSE;

        case WM_DESTROY:
            m_hDlg = nullptr;
            return FALSE;
    }
    return FALSE;
}

void SettingsDlg::onInit(HWND hDlg) {
    HICON hIco = static_cast<HICON>(
        LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_TRAY),
                   IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    if (hIco) {
        SendMessage(hDlg, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(hIco));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIco));
    }

    m_bgBrush   = CreateSolidBrush(kBg);
    m_editBrush = CreateSolidBrush(kEdit);

    BOOL dark = TRUE;
    DwmSetWindowAttribute(hDlg, 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &dark, sizeof(dark));

    SetWindowTheme(GetDlgItem(hDlg, IDC_AUTOSTART),  L"DarkMode_Explorer", nullptr);
    SetWindowTheme(GetDlgItem(hDlg, IDC_HK_FIX_SEL), L"DarkMode_Explorer", nullptr);
    SetWindowTheme(GetDlgItem(hDlg, IDOK),            L"DarkMode_Explorer", nullptr);
    SetWindowTheme(GetDlgItem(hDlg, IDCANCEL),        L"DarkMode_Explorer", nullptr);

    CheckDlgButton(hDlg, IDC_AUTOSTART,
                   m_edit.autoStart ? BST_CHECKED : BST_UNCHECKED);
    SendDlgItemMessageW(hDlg, IDC_HK_FIX_SEL, HKM_SETHOTKEY,
                        hkToControl(m_edit.hkFixSelection), 0);
}

void SettingsDlg::onApply(HWND hDlg) {
    m_edit.autoStart = IsDlgButtonChecked(hDlg, IDC_AUTOSTART) == BST_CHECKED;
    m_edit.hkFixSelection = hkFromControl(
        static_cast<WORD>(SendDlgItemMessage(hDlg, IDC_HK_FIX_SEL, HKM_GETHOTKEY, 0, 0)));
}
