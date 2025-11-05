#include <windows.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <vector>
#include "detours.h"
#include "../libs/json/json.hpp" // Include the JSON header

using json = nlohmann::json;

// --- Global Theme Variables ---
struct ThemeColors {
    COLORREF window_bg;
    COLORREF window_text;
    COLORREF highlight_bg;
    COLORREF highlight_text;
    COLORREF button_face;
    COLORREF button_text;
    COLORREF header_bg;
    HBRUSH h_window_bg_brush;
    HBRUSH h_header_bg_brush;
};
ThemeColors g_Theme;
bool g_ThemeLoaded = false;

// --- Global Hooking Variables ---
WNDPROC g_pOriginalListViewProc = NULL;
HWND g_hListView = NULL;
const wchar_t* g_pMutexName = L"Global\\EventViewerThemeActive";

// --- Original Function Pointers ---
static DWORD (WINAPI *TrueGetSysColor)(int nIndex) = GetSysColor;
static HBRUSH (WINAPI *TrueGetSysColorBrush)(int nIndex) = GetSysColorBrush;
static HRESULT (WINAPI *TrueDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) = DrawThemeBackground;

// --- Helper Function to Convert Hex String to COLORREF ---
COLORREF HexToCOLORREF(const std::string& hex) {
    int r, g, b;
    sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
    return RGB(r, g, b);
}

// --- Theme Loading ---
void LoadTheme() {
    std::ifstream f("theme.json");
    if (!f.is_open()) return;

    json data = json::parse(f);
    g_Theme.window_bg = HexToCOLORREF(data["colors"]["window_bg"]);
    g_Theme.window_text = HexToCOLORREF(data["colors"]["window_text"]);
    g_Theme.highlight_bg = HexToCOLORREF(data["colors"]["highlight_bg"]);
    g_Theme.highlight_text = HexToCOLORREF(data["colors"]["highlight_text"]);
    g_Theme.button_face = HexToCOLORREF(data["colors"]["button_face"]);
    g_Theme.button_text = HexToCOLORREF(data["colors"]["button_text"]);
    g_Theme.header_bg = HexToCOLORREF(data["colors"]["header_bg"]);

    g_Theme.h_window_bg_brush = CreateSolidBrush(g_Theme.window_bg);
    g_Theme.h_header_bg_brush = CreateSolidBrush(g_Theme.header_bg);
    g_ThemeLoaded = true;
}

// --- Detour Functions ---
DWORD WINAPI DetouredGetSysColor(int nIndex) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueGetSysColor(nIndex);
    CloseHandle(hMutex);

    if (!g_ThemeLoaded) return TrueGetSysColor(nIndex);

    switch (nIndex) {
        case COLOR_WINDOW: return g_Theme.window_bg;
        case COLOR_WINDOWTEXT: return g_Theme.window_text;
        case COLOR_HIGHLIGHT: return g_Theme.highlight_bg;
        case COLOR_HIGHLIGHTTEXT: return g_Theme.highlight_text;
        case COLOR_BTNFACE: return g_Theme.button_face;
        case COLOR_BTNTEXT: return g_Theme.button_text;
    }
    return TrueGetSysColor(nIndex);
}

HBRUSH WINAPI DetouredGetSysColorBrush(int nIndex) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueGetSysColorBrush(nIndex);
    CloseHandle(hMutex);

    if (!g_ThemeLoaded) return TrueGetSysColorBrush(nIndex);

    if (nIndex == COLOR_WINDOW) return g_Theme.h_window_bg_brush;

    return TrueGetSysColorBrush(nIndex);
}

HRESULT WINAPI DetouredDrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
    CloseHandle(hMutex);

    if (g_ThemeLoaded && iPartId == 1) { // HP_HEADERITEM
        FillRect(hdc, pRect, g_Theme.h_header_bg_brush);
        return S_OK;
    }
    return TrueDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

// --- Custom ListView Procedure ---
LRESULT CALLBACK CustomListViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return CallWindowProc(g_pOriginalListViewProc, hWnd, uMsg, wParam, lParam);
    CloseHandle(hMutex);

    if (g_ThemeLoaded && uMsg == WM_NOTIFY) {
        LPNMHDR lpnmh = (LPNMHDR)lParam;
        if (lpnmh->code == NM_CUSTOMDRAW) {
            LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
            switch (lplvcd->nmcd.dwDrawStage) {
                case CDDS_PREPAINT: return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                    if (lplvcd->nmcd.uItemState & CDIS_SELECTED) {
                        lplvcd->clrText = g_Theme.highlight_text;
                        lplvcd->clrTextBk = g_Theme.highlight_bg;
                    } else {
                        lplvcd->clrText = g_Theme.window_text;
                        lplvcd->clrTextBk = g_Theme.window_bg;
                    }
                    return CDRF_DODEFAULT;
            }
        }
    }
    return CallWindowProc(g_pOriginalListViewProc, hWnd, uMsg, wParam, lParam);
}

// --- Helper function to find the ListView control ---
HWND FindEventListView(HWND hParent) {
    HWND hChild = FindWindowEx(hParent, NULL, NULL, NULL);
    while (hChild != NULL) {
        wchar_t className[256];
        GetClassNameW(hChild, className, 256);
        if (wcscmp(className, L"SysListView32") == 0) {
            return hChild;
        }
        HWND hGrandChild = FindEventListView(hChild);
        if (hGrandChild != NULL) {
            return hGrandChild;
        }
        hChild = FindWindowEx(hParent, hChild, NULL, NULL);
    }
    return NULL;
}

// --- Initialization and Deinitialization ---
void InitializeTheme() {
    LoadTheme();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&TrueGetSysColor, (PVOID)DetouredGetSysColor);
    DetourAttach((PVOID*)&TrueGetSysColorBrush, (PVOID)DetouredGetSysColorBrush);
    DetourAttach((PVOID*)&TrueDrawThemeBackground, (PVOID)DetouredDrawThemeBackground);
    DetourTransactionCommit();

    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        g_hListView = FindEventListView(hEventViewer);
        if (g_hListView) {
            g_pOriginalListViewProc = (WNDPROC)SetWindowLongPtr(g_hListView, GWLP_WNDPROC, (LONG_PTR)CustomListViewProc);
        }
    }
}

void DeinitializeTheme() {
    if (g_pOriginalListViewProc && g_hListView) {
        SetWindowLongPtr(g_hListView, GWLP_WNDPROC, (LONG_PTR)g_pOriginalListViewProc);
    }
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((PVOID*)&TrueGetSysColor, (PVOID)DetouredGetSysColor);
    DetourDetach((PVOID*)&TrueGetSysColorBrush, (PVOID)DetouredGetSysColorBrush);
    DetourDetach((PVOID*)&TrueDrawThemeBackground, (PVOID)DetouredDrawThemeBackground);
    DetourTransactionCommit();

    if (g_ThemeLoaded) {
        DeleteObject(g_Theme.h_window_bg_brush);
        DeleteObject(g_Theme.h_header_bg_brush);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitializeTheme, NULL, 0, NULL);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            DeinitializeTheme();
            break;
    }
    return TRUE;
}
