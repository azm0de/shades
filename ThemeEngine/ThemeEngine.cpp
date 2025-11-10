#include <windows.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include "detours.h"
#include "../libs/json/json.hpp" // Include the JSON header

using json = nlohmann::json;

// --- Global Module Handle ---
HMODULE g_hModule = NULL;

// --- Debug Logging Function ---
void LogError(const char* message) {
    FILE* log = fopen("C:\\ThemeEngine_debug.log", "a");
    if (log) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(log, "[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, message);
        fclose(log);
    }
}

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
    HBRUSH h_button_face_brush;
    HBRUSH h_highlight_bg_brush;
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
    LogError("LoadTheme: Starting theme load...");

    // Get the DLL's directory path
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(g_hModule, dllPath, MAX_PATH);

    // Remove DLL filename, keep directory
    wchar_t* lastSlash = wcsrchr(dllPath, L'\\');
    if (lastSlash) *(lastSlash + 1) = L'\0';

    // Append theme.json
    wcscat_s(dllPath, MAX_PATH, L"theme.json");

    // Convert to narrow string for ifstream
    char narrowPath[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, dllPath, -1, narrowPath, MAX_PATH, NULL, NULL);

    char logMsg[512];
    sprintf_s(logMsg, 512, "LoadTheme: Looking for theme.json at: %s", narrowPath);
    LogError(logMsg);

    std::ifstream f(narrowPath);
    if (!f.is_open()) {
        LogError("LoadTheme: FAILED to open theme.json - File not found!");
        return;
    }

    LogError("LoadTheme: theme.json opened successfully, parsing...");

    try {
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
        g_Theme.h_button_face_brush = CreateSolidBrush(g_Theme.button_face);
        g_Theme.h_highlight_bg_brush = CreateSolidBrush(g_Theme.highlight_bg);
        g_ThemeLoaded = true;

        LogError("LoadTheme: Theme loaded and parsed successfully!");
    } catch (const std::exception& e) {
        char errorMsg[512];
        sprintf_s(errorMsg, 512, "LoadTheme: JSON parse error: %s", e.what());
        LogError(errorMsg);
    }
}

// --- Detour Functions ---
DWORD WINAPI DetouredGetSysColor(int nIndex) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueGetSysColor(nIndex);
    CloseHandle(hMutex);

    if (!g_ThemeLoaded) return TrueGetSysColor(nIndex);

    switch (nIndex) {
        // Main window colors
        case COLOR_WINDOW: return g_Theme.window_bg;
        case COLOR_WINDOWTEXT: return g_Theme.window_text;
        case COLOR_HIGHLIGHT: return g_Theme.highlight_bg;
        case COLOR_HIGHLIGHTTEXT: return g_Theme.highlight_text;

        // Button and dialog colors (COLOR_3DFACE == COLOR_BTNFACE, so only use one)
        case COLOR_BTNFACE: // Also covers COLOR_3DFACE (same value)
            return g_Theme.button_face;
        case COLOR_BTNTEXT: return g_Theme.button_text;
        case COLOR_3DSHADOW: return RGB(0x1A, 0x11, 0x45); // Darker purple shadow
        case COLOR_BTNHIGHLIGHT: // Also covers COLOR_3DHIGHLIGHT, COLOR_3DHILIGHT, COLOR_BTNHILIGHT
            return RGB(0x50, 0x35, 0x90); // Lighter purple highlight
        case COLOR_3DDKSHADOW: return RGB(0x00, 0x00, 0x00); // Black
        case COLOR_3DLIGHT: return RGB(0x40, 0x2A, 0x75); // Medium purple

        // Menu colors
        case COLOR_MENU: return g_Theme.window_bg;
        case COLOR_MENUTEXT: return g_Theme.window_text;
        case COLOR_MENUBAR: return g_Theme.header_bg;
        case COLOR_MENUHILIGHT: return g_Theme.highlight_bg;

        // Additional UI element colors
        case COLOR_GRAYTEXT: return RGB(0x60, 0xFF, 0x60); // Dim green for disabled
        case COLOR_INFOBK: return RGB(0x2A, 0x1A, 0x60); // Dark purple tooltip
        case COLOR_INFOTEXT: return g_Theme.window_text;
        case COLOR_HOTLIGHT: return RGB(0x00, 0xFF, 0x00); // Bright green for hyperlinks
        case COLOR_SCROLLBAR: return g_Theme.window_bg;
        case COLOR_APPWORKSPACE: return g_Theme.window_bg;
        case COLOR_GRADIENTACTIVECAPTION: return g_Theme.highlight_bg;
        case COLOR_GRADIENTINACTIVECAPTION: return g_Theme.button_face;
        case COLOR_ACTIVECAPTION: return g_Theme.highlight_bg;
        case COLOR_INACTIVECAPTION: return g_Theme.button_face;
        case COLOR_CAPTIONTEXT: return g_Theme.window_text;
        case COLOR_INACTIVECAPTIONTEXT: return RGB(0x80, 0xFF, 0x80); // Dim green
    }
    return TrueGetSysColor(nIndex);
}

