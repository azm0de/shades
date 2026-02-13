#include <windows.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <richedit.h>
#include <prsht.h>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstdio>
#include "detours.h"
#include "../libs/json/json.hpp" // Include the JSON header

using json = nlohmann::json;

// --- Forward Declarations ---
void ThemeRichEditControl(HWND hRichEdit);
void ThemeEditControl(HWND hEdit);
void ThemeStaticControl(HWND hStatic);
BOOL CALLBACK EnumChildProcForAllControls(HWND hChild, LPARAM lParam);

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
WNDPROC g_pOriginalTreeViewProc = NULL;
HWND g_hTreeView = NULL;
WNDPROC g_pOriginalTabControlProc = NULL;
HWND g_hTabControl = NULL;
HHOOK g_hCallWndProcHook = NULL;
UINT_PTR g_ThemeTimerId = 0;
std::set<HWND> g_ThemedControls; // Track which controls we've already themed
const wchar_t* g_pMutexName = L"Global\\EventViewerThemeActive";

// --- Parent Window Subclassing for ListView NM_CUSTOMDRAW ---
struct SubclassedParent {
    HWND hwnd;
    WNDPROC originalProc;
};
std::vector<SubclassedParent> g_SubclassedParents;

// --- Original Function Pointers ---
static DWORD (WINAPI *TrueGetSysColor)(int nIndex) = GetSysColor;
static HBRUSH (WINAPI *TrueGetSysColorBrush)(int nIndex) = GetSysColorBrush;
static HRESULT (WINAPI *TrueDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) = DrawThemeBackground;
static HWND (WINAPI *TrueCreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = CreateWindowExW;
static int (WINAPI *TrueDrawTextW)(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format) = DrawTextW;
static int (WINAPI *TrueDrawTextExW)(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp) = DrawTextExW;
static BOOL (WINAPI *TrueSetWindowTextW)(HWND hWnd, LPCWSTR lpString) = SetWindowTextW;

// --- Helper Function to Convert Hex String to COLORREF ---
COLORREF HexToCOLORREF(const std::string& hex) {
    int r, g, b;
    sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
    return RGB(r, g, b);
}

// --- File Timestamp Helper ---
FILETIME GetLastWriteTime(const wchar_t* path) {
    FILETIME ftCreate, ftAccess, ftWrite = { 0, 0 };
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);
        CloseHandle(hFile);
    }
    return ftWrite;
}

FILETIME g_lastThemeWriteTime = { 0, 0 };
wchar_t g_themeFilePath[MAX_PATH] = { 0 };

// --- Theme Loading ---
void LoadTheme() {
    LogError("LoadTheme: Starting theme load...");

    // Get the theme file path if not already set
    if (g_themeFilePath[0] == 0) {
        // Check for custom path via environment variable (set by --config flag)
        char customPath[MAX_PATH] = { 0 };
        DWORD envLen = GetEnvironmentVariableA("SHADES_THEME_PATH", customPath, MAX_PATH);
        if (envLen > 0 && envLen < MAX_PATH) {
            MultiByteToWideChar(CP_UTF8, 0, customPath, -1, g_themeFilePath, MAX_PATH);
            LogError("LoadTheme: Using custom theme path from SHADES_THEME_PATH");
        } else {
            // Default: theme.json next to the DLL
            GetModuleFileNameW(g_hModule, g_themeFilePath, MAX_PATH);
            wchar_t* lastSlash = wcsrchr(g_themeFilePath, L'\\');
            if (lastSlash) *(lastSlash + 1) = L'\0';
            wcscat_s(g_themeFilePath, MAX_PATH, L"theme.json");
        }
    }

    // Update timestamp
    g_lastThemeWriteTime = GetLastWriteTime(g_themeFilePath);

    // Convert to narrow string for ifstream
    char narrowPath[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, g_themeFilePath, -1, narrowPath, MAX_PATH, NULL, NULL);

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
        
        // Store old brushes to delete later
        HBRUSH old_window_bg = g_Theme.h_window_bg_brush;
        HBRUSH old_header_bg = g_Theme.h_header_bg_brush;
        HBRUSH old_button_face = g_Theme.h_button_face_brush;
        HBRUSH old_highlight_bg = g_Theme.h_highlight_bg_brush;

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
        
        // Delete old brushes if they existed
        if (g_ThemeLoaded) {
            DeleteObject(old_window_bg);
            DeleteObject(old_header_bg);
            DeleteObject(old_button_face);
            DeleteObject(old_highlight_bg);
        }

        g_ThemeLoaded = true;

        LogError("LoadTheme: Theme loaded and parsed successfully!");
    } catch (const std::exception& e) {
        char errorMsg[512];
        sprintf_s(errorMsg, 512, "LoadTheme: JSON parse error: %s", e.what());
        LogError(errorMsg);
    }
}


// --- Toggle transition detection (called from Detours hooks) ---
// The timer and WH_CALLWNDPROC hook are created on the injection remote thread,
// which exits after LoadLibraryA returns - so they never fire. Instead, we detect
// enable/disable transitions in the Detours hooks which are process-wide.
static bool g_lastKnownMutexState = true;
static bool g_inMutexTransition = false;

