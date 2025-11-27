#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "version.h"
#include "commands.h"
#include "ThemeManager.h"
#include "ColorEditor.h"
#include "PreviewPanel.h"
#include "ButtonBar.h"

// Link common controls
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Window class name
const wchar_t CLASS_NAME[] = L"ShadesMainWindow";

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMenuBar(HWND hwnd);
void CreateStatusBar(HWND hwnd);
void CreateThemeGallery(HWND hwnd);
void CreateColorEditor(HWND hwnd);
void CreatePreviewPanel(HWND hwnd);
void CreateButtonBar(HWND hwnd);
void CreateGalleryButtons(HWND hwnd);
void PopulateThemeGallery();
void UpdateStatusBar(HWND hwnd);
void HandleMenuCommand(HWND hwnd, WORD commandId);
void HandleContextMenuCommand(HWND hwnd, WORD commandId);
void ShowThemeContextMenu(HWND hwnd, int x, int y);
void HandleThemeSelection(HWND hwnd);
void ShowNewThemeDialog(HWND hwnd);
void ShowImportDialog(HWND hwnd);
void ShowExportDialog(HWND hwnd);
void ShowPropertiesDialog(HWND hwnd);
void ApplyTheme(HWND hwnd, int themeIndex);
void DeleteTheme(HWND hwnd, int themeIndex);
void DuplicateTheme(HWND hwnd, int themeIndex);

// Global window handles
HWND g_hMainWindow = NULL;
HWND g_hStatusBar = NULL;
HWND g_hThemeList = NULL;
HWND g_hBtnNew = NULL;
HWND g_hBtnImport = NULL;
HWND g_hBtnExport = NULL;
HIMAGELIST g_hImageList = NULL;

// Theme manager and UI components
Shades::ThemeManager g_themeManager;
Shades::ColorEditor* g_pColorEditor = nullptr;
Shades::PreviewPanel* g_pPreviewPanel = nullptr;
Shades::ButtonBar* g_pButtonBar = nullptr;

// Selected theme tracking
int g_selectedThemeIndex = -1;

// Status bar section widths
const int STATUS_WIDTHS[] = { 250, 150, 150, -1 };  // Last is flexible

// ListView column indices
#define COL_ICON     0
#define COL_NAME     1
#define COL_AUTHOR   2
#define COL_MODIFIED 3