HBRUSH WINAPI DetouredGetSysColorBrush(int nIndex) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueGetSysColorBrush(nIndex);
    CloseHandle(hMutex);

    if (!g_ThemeLoaded) return TrueGetSysColorBrush(nIndex);

    switch (nIndex) {
        case COLOR_WINDOW:
        case COLOR_APPWORKSPACE:
        case COLOR_SCROLLBAR:
        case COLOR_MENU:
        case COLOR_MENUBAR:
            return g_Theme.h_window_bg_brush;
        case COLOR_BTNFACE: // Also COLOR_3DFACE (same value)
            return g_Theme.h_button_face_brush;
        case COLOR_HIGHLIGHT:
            return g_Theme.h_highlight_bg_brush;
    }

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
    LogError("InitializeTheme: Starting initialization...");

    LoadTheme();
    if (!g_ThemeLoaded) {
        LogError("InitializeTheme: Theme failed to load, aborting hook installation");
        return;
    }

    LogError("InitializeTheme: Installing Detours hooks...");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&TrueGetSysColor, (PVOID)DetouredGetSysColor);
    DetourAttach((PVOID*)&TrueGetSysColorBrush, (PVOID)DetouredGetSysColorBrush);
    DetourAttach((PVOID*)&TrueDrawThemeBackground, (PVOID)DetouredDrawThemeBackground);

    LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
        char errorMsg[256];
        sprintf_s(errorMsg, 256, "InitializeTheme: DetourTransactionCommit FAILED with error %ld", error);
        LogError(errorMsg);
        return;
    }

    LogError("InitializeTheme: Detours hooks installed successfully!");

    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        LogError("InitializeTheme: Found Event Viewer window, searching for ListView...");
        g_hListView = FindEventListView(hEventViewer);
        if (g_hListView) {
            g_pOriginalListViewProc = (WNDPROC)SetWindowLongPtr(g_hListView, GWLP_WNDPROC, (LONG_PTR)CustomListViewProc);
            LogError("InitializeTheme: ListView subclassed successfully");
        } else {
            LogError("InitializeTheme: ListView not found");
        }
    } else {
        LogError("InitializeTheme: Event Viewer window not found");
    }

    LogError("InitializeTheme: Initialization complete!");
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
        DeleteObject(g_Theme.h_button_face_brush);
        DeleteObject(g_Theme.h_highlight_bg_brush);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            g_hModule = hModule;
            LogError("DllMain: DLL_PROCESS_ATTACH - Calling InitializeTheme...");
            InitializeTheme();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            LogError("DllMain: DLL_PROCESS_DETACH - Cleaning up...");
            DeinitializeTheme();
            break;
    }
    return TRUE;
}