void CheckMutexTransition(bool currentMutexState) {
    if (g_inMutexTransition) return; // Prevent reentrancy
    if (currentMutexState == g_lastKnownMutexState) return;

    g_inMutexTransition = true;
    g_ThemedControls.clear();

    if (currentMutexState && g_ThemeLoaded) {
        // Theme just RE-ENABLED - re-apply direct colors to known controls
        LogError("CheckMutexTransition: Theme RE-ENABLED - re-applying colors");
        if (g_hListView && IsWindow(g_hListView)) {
            PostMessage(g_hListView, LVM_SETBKCOLOR, 0, (LPARAM)g_Theme.window_bg);
            PostMessage(g_hListView, LVM_SETTEXTBKCOLOR, 0, (LPARAM)g_Theme.window_bg);
            PostMessage(g_hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)g_Theme.window_text);
        }
        if (g_hTreeView && IsWindow(g_hTreeView)) {
            PostMessage(g_hTreeView, TVM_SETBKCOLOR, 0, (LPARAM)g_Theme.window_bg);
            PostMessage(g_hTreeView, TVM_SETTEXTCOLOR, 0, (LPARAM)g_Theme.window_text);
        }
    } else {
        // Theme just DISABLED - reset direct colors to system defaults
        LogError("CheckMutexTransition: Theme DISABLED - resetting colors");
        if (g_hListView && IsWindow(g_hListView)) {
            PostMessage(g_hListView, LVM_SETBKCOLOR, 0, (LPARAM)CLR_DEFAULT);
            PostMessage(g_hListView, LVM_SETTEXTBKCOLOR, 0, (LPARAM)CLR_DEFAULT);
            PostMessage(g_hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)CLR_DEFAULT);
        }
        if (g_hTreeView && IsWindow(g_hTreeView)) {
            PostMessage(g_hTreeView, TVM_SETBKCOLOR, 0, (LPARAM)(COLORREF)-1);
            PostMessage(g_hTreeView, TVM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)-1);
        }
    }

    g_lastKnownMutexState = currentMutexState;
    g_inMutexTransition = false;
}

// --- Timer callback for periodic re-theming (Phase 2.1) ---
// NOTE: This timer is created on the injection remote thread, which exits after
// LoadLibraryA returns. The timer is destroyed when its thread exits, so this
// callback effectively never fires. It's kept for correctness in case the
// architecture changes to use a persistent thread. Toggle detection is handled
// by CheckMutexTransition() in the Detours hooks instead.
void CALLBACK ThemeTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (!g_ThemeLoaded) return;

    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    bool mutexPresent = (hMutex != NULL);
    if (hMutex) CloseHandle(hMutex);

    if (!mutexPresent) return;

    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");

    // Check for theme file updates (hot reload)
    if (g_themeFilePath[0] != 0) {
        FILETIME currentWriteTime = GetLastWriteTime(g_themeFilePath);
        if (CompareFileTime(&currentWriteTime, &g_lastThemeWriteTime) != 0) {
            LogError("ThemeTimerProc: Theme file changed, reloading...");
            LoadTheme();
            g_ThemedControls.clear();

            if (g_hListView && IsWindow(g_hListView)) {
                ListView_SetBkColor(g_hListView, g_Theme.window_bg);
                ListView_SetTextBkColor(g_hListView, g_Theme.window_bg);
                ListView_SetTextColor(g_hListView, g_Theme.window_text);
            }
            if (g_hTreeView && IsWindow(g_hTreeView)) {
                TreeView_SetBkColor(g_hTreeView, g_Theme.window_bg);
                TreeView_SetTextColor(g_hTreeView, g_Theme.window_text);
            }

            if (hEventViewer) {
                InvalidateRect(hEventViewer, NULL, TRUE);
                RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
            }
        }
    }

    if (hEventViewer) {
        EnumChildWindows(hEventViewer, EnumChildProcForAllControls, 0);
    }
}

// --- Detour Functions ---
DWORD WINAPI DetouredGetSysColor(int nIndex) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    bool mutexPresent = (hMutex != NULL);
    if (hMutex) CloseHandle(hMutex);

    // Detect enable/disable transitions for toggle support
    CheckMutexTransition(mutexPresent);

    if (!mutexPresent) return TrueGetSysColor(nIndex);
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

// --- Phase 3.1: DrawText hooks for text color ---
int WINAPI DetouredDrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueDrawTextW(hdc, lpchText, cchText, lprc, format);
    CloseHandle(hMutex);

    if (g_ThemeLoaded) {
        SetTextColor(hdc, g_Theme.window_text);
        SetBkColor(hdc, g_Theme.button_face);
    }
    return TrueDrawTextW(hdc, lpchText, cchText, lprc, format);
}