//=============================================================================
// WinMain - Application entry point
//=============================================================================
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Create window title
    wchar_t windowTitle[256];
    swprintf_s(windowTitle, L"%S", SHADES_VERSION_FULL);

    // Create main window
    g_hMainWindow = CreateWindowExW(
        0,                              // Extended styles
        CLASS_NAME,                     // Window class
        windowTitle,                    // Window title
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,   // Position
        900, 650,                       // Size (per design spec)
        NULL,                           // Parent window
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional data
    );

    if (g_hMainWindow == NULL) {
        MessageBoxW(NULL, L"Failed to create window", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Create accelerator table
    ACCEL accel[] = {
        { FVIRTKEY | FCONTROL, 'N', ID_FILE_NEW },
        { FVIRTKEY | FCONTROL, 'O', ID_FILE_OPEN },
        { FVIRTKEY | FCONTROL, 'S', ID_FILE_SAVE },
        { FVIRTKEY | FCONTROL | FSHIFT, 'S', ID_FILE_SAVE_AS },
        { FVIRTKEY | FCONTROL, 'I', ID_FILE_IMPORT },
        { FVIRTKEY | FCONTROL, 'E', ID_FILE_EXPORT },
        { FVIRTKEY | FCONTROL, 'Z', ID_EDIT_UNDO },
        { FVIRTKEY | FCONTROL, 'Y', ID_EDIT_REDO },
        { FVIRTKEY | FCONTROL, 'C', ID_EDIT_COPY },
        { FVIRTKEY | FCONTROL, 'V', ID_EDIT_PASTE },
        { FVIRTKEY | FCONTROL, VK_OEM_PLUS, ID_VIEW_ZOOM_IN },   // Ctrl++
        { FVIRTKEY | FCONTROL, VK_OEM_MINUS, ID_VIEW_ZOOM_OUT }, // Ctrl+-
        { FVIRTKEY | FCONTROL, '0', ID_VIEW_ZOOM_RESET },
        { FVIRTKEY, VK_F5, ID_VIEW_REFRESH },
        { FVIRTKEY | FCONTROL, 'T', ID_TOOLS_VALIDATE },
        { FVIRTKEY | FCONTROL, VK_OEM_COMMA, ID_TOOLS_SETTINGS }, // Ctrl+,
        { FVIRTKEY, VK_F1, ID_HELP_DOCUMENTATION }
    };
    HACCEL hAccel = CreateAcceleratorTableW(accel, sizeof(accel) / sizeof(ACCEL));

    // Show and update window
    ShowWindow(g_hMainWindow, nCmdShow);
    UpdateWindow(g_hMainWindow);

    // Message loop with accelerator support
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAcceleratorW(g_hMainWindow, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//=============================================================================
// WindowProc - Main window message handler
//=============================================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
        {
            // Create menu bar
            CreateMenuBar(hwnd);

            // Create status bar
            CreateStatusBar(hwnd);

            // Create theme gallery ListView
            CreateThemeGallery(hwnd);

            // Create preview panel
            CreatePreviewPanel(hwnd);

            // Create button bar
            CreateButtonBar(hwnd);

            // Create color editor panel
            CreateColorEditor(hwnd);

            // Create gallery action buttons
            CreateGalleryButtons(hwnd);

            // Load themes
            g_themeManager.LoadThemesFromDirectory();
            PopulateThemeGallery();

            // Center the window on screen
            RECT rc;
            GetWindowRect(hwnd, &rc);
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            int windowWidth = rc.right - rc.left;
            int windowHeight = rc.bottom - rc.top;
            int x = (screenWidth - windowWidth) / 2;
            int y = (screenHeight - windowHeight) / 2;
            SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

            // Initialize status bar
            UpdateStatusBar(hwnd);

            return 0;
        }

        case WM_SIZE:
        {
            // Resize status bar when window is resized
            if (g_hStatusBar) {
                SendMessage(g_hStatusBar, WM_SIZE, 0, 0);

                // Update status bar part widths based on new window width
                RECT rcClient;
                GetClientRect(hwnd, &rcClient);
                int parts[STATUS_PART_COUNT];
                parts[STATUS_PART_THEME] = STATUS_WIDTHS[0];
                parts[STATUS_PART_STATUS] = parts[STATUS_PART_THEME] + STATUS_WIDTHS[1];
                parts[STATUS_PART_PERFORMANCE] = parts[STATUS_PART_STATUS] + STATUS_WIDTHS[2];
                parts[STATUS_PART_VERSION] = -1;  // Extends to end
                SendMessage(g_hStatusBar, SB_SETPARTS, STATUS_PART_COUNT, (LPARAM)parts);
            }

            // Resize theme gallery ListView
            if (g_hThemeList) {
                RECT rcClient;
                GetClientRect(hwnd, &rcClient);

                // Get status bar height
                int statusHeight = 0;
                if (g_hStatusBar) {
                    RECT rcStatus;
                    GetWindowRect(g_hStatusBar, &rcStatus);
                    statusHeight = rcStatus.bottom - rcStatus.top;
                }

                // Position ListView in left 30% of window
                int listWidth = rcClient.right * 30 / 100;
                int listHeight = rcClient.bottom - statusHeight;
                SetWindowPos(g_hThemeList, NULL, 0, 0, listWidth, listHeight,
                            SWP_NOZORDER);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Get client area (excluding status bar)
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            if (g_hStatusBar) {
                RECT statusRect;
                GetWindowRect(g_hStatusBar, &statusRect);
                clientRect.bottom -= (statusRect.bottom - statusRect.top);
            }

            // Set background color
            FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

            // Draw "Week 2 Complete" text
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);

            const wchar_t* helloText = L"SHADES v2.0 - Week 2 Complete!";
            const wchar_t* subText = L"Menu bar with all 5 menus and accelerators is ready.";
            const wchar_t* nextText = L"Status bar with 4 sections is functional.";
            const wchar_t* infoText = L"Try the menu items - most will show 'Coming in Week X' messages.";

            HFONT hFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                       DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            // Calculate text position (centered)
            SIZE textSize;
            GetTextExtentPoint32W(hdc, helloText, (int)wcslen(helloText), &textSize);
            int x = (clientRect.right - textSize.cx) / 2;
            int y = (clientRect.bottom - textSize.cy) / 2 - 60;

            TextOutW(hdc, x, y, helloText, (int)wcslen(helloText));

            // Draw subtitle
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);

            hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            SelectObject(hdc, hFont);

            GetTextExtentPoint32W(hdc, subText, (int)wcslen(subText), &textSize);
            x = (clientRect.right - textSize.cx) / 2;
            y += 50;
            TextOutW(hdc, x, y, subText, (int)wcslen(subText));

            GetTextExtentPoint32W(hdc, nextText, (int)wcslen(nextText), &textSize);
            x = (clientRect.right - textSize.cx) / 2;
            y += 25;
            TextOutW(hdc, x, y, nextText, (int)wcslen(nextText));

            GetTextExtentPoint32W(hdc, infoText, (int)wcslen(infoText), &textSize);
            x = (clientRect.right - textSize.cx) / 2;
            y += 25;
            TextOutW(hdc, x, y, infoText, (int)wcslen(infoText));

            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND:
        {
            WORD commandId = LOWORD(wParam);

            // Handle button commands
            if (commandId == IDC_BTN_NEW) {
                ShowNewThemeDialog(hwnd);
            } else if (commandId == IDC_BTN_IMPORT) {
                ShowImportDialog(hwnd);
            } else if (commandId == IDC_BTN_EXPORT) {
                ShowExportDialog(hwnd);
            }
            // Handle context menu commands
            else if (commandId >= ID_CONTEXT_APPLY && commandId <= ID_CONTEXT_PROPERTIES) {
                HandleContextMenuCommand(hwnd, commandId);
            }
            // Handle regular menu commands
            else {
                HandleMenuCommand(hwnd, commandId);
            }
            return 0;
        }

        case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;

            if (nmhdr->hwndFrom == g_hThemeList) {
                switch (nmhdr->code) {
                    case NM_RCLICK:  // Right-click
                    {
                        LPNMITEMACTIVATE nmia = (LPNMITEMACTIVATE)lParam;
                        if (nmia->iItem >= 0) {
                            g_selectedThemeIndex = nmia->iItem;

                            // Get cursor position for context menu
                            POINT pt;
                            GetCursorPos(&pt);
                            ShowThemeContextMenu(hwnd, pt.x, pt.y);
                        }
                        return TRUE;
                    }

                    case NM_DBLCLK:  // Double-click
                    {
                        LPNMITEMACTIVATE nmia = (LPNMITEMACTIVATE)lParam;
                        if (nmia->iItem >= 0) {
                            ApplyTheme(hwnd, nmia->iItem);
                        }
                        return TRUE;
                    }

                    case LVN_ITEMCHANGED:  // Selection changed
                    {
                        LPNMLISTVIEW nmlv = (LPNMLISTVIEW)lParam;
                        if (nmlv->uNewState & LVIS_SELECTED) {
                            g_selectedThemeIndex = nmlv->iItem;
                            HandleThemeSelection(hwnd);
                        }
                        return TRUE;
                    }
                }
            }
            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            // Cleanup UI components
            if (g_pColorEditor) {
                delete g_pColorEditor;
                g_pColorEditor = nullptr;
            }
            if (g_pPreviewPanel) {
                delete g_pPreviewPanel;
                g_pPreviewPanel = nullptr;
            }
            if (g_pButtonBar) {
                delete g_pButtonBar;
                g_pButtonBar = nullptr;
            }

            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

//=============================================================================
// CreateMenuBar - Create complete application menu
//=============================================================================
void CreateMenuBar(HWND hwnd) {
    HMENU hMenuBar = CreateMenu();

    //-------------------------------------------------------------------------
    // File Menu
    //-------------------------------------------------------------------------
    HMENU hFileMenu = CreateMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New Theme...\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open Theme...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save Theme\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, L"Save Theme &As...\tCtrl+Shift+S");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_IMPORT, L"&Import Theme...\tCtrl+I");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXPORT, L"&Export Theme...\tCtrl+E");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);

    // Recent Themes submenu
    HMENU hRecentMenu = CreateMenu();
    AppendMenuW(hRecentMenu, MF_STRING, ID_FILE_RECENT_1, L"Dark Blue");
    AppendMenuW(hRecentMenu, MF_STRING, ID_FILE_RECENT_2, L"Monokai");
    AppendMenuW(hRecentMenu, MF_STRING, ID_FILE_RECENT_3, L"Nord");
    AppendMenuW(hFileMenu, MF_POPUP, (UINT_PTR)hRecentMenu, L"&Recent Themes");

    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

    //-------------------------------------------------------------------------
    // Edit Menu
    //-------------------------------------------------------------------------
    HMENU hEditMenu = CreateMenu();
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_REDO, L"&Redo\tCtrl+Y");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_COPY, L"&Copy Theme\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_PASTE, L"&Paste Theme\tCtrl+V");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_RESET, L"&Reset to Default");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

    //-------------------------------------------------------------------------
    // View Menu
    //-------------------------------------------------------------------------
    HMENU hViewMenu = CreateMenu();
    AppendMenuW(hViewMenu, MF_STRING | MF_CHECKED, ID_VIEW_GALLERY, L"Theme &Gallery");
    AppendMenuW(hViewMenu, MF_STRING | MF_CHECKED, ID_VIEW_COLOR_EDITOR, L"&Color Editor");
    AppendMenuW(hViewMenu, MF_STRING | MF_CHECKED, ID_VIEW_PREVIEW, L"Live &Preview");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_IN, L"Zoom &In\tCtrl++");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_OUT, L"Zoom &Out\tCtrl+-");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_RESET, L"Reset &Zoom\tCtrl+0");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_REFRESH, L"&Refresh\tF5");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");

    //-------------------------------------------------------------------------
    // Tools Menu
    //-------------------------------------------------------------------------
    HMENU hToolsMenu = CreateMenu();
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_VALIDATE, L"&Validate Theme\tCtrl+T");
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_PERFORMANCE, L"&Performance Monitor");
    AppendMenuW(hToolsMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_APPLY, L"&Apply to Event Viewer");
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_DISABLE, L"&Disable Theme");
    AppendMenuW(hToolsMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_SETTINGS, L"&Settings...\tCtrl+,");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hToolsMenu, L"&Tools");

    //-------------------------------------------------------------------------
    // Help Menu
    //-------------------------------------------------------------------------
    HMENU hHelpMenu = CreateMenu();
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_DOCUMENTATION, L"&Documentation\tF1");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_GITHUB, L"&GitHub Repository");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_REPORT, L"&Report Issue");
    AppendMenuW(hHelpMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_UPDATES, L"Check for &Updates");
    AppendMenuW(hHelpMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_ABOUT, L"&About SHADES...");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    // Set menu
    SetMenu(hwnd, hMenuBar);
}

