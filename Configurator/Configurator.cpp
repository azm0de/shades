#include <windows.h>
#include <commdlg.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "../libs/json/json.hpp"

using json = nlohmann::json;

// Global variables
HWND g_hMainWindow = NULL;
std::map<std::string, COLORREF> g_currentColors;
std::map<std::string, HWND> g_colorButtons;
std::string g_themePath = "theme.json";

// Color definitions to edit
struct ColorDef {
    std::string key;
    std::string label;
};

std::vector<ColorDef> g_colorDefs = {
    {"window_bg", "Window Background"},
    {"window_text", "Window Text"},
    {"highlight_bg", "Highlight Background"},
    {"highlight_text", "Highlight Text"},
    {"button_face", "Button Face"},
    {"button_text", "Button Text"},
    {"header_bg", "Header Background"}
};

// Helper to convert hex string to COLORREF
COLORREF HexToColor(const std::string& hex) {
    int r, g, b;
    sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
    return RGB(r, g, b);
}

// Helper to convert COLORREF to hex string
std::string ColorToHex(COLORREF color) {
    char buffer[8];
    sprintf_s(buffer, 8, "#%02X%02X%02X", GetRValue(color), GetGValue(color), GetBValue(color));
    return std::string(buffer);
}

// Load theme from JSON
void LoadTheme() {
    std::ifstream f(g_themePath);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            for (const auto& def : g_colorDefs) {
                if (data["colors"].contains(def.key)) {
                    g_currentColors[def.key] = HexToColor(data["colors"][def.key]);
                }
            }
        } catch (...) {
            MessageBox(g_hMainWindow, "Error parsing theme.json", "Error", MB_ICONERROR);
        }
    } else {
        // Defaults if file missing
        g_currentColors["window_bg"] = RGB(30, 30, 30);
        g_currentColors["window_text"] = RGB(212, 212, 212);
        // ... others
    }
}

// Save theme to JSON
void SaveTheme() {
    json data;
    for (const auto& pair : g_currentColors) {
        data["colors"][pair.first] = ColorToHex(pair.second);
    }

    std::ofstream f(g_themePath);
    if (f.is_open()) {
        f << data.dump(4);
        MessageBox(g_hMainWindow, "Theme saved successfully!", "Success", MB_ICONINFORMATION);
    } else {
        MessageBox(g_hMainWindow, "Failed to save theme.json", "Error", MB_ICONERROR);
    }
}

// Choose Color Dialog
void PickColor(const std::string& key) {
    CHOOSECOLOR cc;                 // common dialog box structure 
    static COLORREF acrCustClr[16]; // array of custom colors 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = g_hMainWindow;
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = g_currentColors[key];
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc) == TRUE) {
        g_currentColors[key] = cc.rgbResult;
        InvalidateRect(g_colorButtons[key], NULL, TRUE); // Redraw button
    }
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        LoadTheme();
        
        int y = 20;
        for (const auto& def : g_colorDefs) {
            // Label
            CreateWindow("STATIC", def.label.c_str(), WS_VISIBLE | WS_CHILD, 
                20, y, 150, 25, hWnd, NULL, NULL, NULL);

            // Color Button (Owner drawn)
            HWND hBtn = CreateWindow("BUTTON", "", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 
                180, y, 50, 25, hWnd, (HMENU)(100 + &def - &g_colorDefs[0]), NULL, NULL);
            
            g_colorButtons[def.key] = hBtn;
            
            // Set tag or ID to map back to key? We used ID based on index.
            y += 40;
        }

        // Save Button
        CreateWindow("BUTTON", "Save Theme", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
            20, y + 20, 100, 30, hWnd, (HMENU)200, NULL, NULL);

        // Close Button
        CreateWindow("BUTTON", "Close", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
            140, y + 20, 100, 30, hWnd, (HMENU)201, NULL, NULL);
        
        break;
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        if (id >= 100 && id < 100 + g_colorDefs.size()) {
            // Color button clicked
            int index = id - 100;
            PickColor(g_colorDefs[index].key);
        }
        else if (id == 200) {
            SaveTheme();
        }
        else if (id == 201) {
            PostQuitMessage(0);
        }
        break;
    }
    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        if (pDIS->CtlType == ODT_BUTTON && pDIS->CtlID >= 100 && pDIS->CtlID < 100 + g_colorDefs.size()) {
            int index = pDIS->CtlID - 100;
            std::string key = g_colorDefs[index].key;
            
            HBRUSH hBrush = CreateSolidBrush(g_currentColors[key]);
            FillRect(pDIS->hDC, &pDIS->rcItem, hBrush);
            DeleteObject(hBrush);
            
            DrawEdge(pDIS->hDC, &pDIS->rcItem, EDGE_SUNKEN, BF_RECT);
            return TRUE;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "ThemeConfigurator";

    RegisterClassEx(&wc);

    g_hMainWindow = CreateWindow("ThemeConfigurator", "Theme Configurator", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 400, NULL, NULL, hInstance, NULL);

    ShowWindow(g_hMainWindow, nCmdShow);
    UpdateWindow(g_hMainWindow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