int WINAPI DetouredDrawTextExW(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return TrueDrawTextExW(hdc, lpchText, cchText, lprc, format, lpdtp);
    CloseHandle(hMutex);

    if (g_ThemeLoaded) {
        SetTextColor(hdc, g_Theme.window_text);
        SetBkColor(hdc, g_Theme.button_face);
    }
    return TrueDrawTextExW(hdc, lpchText, cchText, lprc, format, lpdtp);
}

// --- Phase 4.1: CreateWindowExW hook for instant theming ---
HWND WINAPI DetouredCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
                                     DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                     HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    HWND hWnd = TrueCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight,
                                      hWndParent, hMenu, hInstance, lpParam);

    if (hWnd && g_ThemeLoaded) {
        HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
        if (hMutex != NULL) {
            CloseHandle(hMutex);

            wchar_t className[256];
            GetClassNameW(hWnd, className, 256);

            // Theme RichEdit controls immediately
            if (wcscmp(className, L"RichEdit20W") == 0 ||
                wcscmp(className, L"RICHEDIT50W") == 0 ||
                wcscmp(className, L"RichEdit") == 0) {
                ThemeRichEditControl(hWnd);
                g_ThemedControls.insert(hWnd);

                char logMsg[256];
                sprintf_s(logMsg, 256, "DetouredCreateWindowExW: Themed RichEdit at creation, HWND=%p", hWnd);
                LogError(logMsg);
            }
            // Theme Edit controls immediately
            else if (wcscmp(className, L"Edit") == 0) {
                ThemeEditControl(hWnd);
            }
            // Theme Static controls immediately
            else if (wcscmp(className, L"Static") == 0) {
                ThemeStaticControl(hWnd);
            }
        }
    }

    return hWnd;
}

// --- Phase 4.2: SetWindowTextW hook to maintain theming ---
BOOL WINAPI DetouredSetWindowTextW(HWND hWnd, LPCWSTR lpString) {
    BOOL result = TrueSetWindowTextW(hWnd, lpString);

    if (result && g_ThemeLoaded) {
        HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
        if (hMutex != NULL) {
            CloseHandle(hMutex);

            // Re-apply theme to this control after text change
            InvalidateRect(hWnd, NULL, TRUE);

            wchar_t className[256];
            GetClassNameW(hWnd, className, 256);

            if (wcscmp(className, L"RichEdit20W") == 0 ||
                wcscmp(className, L"RICHEDIT50W") == 0) {
                ThemeRichEditControl(hWnd);
            }
        }
    }

    return result;
}

// --- Custom ListView Procedure ---
// NM_CUSTOMDRAW is handled by the parent window (CustomParentProc), not here.
// This proc handles direct messages to the ListView: WM_ERASEBKGND, colors, etc.
LRESULT CALLBACK CustomListViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return CallWindowProc(g_pOriginalListViewProc, hWnd, uMsg, wParam, lParam);
    CloseHandle(hMutex);

    // Handle background erasing for ListView
    if (g_ThemeLoaded && uMsg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, g_Theme.h_window_bg_brush);
        return 1;
    }

    return CallWindowProc(g_pOriginalListViewProc, hWnd, uMsg, wParam, lParam);
}

// --- Custom TreeView Procedure ---
LRESULT CALLBACK CustomTreeViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return CallWindowProc(g_pOriginalTreeViewProc, hWnd, uMsg, wParam, lParam);
    CloseHandle(hMutex);

    if (g_ThemeLoaded && uMsg == WM_NOTIFY) {
        LPNMHDR lpnmh = (LPNMHDR)lParam;
        if (lpnmh->code == NM_CUSTOMDRAW) {
            LPNMTVCUSTOMDRAW lptvcd = (LPNMTVCUSTOMDRAW)lParam;
            switch (lptvcd->nmcd.dwDrawStage) {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;

                case CDDS_ITEMPREPAINT:
                    if (lptvcd->nmcd.uItemState & CDIS_SELECTED) {
                        lptvcd->clrText = g_Theme.highlight_text;
                        lptvcd->clrTextBk = g_Theme.highlight_bg;
                    } else {
                        lptvcd->clrText = g_Theme.window_text;
                        lptvcd->clrTextBk = g_Theme.window_bg;
                    }
                    return CDRF_NEWFONT;
            }
        }
    }

    if (g_ThemeLoaded && uMsg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, g_Theme.h_window_bg_brush);
        return 1;
    }

    return CallWindowProc(g_pOriginalTreeViewProc, hWnd, uMsg, wParam, lParam);
}

// --- Helper to theme RichEdit controls ---
void ThemeRichEditControl(HWND hRichEdit) {
    if (!g_ThemeLoaded) return;

    SendMessage(hRichEdit, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)g_Theme.window_bg);

    CHARFORMAT2 cf;
    ZeroMemory(&cf, sizeof(CHARFORMAT2));
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = g_Theme.window_text;
    cf.dwEffects = 0;

    SendMessage(hRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);

    char logMsg[256];
    sprintf_s(logMsg, 256, "ThemeRichEditControl: Applied theme to RichEdit HWND=%p", hRichEdit);
    LogError(logMsg);
}