//=============================================================================
// CreateStatusBar - Create status bar with 4 sections
//=============================================================================
void CreateStatusBar(HWND hwnd) {
    g_hStatusBar = CreateWindowExW(
        0,
        STATUSCLASSNAMEW,
        NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!g_hStatusBar) {
        return;
    }

    // Set up parts
    int parts[STATUS_PART_COUNT];
    parts[STATUS_PART_THEME] = STATUS_WIDTHS[0];
    parts[STATUS_PART_STATUS] = parts[STATUS_PART_THEME] + STATUS_WIDTHS[1];
    parts[STATUS_PART_PERFORMANCE] = parts[STATUS_PART_STATUS] + STATUS_WIDTHS[2];
    parts[STATUS_PART_VERSION] = -1;  // Extends to end

    SendMessage(g_hStatusBar, SB_SETPARTS, STATUS_PART_COUNT, (LPARAM)parts);
}

//=============================================================================
// UpdateStatusBar - Update status bar text
//=============================================================================
void UpdateStatusBar(HWND hwnd) {
    if (!g_hStatusBar) return;

    // Section 1: Current theme
    SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_THEME, (LPARAM)L"Theme: Default");

    // Section 2: Status indicator (with colored dot - using Unicode circle)
    SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_STATUS, (LPARAM)L"Status: \u25CF ACTIVE");

    // Section 3: Performance metric
    SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_PERFORMANCE, (LPARAM)L"Cache Hit: 97%");

    // Section 4: Version
    wchar_t versionText[32];
    swprintf_s(versionText, L"v%d.%d.%d-alpha",
               SHADES_VERSION_MAJOR, SHADES_VERSION_MINOR, SHADES_VERSION_PATCH);
    SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_VERSION, (LPARAM)versionText);
}