// --- Callback to find and theme RichEdit controls ---
BOOL CALLBACK EnumChildProcForRichEdit(HWND hChild, LPARAM lParam) {
    wchar_t className[256];
    GetClassNameW(hChild, className, 256);

    if (wcscmp(className, L"RichEdit20W") == 0 ||
        wcscmp(className, L"RICHEDIT50W") == 0 ||
        wcscmp(className, L"RichEdit") == 0) {
        ThemeRichEditControl(hChild);
    }

    EnumChildWindows(hChild, EnumChildProcForRichEdit, lParam);

    return TRUE;
}

// --- Helper to theme Edit controls ---
void ThemeEditControl(HWND hEdit) {
    if (!g_ThemeLoaded) return;
    if (g_ThemedControls.find(hEdit) != g_ThemedControls.end()) return; // Already themed

    InvalidateRect(hEdit, NULL, TRUE);
    g_ThemedControls.insert(hEdit);

    char logMsg[256];
    sprintf_s(logMsg, 256, "ThemeEditControl: Applied theme to Edit HWND=%p", hEdit);
    LogError(logMsg);
}

// --- Helper to theme Static controls ---
void ThemeStaticControl(HWND hStatic) {
    if (!g_ThemeLoaded) return;
    if (g_ThemedControls.find(hStatic) != g_ThemedControls.end()) return; // Already themed

    InvalidateRect(hStatic, NULL, TRUE);
    g_ThemedControls.insert(hStatic);

    char logMsg[256];
    sprintf_s(logMsg, 256, "ThemeStaticControl: Applied theme to Static HWND=%p", hStatic);
    LogError(logMsg);
}

// --- Callback to find and theme ALL control types ---
BOOL CALLBACK EnumChildProcForAllControls(HWND hChild, LPARAM lParam) {
    if (!g_ThemeLoaded) return TRUE;

    wchar_t className[256];
    GetClassNameW(hChild, className, 256);

    // Theme RichEdit
    if (wcscmp(className, L"RichEdit20W") == 0 ||
        wcscmp(className, L"RICHEDIT50W") == 0 ||
        wcscmp(className, L"RichEdit") == 0) {
        if (g_ThemedControls.find(hChild) == g_ThemedControls.end()) {
            ThemeRichEditControl(hChild);
            g_ThemedControls.insert(hChild);
        }
    }

    // Theme ListView controls
    else if (wcscmp(className, L"SysListView32") == 0) {
        if (g_ThemedControls.find(hChild) == g_ThemedControls.end()) {
            // Set ListView background and text colors
            SendMessageW(hChild, LVM_SETBKCOLOR, 0, (LPARAM)g_Theme.window_bg);
            SendMessageW(hChild, LVM_SETTEXTBKCOLOR, 0, (LPARAM)g_Theme.window_bg);
            SendMessageW(hChild, LVM_SETTEXTCOLOR, 0, (LPARAM)g_Theme.window_text);

            // Force redraw
            InvalidateRect(hChild, NULL, TRUE);

            g_ThemedControls.insert(hChild);

            char logMsg[256];
            sprintf_s(logMsg, 256, "EnumChildProcForAllControls: Themed ListView HWND=%p", hChild);
            LogError(logMsg);
        }
    }

    // Theme Edit controls
    else if (wcscmp(className, L"Edit") == 0) {
        ThemeEditControl(hChild);
    }

    // Theme Static controls
    else if (wcscmp(className, L"Static") == 0) {
        ThemeStaticControl(hChild);
    }

    // Recurse into children
    EnumChildWindows(hChild, EnumChildProcForAllControls, lParam);

    return TRUE;
}

// --- Custom Tab Control Procedure (Phase 1.2) ---
LRESULT CALLBACK CustomTabControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) return CallWindowProc(g_pOriginalTabControlProc, hWnd, uMsg, wParam, lParam);
    CloseHandle(hMutex);

    if (g_ThemeLoaded) {
        if (uMsg == WM_NOTIFY) {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->code == NM_CUSTOMDRAW) {
                LPNMCUSTOMDRAW lpnmcd = (LPNMCUSTOMDRAW)lParam;
                switch (lpnmcd->dwDrawStage) {
                    case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW;

                    case CDDS_ITEMPREPAINT:
                    {
                        HDC hdc = lpnmcd->hdc;
                        RECT rc = lpnmcd->rc;

                        // Draw tab background
                        if (lpnmcd->uItemState & CDIS_SELECTED) {
                            FillRect(hdc, &rc, g_Theme.h_highlight_bg_brush);
                            SetTextColor(hdc, g_Theme.highlight_text);
                        } else {
                            FillRect(hdc, &rc, g_Theme.h_button_face_brush);
                            SetTextColor(hdc, g_Theme.button_text);
                        }
                        SetBkMode(hdc, TRANSPARENT);

                        return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
                    }
                }
            }
        }

        if (uMsg == WM_ERASEBKGND) {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, g_Theme.h_button_face_brush);
            return 1;
        }
    }

    return CallWindowProc(g_pOriginalTabControlProc, hWnd, uMsg, wParam, lParam);
}