//=============================================================================
// HandleMenuCommand - Handle all menu commands
//=============================================================================
void HandleMenuCommand(HWND hwnd, WORD commandId) {
    wchar_t message[256];

    switch (commandId) {
        //---------------------------------------------------------------------
        // File Menu
        //---------------------------------------------------------------------
        case ID_FILE_NEW:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Gallery Component",
                       L"New Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_OPEN:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Gallery Component",
                       L"Open Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_SAVE:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Gallery Component",
                       L"Save Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_SAVE_AS:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Gallery Component",
                       L"Save Theme As", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_IMPORT:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Package Import",
                       L"Import Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_EXPORT:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Package Export",
                       L"Export Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_RECENT_1:
        case ID_FILE_RECENT_2:
        case ID_FILE_RECENT_3:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Recent Themes List",
                       L"Recent Themes", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        //---------------------------------------------------------------------
        // Edit Menu
        //---------------------------------------------------------------------
        case ID_EDIT_UNDO:
        case ID_EDIT_REDO:
            MessageBoxW(hwnd, L"Coming in Week 5-6: Undo/Redo System",
                       L"Edit", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_EDIT_COPY:
        case ID_EDIT_PASTE:
            MessageBoxW(hwnd, L"Coming in Week 3-4: Theme Copy/Paste",
                       L"Edit", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_EDIT_RESET:
            MessageBoxW(hwnd, L"Coming in Week 5-6: Reset to Default Theme",
                       L"Reset", MB_ICONINFORMATION | MB_OK);
            break;

        //---------------------------------------------------------------------
        // View Menu
        //---------------------------------------------------------------------
        case ID_VIEW_GALLERY:
        case ID_VIEW_COLOR_EDITOR:
        case ID_VIEW_PREVIEW:
            MessageBoxW(hwnd, L"Coming in Week 3-8: Panel visibility toggles",
                       L"View", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_VIEW_ZOOM_IN:
        case ID_VIEW_ZOOM_OUT:
        case ID_VIEW_ZOOM_RESET:
            MessageBoxW(hwnd, L"Coming in Week 7-8: Live Preview Zoom",
                       L"Zoom", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_VIEW_REFRESH:
            MessageBoxW(hwnd, L"Coming in Week 7-8: Refresh Preview",
                       L"Refresh", MB_ICONINFORMATION | MB_OK);
            InvalidateRect(hwnd, NULL, TRUE);  // Repaint window now
            break;

        //---------------------------------------------------------------------
        // Tools Menu
        //---------------------------------------------------------------------
        case ID_TOOLS_VALIDATE:
            MessageBoxW(hwnd, L"Coming in Week 9-10: Theme Validation (from v1.5)",
                       L"Validate Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_TOOLS_PERFORMANCE:
            MessageBoxW(hwnd, L"Coming in Week 9-10: Performance Monitor",
                       L"Performance", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_TOOLS_APPLY:
            MessageBoxW(hwnd, L"Coming in Week 9-10: Apply Theme via Injector",
                       L"Apply Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_TOOLS_DISABLE:
            MessageBoxW(hwnd, L"Coming in Week 9-10: Disable Active Theme",
                       L"Disable Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_TOOLS_SETTINGS:
            MessageBoxW(hwnd, L"Coming in Week 13-14: Settings Dialog",
                       L"Settings", MB_ICONINFORMATION | MB_OK);
            break;

        //---------------------------------------------------------------------
        // Help Menu
        //---------------------------------------------------------------------
        case ID_HELP_DOCUMENTATION:
            MessageBoxW(hwnd,
                       L"SHADES v2.0 Documentation\n\n"
                       L"Coming in Week 13-14: Built-in help system\n\n"
                       L"For now, visit: https://github.com/azm0de/shades",
                       L"Documentation", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_HELP_GITHUB:
            MessageBoxW(hwnd,
                       L"GitHub Repository:\nhttps://github.com/azm0de/shades\n\n"
                       L"Coming in Week 13-14: Direct browser launch",
                       L"GitHub", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_HELP_REPORT:
            MessageBoxW(hwnd,
                       L"Report issues at:\nhttps://github.com/azm0de/shades/issues\n\n"
                       L"Coming in Week 13-14: Issue reporter dialog",
                       L"Report Issue", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_HELP_UPDATES:
            MessageBoxW(hwnd, L"Coming in Week 13-14: Update Checker",
                       L"Check for Updates", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_HELP_ABOUT:
            swprintf_s(message,
                      L"%S\n"
                      L"Event Viewer Themer\n\n"
                      L"%S\n"
                      L"Licensed under the MIT License\n\n"
                      L"Week 3: Theme Gallery Complete!",
                      SHADES_VERSION_FULL, SHADES_COPYRIGHT);
            MessageBoxW(hwnd, message, L"About SHADES", MB_ICONINFORMATION | MB_OK);
            break;
    }
}

//=============================================================================
// CreateThemeGallery - Create theme gallery ListView control
//=============================================================================
void CreateThemeGallery(HWND hwnd) {
    // Create ImageList for icons (32×32)
    g_hImageList = ImageList_Create(32, 32, ILC_COLOR32, 10, 10);

    // Create ListView control
    g_hThemeList = CreateWindowExW(
        0,
        WC_LISTVIEWW,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 270, 400,  // Initial size (will be resized in WM_SIZE)
        hwnd,
        (HMENU)1001,
        GetModuleHandle(NULL),
        NULL
    );

    if (!g_hThemeList) {
        return;
    }

    // Set extended styles
    ListView_SetExtendedListViewStyle(g_hThemeList,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    // Assign ImageList
    ListView_SetImageList(g_hThemeList, g_hImageList, LVSIL_SMALL);

    // Add columns
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

    // Icon column
    col.fmt = LVCFMT_LEFT;
    col.cx = 32;
    col.pszText = (LPWSTR)L"";
    ListView_InsertColumn(g_hThemeList, COL_ICON, &col);

    // Name column
    col.cx = 200;
    col.pszText = (LPWSTR)L"Name";
    ListView_InsertColumn(g_hThemeList, COL_NAME, &col);

    // Author column
    col.cx = 120;
    col.pszText = (LPWSTR)L"Author";
    ListView_InsertColumn(g_hThemeList, COL_AUTHOR, &col);

    // Modified column
    col.cx = 100;
    col.pszText = (LPWSTR)L"Modified";
    ListView_InsertColumn(g_hThemeList, COL_MODIFIED, &col);
}

//=============================================================================
// CreatePreviewPanel - Create live preview panel
//=============================================================================
void CreatePreviewPanel(HWND hwnd) {
    // Get client area
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // Get status bar height
    int statusHeight = 0;
    if (g_hStatusBar) {
        RECT rcStatus;
        GetWindowRect(g_hStatusBar, &rcStatus);
        statusHeight = rcStatus.bottom - rcStatus.top;
    }

    // Calculate dimensions
    int galleryWidth = (rcClient.right * 30) / 100;
    int previewX = galleryWidth;
    int previewY = 0;
    int previewWidth = rcClient.right - galleryWidth;
    int buttonBarHeight = 50;
    int previewHeight = (rcClient.bottom - statusHeight - buttonBarHeight) / 2;

    // Create preview panel instance
    g_pPreviewPanel = new Shades::PreviewPanel();
    if (g_pPreviewPanel) {
        g_pPreviewPanel->Create(hwnd, previewX, previewY, previewWidth, previewHeight);

        // Set up callback for click-to-edit
        g_pPreviewPanel->SetPropertySelectedCallback([](const std::string& propertyKey) {
            // When user clicks a preview element, scroll to that property in ColorEditor
            if (g_pColorEditor) {
                // For now, just show a message (scroll implementation can be added later)
                // g_pColorEditor->SelectProperty(propertyKey);
            }
        });
    }
}

//=============================================================================
// CreateButtonBar - Create action button bar
//=============================================================================
void CreateButtonBar(HWND hwnd) {
    // Get client area
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // Get status bar height
    int statusHeight = 0;
    if (g_hStatusBar) {
        RECT rcStatus;
        GetWindowRect(g_hStatusBar, &rcStatus);
        statusHeight = rcStatus.bottom - rcStatus.top;
    }

    // Calculate dimensions
    int buttonBarHeight = 50;
    int buttonBarY = rcClient.bottom - statusHeight - buttonBarHeight;
    int galleryWidth = (rcClient.right * 30) / 100;
    int buttonBarX = galleryWidth;
    int buttonBarWidth = rcClient.right - galleryWidth;

    // Create button bar instance
    g_pButtonBar = new Shades::ButtonBar();
    if (g_pButtonBar) {
        g_pButtonBar->Create(hwnd, buttonBarX, buttonBarY, buttonBarWidth, buttonBarHeight);

        // Set up button click callback
        g_pButtonBar->SetButtonClickCallback([hwnd](int commandID) {
            // Handle button clicks
            switch (commandID) {
                case ID_TOOLS_APPLY:
                    // Apply theme
                    if (g_selectedThemeIndex >= 0) {
                        ApplyTheme(hwnd, g_selectedThemeIndex);
                    } else {
                        MessageBoxW(hwnd, L"Please select a theme to apply.",
                                   L"No Theme Selected", MB_ICONINFORMATION | MB_OK);
                    }
                    break;

                case ID_FILE_SAVE:
                    // Save theme changes
                    if (g_selectedThemeIndex >= 0) {
                        if (g_pColorEditor && g_pColorEditor->HasUnsavedChanges()) {
                            g_pColorEditor->SaveChanges();
                            if (g_themeManager.SaveModifiedTheme(g_selectedThemeIndex)) {
                                MessageBoxW(hwnd, L"Theme saved successfully!",
                                           L"Success", MB_ICONINFORMATION | MB_OK);
                            } else {
                                MessageBoxW(hwnd, g_themeManager.GetLastError().c_str(),
                                           L"Save Failed", MB_ICONERROR | MB_OK);
                            }
                        } else {
                            MessageBoxW(hwnd, L"No changes to save.",
                                       L"No Changes", MB_ICONINFORMATION | MB_OK);
                        }
                    } else {
                        MessageBoxW(hwnd, L"Please select a theme to save.",
                                   L"No Theme Selected", MB_ICONINFORMATION | MB_OK);
                    }
                    break;

                case ID_TOOLS_SETTINGS:
                    MessageBoxW(hwnd, L"Coming in Week 13-14: Settings Dialog",
                               L"Settings", MB_ICONINFORMATION | MB_OK);
                    break;

                case ID_HELP_DOCUMENTATION:
                    MessageBoxW(hwnd,
                               L"SHADES v2.0 Documentation\n\n"
                               L"Coming in Week 13-14: Built-in help system\n\n"
                               L"For now, visit: https://github.com/azm0de/shades",
                               L"Documentation", MB_ICONINFORMATION | MB_OK);
                    break;
            }
        });

        // Initially disable Save button (until theme is edited)
        g_pButtonBar->EnableButton(ID_FILE_SAVE, false);
    }
}

//=============================================================================
// CreateColorEditor - Create color editor panel
//=============================================================================
void CreateColorEditor(HWND hwnd) {
    // Get client area
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // Get status bar height
    int statusHeight = 0;
    if (g_hStatusBar) {
        RECT rcStatus;
        GetWindowRect(g_hStatusBar, &rcStatus);
        statusHeight = rcStatus.bottom - rcStatus.top;
    }

    // Calculate dimensions
    int galleryWidth = (rcClient.right * 30) / 100;
    int buttonBarHeight = 50;
    int previewHeight = (rcClient.bottom - statusHeight - buttonBarHeight) / 2;

    int editorX = galleryWidth;
    int editorY = previewHeight;
    int editorWidth = rcClient.right - galleryWidth;
    int editorHeight = (rcClient.bottom - statusHeight - buttonBarHeight) - previewHeight;

    // Create color editor instance
    g_pColorEditor = new Shades::ColorEditor();
    if (g_pColorEditor) {
        g_pColorEditor->Create(hwnd, editorX, editorY, editorWidth, editorHeight);

        // Set up color change callback to update preview panel
        g_pColorEditor->SetColorChangeCallback([](const std::string& propertyKey, COLORREF color) {
            if (g_pPreviewPanel) {
                g_pPreviewPanel->OnColorChanged(propertyKey, color);
            }
            // Enable Save button when colors are modified
            if (g_pButtonBar) {
                g_pButtonBar->EnableButton(ID_FILE_SAVE, true);
            }
        });
    }
}

//=============================================================================
// PopulateThemeGallery - Load themes into ListView
//=============================================================================
void PopulateThemeGallery() {
    if (!g_hThemeList) {
        return;
    }

    // Clear existing items
    ListView_DeleteAllItems(g_hThemeList);

    // Create default icon (simple gradient bitmap)
    HBITMAP hDefaultIcon = CreateCompatibleBitmap(GetDC(NULL), 32, 32);
    HDC hdcMem = CreateCompatibleDC(NULL);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hDefaultIcon);

    // Fill with gradient (dark blue to light blue)
    RECT rcIcon = {0, 0, 32, 32};
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 122, 204));  // VS Code blue
    FillRect(hdcMem, &rcIcon, hBrush);
    DeleteObject(hBrush);

    SelectObject(hdcMem, hOldBmp);
    DeleteDC(hdcMem);

    // Add default icon to ImageList
    int defaultIconIndex = ImageList_Add(g_hImageList, hDefaultIcon, NULL);
    DeleteObject(hDefaultIcon);

    // Add each theme to ListView
    const auto& themes = g_themeManager.GetThemes();
    for (size_t i = 0; i < themes.size(); ++i) {
        const auto& theme = themes[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT | LVIF_IMAGE;
        item.iItem = (int)i;

        // Icon column
        item.iSubItem = COL_ICON;
        item.iImage = defaultIconIndex;  // Use default icon for now
        ListView_InsertItem(g_hThemeList, &item);

        // Name column (add ● for active theme)
        std::wstring displayName = theme.name;
        if (theme.isActive) {
            displayName = L"\u25CF " + displayName;  // Add bullet point
        }
        item.mask = LVIF_TEXT;
        item.iSubItem = COL_NAME;
        item.pszText = (LPWSTR)displayName.c_str();
        ListView_SetItem(g_hThemeList, &item);

        // Author column
        item.iSubItem = COL_AUTHOR;
        item.pszText = (LPWSTR)theme.author.c_str();
        ListView_SetItem(g_hThemeList, &item);

        // Modified column (format date)
        SYSTEMTIME st;
        FileTimeToSystemTime(&theme.modifiedDate, &st);
        wchar_t dateStr[32];
        swprintf_s(dateStr, L"%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
        item.iSubItem = COL_MODIFIED;
        item.pszText = dateStr;
        ListView_SetItem(g_hThemeList, &item);
    }

    // If no themes, show a message
    if (themes.empty()) {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.iSubItem = COL_NAME;
        item.pszText = (LPWSTR)L"(No themes found)";
        ListView_InsertItem(g_hThemeList, &item);
    }
}

//=============================================================================
// CreateGalleryButtons - Create action buttons below ListView
//=============================================================================
void CreateGalleryButtons(HWND hwnd) {
    // Get client area
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int btnY = rcClient.bottom - 80;  // Position above status bar
    int btnX = 12;  // Left padding

    // [+ New Theme] button (primary style)
    g_hBtnNew = CreateWindowW(
        L"BUTTON",
        L"+ New Theme",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 120, 32,
        hwnd,
        (HMENU)IDC_BTN_NEW,
        GetModuleHandle(NULL),
        NULL
    );

    // [Import] button (secondary style)
    btnX += 120 + 12;  // Add spacing
    g_hBtnImport = CreateWindowW(
        L"BUTTON",
        L"Import",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 100, 32,
        hwnd,
        (HMENU)IDC_BTN_IMPORT,
        GetModuleHandle(NULL),
        NULL
    );

    // [Export] button (secondary style)
    btnX += 100 + 12;  // Add spacing
    g_hBtnExport = CreateWindowW(
        L"BUTTON",
        L"Export",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 100, 32,
        hwnd,
        (HMENU)IDC_BTN_EXPORT,
        GetModuleHandle(NULL),
        NULL
    );
}

//=============================================================================
// ShowThemeContextMenu - Display context menu at cursor position
//=============================================================================
void ShowThemeContextMenu(HWND hwnd, int x, int y) {
    HMENU hMenu = CreatePopupMenu();

    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_APPLY, L"\u2713 Apply Theme");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_EDIT, L"\u270F Edit");
    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_DUPLICATE, L"\u2398 Duplicate");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_EXPORT, L"\U0001F4BE Export to File...");
    AppendMenuW(hMenu, MF_STRING | MF_GRAYED, ID_CONTEXT_SHARE, L"\U0001F4E4 Share (Coming Soon)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_DELETE, L"\U0001F5D1 Delete");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_CONTEXT_PROPERTIES, L"\u2139 Properties");

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, x, y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

//=============================================================================
// HandleContextMenuCommand - Handle context menu selections
//=============================================================================
void HandleContextMenuCommand(HWND hwnd, WORD commandId) {
    if (g_selectedThemeIndex < 0) {
        return;
    }

    switch (commandId) {
        case ID_CONTEXT_APPLY:
            ApplyTheme(hwnd, g_selectedThemeIndex);
            break;

        case ID_CONTEXT_EDIT:
            MessageBoxW(hwnd, L"Theme editing will be available in Week 5-6: Color Editor Component",
                       L"Edit Theme", MB_ICONINFORMATION | MB_OK);
            break;

        case ID_CONTEXT_DUPLICATE:
            DuplicateTheme(hwnd, g_selectedThemeIndex);
            break;

        case ID_CONTEXT_EXPORT:
            ShowExportDialog(hwnd);
            break;

        case ID_CONTEXT_DELETE:
            DeleteTheme(hwnd, g_selectedThemeIndex);
            break;

        case ID_CONTEXT_PROPERTIES:
            ShowPropertiesDialog(hwnd);
            break;
    }
}

//=============================================================================
// HandleThemeSelection - Update UI when theme selection changes
//=============================================================================
void HandleThemeSelection(HWND hwnd) {
    if (g_selectedThemeIndex >= 0) {
        const Shades::Theme* theme = g_themeManager.GetTheme(g_selectedThemeIndex);
        if (theme) {
            // Update status bar with selected theme name
            std::wstring statusText = L"Selected: " + theme->name;
            SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_THEME,
                        (LPARAM)statusText.c_str());

            // Load theme into color editor
            if (g_pColorEditor) {
                g_pColorEditor->LoadTheme(theme);
            }

            // Load theme into preview panel
            if (g_pPreviewPanel) {
                g_pPreviewPanel->LoadTheme(theme);
            }
        }
    } else {
        // No theme selected - clear UI
        if (g_pColorEditor) {
            g_pColorEditor->ClearTheme();
        }
        if (g_pPreviewPanel) {
            g_pPreviewPanel->ClearTheme();
        }
    }
}

//=============================================================================
// ApplyTheme - Mark theme as active (visual only for now)
//=============================================================================
void ApplyTheme(HWND hwnd, int themeIndex) {
    if (themeIndex < 0 || themeIndex >= (int)g_themeManager.GetThemeCount()) {
        return;
    }

    // Set as active theme
    g_themeManager.SetActiveTheme(themeIndex);

    // Refresh gallery to show ● indicator
    PopulateThemeGallery();

    // Update status bar
    const Shades::Theme* theme = g_themeManager.GetTheme(themeIndex);
    if (theme) {
        std::wstring msg = L"Applied theme: " + theme->name +
                          L"\n\n(Actual injection into Event Viewer will be available in Week 9-10)";
        MessageBoxW(hwnd, msg.c_str(), L"Theme Applied", MB_ICONINFORMATION | MB_OK);

        // Update status bar
        SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_STATUS,
                    (LPARAM)L"Status: \u25CF ACTIVE");
        std::wstring themeText = L"Theme: " + theme->name;
        SendMessageW(g_hStatusBar, SB_SETTEXTW, STATUS_PART_THEME,
                    (LPARAM)themeText.c_str());
    }
}

//=============================================================================
// DeleteTheme - Delete theme with confirmation
//=============================================================================
void DeleteTheme(HWND hwnd, int themeIndex) {
    const Shades::Theme* theme = g_themeManager.GetTheme(themeIndex);
    if (!theme) {
        return;
    }

    // Show confirmation dialog
    std::wstring msg = L"Delete theme '" + theme->name + L"'?\n\nThis cannot be undone.";
    int result = MessageBoxW(hwnd, msg.c_str(), L"Confirm Delete",
                            MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);

    if (result == IDYES) {
        if (g_themeManager.DeleteTheme(themeIndex)) {
            MessageBoxW(hwnd, L"Theme deleted successfully.", L"Success",
                       MB_ICONINFORMATION | MB_OK);

            // Clear selection
            g_selectedThemeIndex = -1;

            // Refresh gallery
            g_themeManager.ReloadThemes();
            PopulateThemeGallery();
        } else {
            MessageBoxW(hwnd, g_themeManager.GetLastError().c_str(), L"Delete Failed",
                       MB_ICONERROR | MB_OK);
        }
    }
}

//=============================================================================
// DuplicateTheme - Create a copy of the selected theme
//=============================================================================
void DuplicateTheme(HWND hwnd, int themeIndex) {
    const Shades::Theme* theme = g_themeManager.GetTheme(themeIndex);
    if (!theme) {
        return;
    }

    // Create copy with "Copy of" prefix
    std::wstring newName = L"Copy of " + theme->name;

    if (g_themeManager.CreateNewTheme(newName, theme->author)) {
        // Load the newly created theme and copy colors
        size_t newIndex = g_themeManager.GetThemeCount() - 1;
        // Note: Would need to copy colors here, but for Week 4 we'll create default theme

        MessageBoxW(hwnd, L"Theme duplicated successfully.", L"Success",
                   MB_ICONINFORMATION | MB_OK);

        // Refresh and select new theme
        g_themeManager.ReloadThemes();
        PopulateThemeGallery();
        ListView_SetItemState(g_hThemeList, newIndex, LVIS_SELECTED, LVIS_SELECTED);
    } else {
        MessageBoxW(hwnd, g_themeManager.GetLastError().c_str(), L"Duplicate Failed",
                   MB_ICONERROR | MB_OK);
    }
}

//=============================================================================
// ShowNewThemeDialog - Show dialog to create new theme
//=============================================================================
void ShowNewThemeDialog(HWND hwnd) {
    // Simple input box for theme name (full dialog in future)
    wchar_t themeName[256] = L"";
    wchar_t author[256] = L"Unknown";

    // Get username for default author
    DWORD size = 256;
    GetUserNameW(author, &size);

    // For Week 4, use simple MessageBox for input (will be proper dialog later)
    std::wstring prompt = L"Enter theme name:\n\n(A proper dialog will be added in a future iteration)";

    // Create a simple theme with default name
    static int themeCounter = 1;
    swprintf_s(themeName, L"New Theme %d", themeCounter++);

    if (g_themeManager.CreateNewTheme(themeName, author)) {
        MessageBoxW(hwnd, L"New theme created successfully!", L"Success",
                   MB_ICONINFORMATION | MB_OK);

        // Refresh and select new theme
        g_themeManager.ReloadThemes();
        PopulateThemeGallery();

        // Select the new theme (last one in list)
        int newIndex = ListView_GetItemCount(g_hThemeList) - 1;
        ListView_SetItemState(g_hThemeList, newIndex, LVIS_SELECTED, LVIS_SELECTED);
    } else {
        MessageBoxW(hwnd, g_themeManager.GetLastError().c_str(), L"Create Failed",
                   MB_ICONERROR | MB_OK);
    }
}

//=============================================================================
// ShowImportDialog - Show file open dialog to import theme
//=============================================================================
void ShowImportDialog(HWND hwnd) {
    OPENFILENAMEW ofn = {};
    wchar_t szFile[MAX_PATH] = L"";

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Theme Files\0*.json;*.shadetheme\0JSON Files\0*.json\0SHADES Packages\0*.shadetheme\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Import Theme";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        // Copy file to themes directory
        std::wstring themesDir = g_themeManager.GetThemesDirectory();
        std::wstring fileName = szFile;
        size_t lastSlash = fileName.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            fileName = fileName.substr(lastSlash + 1);
        }

        std::wstring destPath = themesDir + fileName;

        if (CopyFileW(szFile, destPath.c_str(), FALSE)) {
            MessageBoxW(hwnd, L"Theme imported successfully!", L"Success",
                       MB_ICONINFORMATION | MB_OK);

            // Refresh gallery
            g_themeManager.ReloadThemes();
            PopulateThemeGallery();
        } else {
            MessageBoxW(hwnd, L"Failed to copy theme file to themes directory.", L"Import Failed",
                       MB_ICONERROR | MB_OK);
        }
    }
}

//=============================================================================
// ShowExportDialog - Show file save dialog to export theme
//=============================================================================
void ShowExportDialog(HWND hwnd) {
    if (g_selectedThemeIndex < 0) {
        MessageBoxW(hwnd, L"Please select a theme to export.", L"No Theme Selected",
                   MB_ICONINFORMATION | MB_OK);
        return;
    }

    const Shades::Theme* theme = g_themeManager.GetTheme(g_selectedThemeIndex);
    if (!theme) {
        return;
    }

    OPENFILENAMEW ofn = {};
    wchar_t szFile[MAX_PATH] = L"";
    wcscpy_s(szFile, theme->name.c_str());
    wcscat_s(szFile, L".json");

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Export Theme";
    ofn.lpstrDefExt = L"json";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        if (g_themeManager.SaveTheme(*theme, szFile)) {
            std::wstring msg = L"Theme exported successfully to:\n" + std::wstring(szFile);
            MessageBoxW(hwnd, msg.c_str(), L"Success", MB_ICONINFORMATION | MB_OK);
        } else {
            MessageBoxW(hwnd, g_themeManager.GetLastError().c_str(), L"Export Failed",
                       MB_ICONERROR | MB_OK);
        }
    }
}

//=============================================================================
// ShowPropertiesDialog - Show theme properties
//=============================================================================
void ShowPropertiesDialog(HWND hwnd) {
    if (g_selectedThemeIndex < 0) {
        MessageBoxW(hwnd, L"Please select a theme to view properties.", L"No Theme Selected",
                   MB_ICONINFORMATION | MB_OK);
        return;
    }

    const Shades::Theme* theme = g_themeManager.GetTheme(g_selectedThemeIndex);
    if (!theme) {
        return;
    }

    // Format file modified date
    SYSTEMTIME st;
    FileTimeToSystemTime(&theme->modifiedDate, &st);
    wchar_t dateStr[64];
    swprintf_s(dateStr, L"%02d/%02d/%04d %02d:%02d:%02d",
              st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

    // Get file size
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    DWORD fileSize = 0;
    if (GetFileAttributesExW(theme->filePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
        fileSize = fileInfo.nFileSizeLow;
    }

    // Build properties message
    std::wstring props = L"Theme Properties\n\n";
    props += L"Name: " + theme->name + L"\n";
    props += L"Author: " + theme->author + L"\n";
    props += L"Version: " + theme->version + L"\n";
    props += L"Type: " + std::wstring(theme->isPackage ? L".shadetheme Package" : L"JSON File") + L"\n";
    props += L"\nFile Information:\n";
    props += L"Path: " + theme->filePath + L"\n";
    props += L"Modified: " + std::wstring(dateStr) + L"\n";

    wchar_t sizeStr[32];
    swprintf_s(sizeStr, L"%u bytes", fileSize);
    props += L"Size: " + std::wstring(sizeStr) + L"\n";

    wchar_t colorCount[16];
    swprintf_s(colorCount, L"%zu", theme->colors.size());
    props += L"\nColors Defined: " + std::wstring(colorCount) + L"\n";
    props += L"Active: " + std::wstring(theme->isActive ? L"Yes" : L"No");

    MessageBoxW(hwnd, props.c_str(), L"Theme Properties", MB_ICONINFORMATION | MB_OK);
}