// --- Custom Parent Window Procedure for ListView NM_CUSTOMDRAW ---
LRESULT CALLBACK CustomParentProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, g_pMutexName);
    if (hMutex == NULL) {
        // Find and call original proc
        for (const auto& parent : g_SubclassedParents) {
            if (parent.hwnd == hWnd) {
                return CallWindowProc(parent.originalProc, hWnd, uMsg, wParam, lParam);
            }
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    CloseHandle(hMutex);

    if (g_ThemeLoaded && uMsg == WM_NOTIFY) {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->code == NM_CUSTOMDRAW) {
            wchar_t className[256];
            GetClassNameW(pnmh->hwndFrom, className, 256);

            // Handle ListView custom draw via parent window
            // NM_CUSTOMDRAW is sent to the parent, not the ListView itself
            if (wcscmp(className, L"SysListView32") == 0) {
                LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

                switch (lplvcd->nmcd.dwDrawStage) {
                    case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;

                    case CDDS_ITEMPREPAINT:
                    {
                        HDC hdc = lplvcd->nmcd.hdc;
                        RECT rc = lplvcd->nmcd.rc;

                        if (lplvcd->nmcd.uItemState & CDIS_SELECTED) {
                            FillRect(hdc, &rc, g_Theme.h_highlight_bg_brush);
                            lplvcd->clrText = g_Theme.highlight_text;
                            lplvcd->clrTextBk = g_Theme.highlight_bg;
                        } else {
                            FillRect(hdc, &rc, g_Theme.h_window_bg_brush);
                            lplvcd->clrText = g_Theme.window_text;
                            lplvcd->clrTextBk = g_Theme.window_bg;
                        }

                        SetBkMode(hdc, TRANSPARENT);
                        return CDRF_NEWFONT;
                    }

                    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
                    {
                        HDC hdc = lplvcd->nmcd.hdc;
                        RECT rc;

                        ListView_GetSubItemRect(lplvcd->nmcd.hdr.hwndFrom,
                                                lplvcd->nmcd.dwItemSpec,
                                                lplvcd->iSubItem,
                                                LVIR_BOUNDS,
                                                &rc);

                        if (lplvcd->nmcd.uItemState & CDIS_SELECTED) {
                            FillRect(hdc, &rc, g_Theme.h_highlight_bg_brush);
                            lplvcd->clrText = g_Theme.highlight_text;
                        } else {
                            FillRect(hdc, &rc, g_Theme.h_window_bg_brush);
                            lplvcd->clrText = g_Theme.window_text;
                        }

                        SetBkMode(hdc, TRANSPARENT);
                        return CDRF_NEWFONT;
                    }
                }
            }
        }
    }

    // Find and call original proc
    for (const auto& parent : g_SubclassedParents) {
        if (parent.hwnd == hWnd) {
            return CallWindowProc(parent.originalProc, hWnd, uMsg, wParam, lParam);
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



// --- Hook procedure for dialog theming ---
LRESULT CALLBACK CallWndProcHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && g_ThemeLoaded) {
        CWPSTRUCT* pCwp = (CWPSTRUCT*)lParam;

        switch (pCwp->message) {
            // Phase 1.1: Dialog background erasing
            case WM_ERASEBKGND:
            {
                wchar_t className[256];
                GetClassNameW(pCwp->hwnd, className, 256);

                // Theme dialog backgrounds
                if (wcsstr(className, L"#32770") ||  // Standard dialog class
                    wcsstr(className, L"Dialog") ||
                    wcsstr(className, L"PropertyPage") ||
                    wcsstr(className, L"SysTabControl32")) {
                    HDC hdc = (HDC)pCwp->wParam;
                    RECT rc;
                    GetClientRect(pCwp->hwnd, &rc);
                    FillRect(hdc, &rc, g_Theme.h_button_face_brush);

                    char logMsg[256];
                    sprintf_s(logMsg, 256, "CallWndProcHook: WM_ERASEBKGND themed for class: %S", className);
                    LogError(logMsg);

                    return 1; // Prevent default erase
                }
                break;
            }

            // Phase 1.3: Dynamic control creation
            case WM_CREATE:
            {
                LogError("CallWndProcHook: WM_CREATE detected, theming new window children...");
                EnumChildWindows(pCwp->hwnd, EnumChildProcForAllControls, 0);
                break;
            }

            case WM_SHOWWINDOW:
            {
                if (pCwp->wParam) { // Only care about showing
                    LogError("CallWndProcHook: WM_SHOWWINDOW detected, theming window children...");
                    EnumChildWindows(pCwp->hwnd, EnumChildProcForAllControls, 0);
                }
                break;
            }

            // Phase 2.2: Property sheet page changes
            case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)pCwp->lParam;

                // Property sheet page changing
                if (pnmh->code == PSN_SETACTIVE) {
                    LogError("CallWndProcHook: PSN_SETACTIVE - Property page becoming active, re-theming...");
                    EnumChildWindows(pnmh->hwndFrom, EnumChildProcForAllControls, 0);
                }
                break;
            }

            // WM_CTLCOLOR* message handling
            case WM_CTLCOLORDLG:
            {
                HDC hdc = (HDC)pCwp->wParam;
                SetBkColor(hdc, g_Theme.button_face);
                SetTextColor(hdc, g_Theme.button_text);
                return (LRESULT)g_Theme.h_button_face_brush;
            }

            case WM_CTLCOLORSTATIC:
            {
                HDC hdc = (HDC)pCwp->wParam;
                SetBkColor(hdc, g_Theme.button_face);
                SetTextColor(hdc, g_Theme.window_text);
                return (LRESULT)g_Theme.h_button_face_brush;
            }

            case WM_CTLCOLOREDIT:
            {
                HDC hdc = (HDC)pCwp->wParam;
                SetBkColor(hdc, g_Theme.window_bg);
                SetTextColor(hdc, g_Theme.window_text);
                return (LRESULT)g_Theme.h_window_bg_brush;
            }

            case WM_CTLCOLORBTN:
            {
                HDC hdc = (HDC)pCwp->wParam;
                SetBkColor(hdc, g_Theme.button_face);
                SetTextColor(hdc, g_Theme.button_text);
                return (LRESULT)g_Theme.h_button_face_brush;
            }

            // Phase 2.3: Additional WM_CTLCOLOR* messages
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORSCROLLBAR:
            {
                HDC hdc = (HDC)pCwp->wParam;
                SetBkColor(hdc, g_Theme.window_bg);
                SetTextColor(hdc, g_Theme.window_text);
                return (LRESULT)g_Theme.h_window_bg_brush;
            }

            case WM_INITDIALOG:
            {
                LogError("CallWndProcHook: WM_INITDIALOG detected, theming dialog children...");
                EnumChildWindows(pCwp->hwnd, EnumChildProcForAllControls, 0);
                break;
            }

            // Phase 3.2: Owner-drawn controls
            case WM_PAINT:
            {
                wchar_t className[256];
                GetClassNameW(pCwp->hwnd, className, 256);

                // Custom paint for static controls
                if (wcscmp(className, L"Static") == 0) {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(pCwp->hwnd, &ps);
                    if (hdc) {
                        SetBkColor(hdc, g_Theme.button_face);
                        SetTextColor(hdc, g_Theme.window_text);

                        // Get text and draw it
                        wchar_t text[256];
                        GetWindowTextW(pCwp->hwnd, text, 256);

                        RECT rc;
                        GetClientRect(pCwp->hwnd, &rc);
                        FillRect(hdc, &rc, g_Theme.h_button_face_brush);
                        DrawTextW(hdc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                        EndPaint(pCwp->hwnd, &ps);
                        return 0;
                    }
                }
                break;
            }
        }
    }

    return CallNextHookEx(g_hCallWndProcHook, nCode, wParam, lParam);
}

// --- Helper function to find controls by class name ---
HWND FindControlByClass(HWND hParent, const wchar_t* className) {
    HWND hChild = FindWindowEx(hParent, NULL, NULL, NULL);
    while (hChild != NULL) {
        wchar_t childClassName[256];
        GetClassNameW(hChild, childClassName, 256);
        if (wcscmp(childClassName, className) == 0) {
            return hChild;
        }
        HWND hGrandChild = FindControlByClass(hChild, className);
        if (hGrandChild != NULL) {
            return hGrandChild;
        }
        hChild = FindWindowEx(hParent, hChild, NULL, NULL);
    }
    return NULL;
}

// --- Helper function to find the ListView control ---
HWND FindEventListView(HWND hParent) {
    return FindControlByClass(hParent, L"SysListView32");
}

// --- Callback to find and subclass parent windows containing ListViews ---
BOOL CALLBACK FindAndSubclassParents(HWND hChild, LPARAM lParam) {
    wchar_t className[256];
    GetClassNameW(hChild, className, 256);

    if (wcscmp(className, L"SysListView32") == 0) {
        HWND hParent = GetParent(hChild);
        if (hParent) {
            // Check if this parent is already subclassed
            bool alreadySubclassed = false;
            for (const auto& parent : g_SubclassedParents) {
                if (parent.hwnd == hParent) {
                    alreadySubclassed = true;
                    break;
                }
            }

            if (!alreadySubclassed) {
                SubclassedParent sp;
                sp.hwnd = hParent;
                sp.originalProc = (WNDPROC)SetWindowLongPtr(hParent, GWLP_WNDPROC, (LONG_PTR)CustomParentProc);

                if (sp.originalProc) {
                    g_SubclassedParents.push_back(sp);

                    char logMsg[256];
                    sprintf_s(logMsg, 256, "FindAndSubclassParents: Subclassed parent HWND=%p for ListView HWND=%p", hParent, hChild);
                    LogError(logMsg);
                }
            }
        }
    }

    // Recurse into children
    EnumChildWindows(hChild, FindAndSubclassParents, lParam);
    return TRUE;
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
    // Phase 3.1: DrawText hooks
    DetourAttach((PVOID*)&TrueDrawTextW, (PVOID)DetouredDrawTextW);
    DetourAttach((PVOID*)&TrueDrawTextExW, (PVOID)DetouredDrawTextExW);
    // Phase 4: Window creation hooks
    DetourAttach((PVOID*)&TrueCreateWindowExW, (PVOID)DetouredCreateWindowExW);
    DetourAttach((PVOID*)&TrueSetWindowTextW, (PVOID)DetouredSetWindowTextW);

    LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
        char errorMsg[256];
        sprintf_s(errorMsg, 256, "InitializeTheme: DetourTransactionCommit FAILED with error %ld", error);
        LogError(errorMsg);
        return;
    }

    LogError("InitializeTheme: All Detours hooks installed successfully!");

    // Phase 2.1: Start periodic re-theming timer (500ms interval)
    // Timer fires on the worker thread's message loop (see InitWorkerThread)
    g_ThemeTimerId = SetTimer(NULL, 0, 500, ThemeTimerProc);
    if (g_ThemeTimerId) {
        LogError("InitializeTheme: Periodic re-theming timer started (500ms)");
    } else {
        LogError("InitializeTheme: Failed to start re-theming timer");
    }

    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");

    // Install WH_CALLWNDPROC hook targeting the Event Viewer's UI thread
    // (not GetCurrentThreadId() which is the worker thread with no windows)
    DWORD uiThreadId = 0;
    if (hEventViewer) {
        uiThreadId = GetWindowThreadProcessId(hEventViewer, NULL);
    }
    g_hCallWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProcHook,
        g_hModule, uiThreadId ? uiThreadId : 0);
    if (g_hCallWndProcHook) {
        char hookMsg[256];
        sprintf_s(hookMsg, 256, "InitializeTheme: WH_CALLWNDPROC hook installed on thread %lu", uiThreadId);
        LogError(hookMsg);
    } else {
        LogError("InitializeTheme: Failed to install WH_CALLWNDPROC hook");
    }
    if (hEventViewer) {
        LogError("InitializeTheme: Found Event Viewer window, searching for controls...");

        // Subclass ListView
        g_hListView = FindEventListView(hEventViewer);
        if (g_hListView) {
            g_pOriginalListViewProc = (WNDPROC)SetWindowLongPtr(g_hListView, GWLP_WNDPROC, (LONG_PTR)CustomListViewProc);
            LogError("InitializeTheme: ListView subclassed successfully");

            // Diagnostic logging for ListView styles
            LONG_PTR style = GetWindowLongPtr(g_hListView, GWL_STYLE);
            char styleMsg[256];
            sprintf_s(styleMsg, 256, "ListView: ownerDraw=%d reportView=%d",
                     (style & LVS_OWNERDRAWFIXED) ? 1 : 0,
                     ((style & LVS_TYPEMASK) == LVS_REPORT) ? 1 : 0);
            LogError(styleMsg);

            // Enable double buffering to reduce flicker during custom draw
            DWORD exStyle = ListView_GetExtendedListViewStyle(g_hListView);
            if (!(exStyle & LVS_EX_DOUBLEBUFFER)) {
                ListView_SetExtendedListViewStyle(g_hListView, exStyle | LVS_EX_DOUBLEBUFFER);
                LogError("ListView: double buffering enabled");
            } else {
                LogError("ListView: double buffering already enabled");
            }

            // Set ListView colors directly as a baseline
            ListView_SetBkColor(g_hListView, g_Theme.window_bg);
            ListView_SetTextBkColor(g_hListView, g_Theme.window_bg);
            ListView_SetTextColor(g_hListView, g_Theme.window_text);

            // Force an immediate redraw to trigger custom draw
            InvalidateRect(g_hListView, NULL, TRUE);
        } else {
            LogError("InitializeTheme: ListView not found");
        }

        // Subclass TreeView
        g_hTreeView = FindControlByClass(hEventViewer, L"SysTreeView32");
        if (g_hTreeView) {
            g_pOriginalTreeViewProc = (WNDPROC)SetWindowLongPtr(g_hTreeView, GWLP_WNDPROC, (LONG_PTR)CustomTreeViewProc);
            LogError("InitializeTheme: TreeView subclassed successfully");
        } else {
            LogError("InitializeTheme: TreeView not found");
        }

        // Phase 1.2: Subclass Tab Control
        g_hTabControl = FindControlByClass(hEventViewer, L"SysTabControl32");
        if (g_hTabControl) {
            g_pOriginalTabControlProc = (WNDPROC)SetWindowLongPtr(g_hTabControl, GWLP_WNDPROC, (LONG_PTR)CustomTabControlProc);
            LogError("InitializeTheme: Tab Control subclassed successfully");
        } else {
            LogError("InitializeTheme: Tab Control not found");
        }

        // Find and subclass all parent windows containing ListViews
        LogError("InitializeTheme: Finding and subclassing parent windows with ListViews...");
        EnumChildWindows(hEventViewer, FindAndSubclassParents, 0);

        char parentCountMsg[256];
        sprintf_s(parentCountMsg, 256, "InitializeTheme: Subclassed %zu parent windows", g_SubclassedParents.size());
        LogError(parentCountMsg);

        // Theme all existing controls
        LogError("InitializeTheme: Theming all existing controls...");
        EnumChildWindows(hEventViewer, EnumChildProcForAllControls, 0);

    } else {
        LogError("InitializeTheme: Event Viewer window not found");
    }

    LogError("InitializeTheme: Initialization complete!");
}

void DeinitializeTheme() {
    LogError("DeinitializeTheme: Starting cleanup...");

    // Kill timer
    if (g_ThemeTimerId) {
        KillTimer(NULL, g_ThemeTimerId);
        g_ThemeTimerId = 0;
        LogError("DeinitializeTheme: Timer killed");
    }

    // Unhook CallWndProc
    if (g_hCallWndProcHook) {
        UnhookWindowsHookEx(g_hCallWndProcHook);
        g_hCallWndProcHook = NULL;
        LogError("DeinitializeTheme: CallWndProc hook removed");
    }

    // Restore all subclassed parent windows
    for (const auto& parent : g_SubclassedParents) {
        if (parent.originalProc && parent.hwnd) {
            SetWindowLongPtr(parent.hwnd, GWLP_WNDPROC, (LONG_PTR)parent.originalProc);
        }
    }
    if (!g_SubclassedParents.empty()) {
        char msg[256];
        sprintf_s(msg, 256, "DeinitializeTheme: Restored %zu parent window subclasses", g_SubclassedParents.size());
        LogError(msg);
        g_SubclassedParents.clear();
    }

    // Restore ListView subclass
    if (g_pOriginalListViewProc && g_hListView) {
        SetWindowLongPtr(g_hListView, GWLP_WNDPROC, (LONG_PTR)g_pOriginalListViewProc);
        LogError("DeinitializeTheme: ListView subclass restored");
    }

    // Restore TreeView subclass
    if (g_pOriginalTreeViewProc && g_hTreeView) {
        SetWindowLongPtr(g_hTreeView, GWLP_WNDPROC, (LONG_PTR)g_pOriginalTreeViewProc);
        LogError("DeinitializeTheme: TreeView subclass restored");
    }

    // Restore Tab Control subclass
    if (g_pOriginalTabControlProc && g_hTabControl) {
        SetWindowLongPtr(g_hTabControl, GWLP_WNDPROC, (LONG_PTR)g_pOriginalTabControlProc);
        LogError("DeinitializeTheme: Tab Control subclass restored");
    }

    // Detach Detours hooks
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((PVOID*)&TrueGetSysColor, (PVOID)DetouredGetSysColor);
    DetourDetach((PVOID*)&TrueGetSysColorBrush, (PVOID)DetouredGetSysColorBrush);
    DetourDetach((PVOID*)&TrueDrawThemeBackground, (PVOID)DetouredDrawThemeBackground);
    DetourDetach((PVOID*)&TrueDrawTextW, (PVOID)DetouredDrawTextW);
    DetourDetach((PVOID*)&TrueDrawTextExW, (PVOID)DetouredDrawTextExW);
    DetourDetach((PVOID*)&TrueCreateWindowExW, (PVOID)DetouredCreateWindowExW);
    DetourDetach((PVOID*)&TrueSetWindowTextW, (PVOID)DetouredSetWindowTextW);
    DetourTransactionCommit();
    LogError("DeinitializeTheme: All Detours hooks detached");

    // Clean up brushes
    if (g_ThemeLoaded) {
        DeleteObject(g_Theme.h_window_bg_brush);
        DeleteObject(g_Theme.h_header_bg_brush);
        DeleteObject(g_Theme.h_button_face_brush);
        DeleteObject(g_Theme.h_highlight_bg_brush);
        LogError("DeinitializeTheme: Brushes deleted");
    }

    // Clear themed controls set
    g_ThemedControls.clear();

    LogError("DeinitializeTheme: Cleanup complete");
}

// Worker thread for initialization — avoids loader lock deadlock.
// DllMain holds the OS loader lock, so cross-thread SendMessage calls
// (like ListView_SetBkColor) deadlock when the UI thread tries to
// acquire it. By deferring to a worker thread, the loader lock is
// released before any SendMessage calls happen.
DWORD WINAPI InitWorkerThread(LPVOID lpParam) {
    Sleep(200); // Wait for DllMain to return and loader lock to release
    LogError("InitWorkerThread: Starting initialization...");
    InitializeTheme();

    // Force full repaint now that all hooks and subclasses are installed
    HWND hEV = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEV) {
        RedrawWindow(hEV, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    }

    // Run message loop so SetTimer callbacks actually fire
    // (timers require a message pump on their owning thread)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            g_hModule = hModule;
            LogError("DllMain: DLL_PROCESS_ATTACH - Spawning init worker thread...");
            CreateThread(NULL, 0, InitWorkerThread, NULL, 0, NULL);
            break;
        case DLL_PROCESS_DETACH:
            LogError("DllMain: DLL_PROCESS_DETACH - Cleaning up...");
            DeinitializeTheme();
            break;
    }
    return TRUE;
}
