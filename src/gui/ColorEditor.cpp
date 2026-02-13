#include "ColorEditor.h"
#include "ColorPicker.h"
#include <windowsx.h>

namespace Shades {

// Static members
const wchar_t* ColorEditor::CLASS_NAME = L"ShadesColorEditor";
bool ColorEditor::s_classRegistered = false;

// Control IDs
#define IDC_PREVIEW_BASE    3000
#define IDC_HEX_INPUT_BASE  3100
#define IDC_PICKER_BTN_BASE 3200

//=============================================================================
// Constructor / Destructor
//=============================================================================

ColorEditor::ColorEditor()
    : m_hwnd(NULL)
    , m_hwndParent(NULL)
    , m_currentTheme(NULL)
    , m_isDirty(false)
    , m_scrollPos(0)
    , m_totalHeight(0)
    , m_hFont(NULL)
    , m_hBoldFont(NULL)
{
    // Initialize 13 color properties in order
    // Section: Window
    m_properties.push_back(ColorProperty(L"Window Background", "window_bg", L"Main window background color"));
    m_properties.push_back(ColorProperty(L"Window Text", "window_text", L"Main window text color"));
    m_properties.push_back(ColorProperty(L"Highlight Background", "highlight_bg", L"Selected item background"));
    m_properties.push_back(ColorProperty(L"Highlight Text", "highlight_text", L"Selected item text"));

    // Section: Buttons
    m_properties.push_back(ColorProperty(L"Button Face", "button_face", L"Button background color"));
    m_properties.push_back(ColorProperty(L"Button Text", "button_text", L"Button text color"));

    // Section: Headers
    m_properties.push_back(ColorProperty(L"Header Background", "header_bg", L"Column header background"));

    // Section: Scrollbars
    m_properties.push_back(ColorProperty(L"Scrollbar Background", "scrollbar_bg", L"Scrollbar track color"));
    m_properties.push_back(ColorProperty(L"Scrollbar Thumb", "scrollbar_thumb", L"Scrollbar thumb color"));

    // Section: Menubar
    m_properties.push_back(ColorProperty(L"Menubar Background", "menubar_bg", L"Menu bar background"));
    m_properties.push_back(ColorProperty(L"Menubar Text", "menubar_text", L"Menu bar text color"));

    // Section: Statusbar
    m_properties.push_back(ColorProperty(L"Statusbar Background", "statusbar_bg", L"Status bar background"));
    m_properties.push_back(ColorProperty(L"Statusbar Text", "statusbar_text", L"Status bar text color"));
}

ColorEditor::~ColorEditor() {
    if (m_hFont) DeleteObject(m_hFont);
    if (m_hBoldFont) DeleteObject(m_hBoldFont);
}

//=============================================================================
// Window Creation
//=============================================================================

void ColorEditor::RegisterWindowClass() {
    if (s_classRegistered) return;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0x1E, 0x1E, 0x1E));  // Dark background
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);
    s_classRegistered = true;
}

bool ColorEditor::Create(HWND hwndParent, int x, int y, int width, int height) {
    m_hwndParent = hwndParent;

    RegisterWindowClass();

    // Create fonts
    m_hFont = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    m_hBoldFont = CreateFontW(
        -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    // Create window
    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Color Editor",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        x, y, width, height,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        this  // Pass 'this' pointer to WM_CREATE
    );

    if (!m_hwnd) {
        return false;
    }

    CreateControls();
    UpdateScrollInfo();

    return true;
}

void ColorEditor::Destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

//=============================================================================
// Control Creation
//=============================================================================

void ColorEditor::CreateControls() {
    int currentY = 20;  // Start position

    // Section indices for headers
    const int sectionStarts[] = {0, 4, 6, 7, 9, 11};  // Window, Buttons, Headers, Scrollbars, Menubar, Statusbar
    const wchar_t* sectionNames[] = {L"Window", L"Buttons", L"Headers", L"Scrollbars", L"Menubar", L"Statusbar"};
    int sectionIndex = 0;

    for (size_t i = 0; i < m_properties.size(); i++) {
        // Check if we need a section header
        if (sectionIndex < 6 && i == sectionStarts[sectionIndex]) {
            currentY += SECTION_HEADER_HEIGHT;
            sectionIndex++;
        }

        m_properties[i].y = currentY;
        CreateColorProperty(m_properties[i], static_cast<int>(i));
        currentY += ROW_HEIGHT;
    }

    m_totalHeight = currentY + 20;  // Add bottom padding
}

void ColorEditor::CreateColorProperty(ColorProperty& prop, int index) {
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);

    int x = LEFT_PADDING;
    int y = prop.y;

    // Label (Static text)
    CreateWindowW(
        L"STATIC",
        prop.label.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        x, y, LABEL_WIDTH, ROW_HEIGHT,
        m_hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    x += LABEL_WIDTH + CONTROL_SPACING;

    // Color Preview (will be owner-drawn)
    prop.hPreview = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
        x, y + (ROW_HEIGHT - PREVIEW_HEIGHT) / 2, PREVIEW_WIDTH, PREVIEW_HEIGHT,
        m_hwnd,
        (HMENU)(IDC_PREVIEW_BASE + index),
        GetModuleHandle(NULL),
        NULL
    );
    x += PREVIEW_WIDTH + CONTROL_SPACING;

    // Hex Input
    prop.hHexInput = CreateWindowW(
        L"EDIT",
        L"#000000",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_UPPERCASE | ES_AUTOHSCROLL,
        x, y + (ROW_HEIGHT - 22) / 2, HEX_INPUT_WIDTH, 22,
        m_hwnd,
        (HMENU)(IDC_HEX_INPUT_BASE + index),
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(prop.hHexInput, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(prop.hHexInput, EM_SETLIMITTEXT, 7, 0);  // #RRGGBB = 7 chars
    x += HEX_INPUT_WIDTH + CONTROL_SPACING;

    // Picker Button
    prop.hPickerBtn = CreateWindowW(
        L"BUTTON",
        L"\u25BC",  // Down arrow
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y + (ROW_HEIGHT - 22) / 2, PICKER_BTN_WIDTH, 22,
        m_hwnd,
        (HMENU)(IDC_PICKER_BTN_BASE + index),
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(prop.hPickerBtn, WM_SETFONT, (WPARAM)m_hFont, TRUE);
}

//=============================================================================
// Theme Binding
//=============================================================================

void ColorEditor::LoadTheme(const Theme* theme) {
    if (!theme) return;

    m_currentTheme = theme;
    m_isDirty = false;

    // Load all colors from theme
    for (auto& prop : m_properties) {
        prop.currentColor = theme->GetColor(prop.key, RGB(0, 0, 0));
        UpdateHexInput(prop.key);
        UpdatePreview(prop.key);
    }

    InvalidateRect(m_hwnd, NULL, TRUE);
}

void ColorEditor::ClearTheme() {
    m_currentTheme = NULL;
    m_isDirty = false;

    // Reset all colors to black
    for (auto& prop : m_properties) {
        prop.currentColor = RGB(0, 0, 0);
        UpdateHexInput(prop.key);
        UpdatePreview(prop.key);
    }

    InvalidateRect(m_hwnd, NULL, TRUE);
}

void ColorEditor::SaveChanges() {
    if (!m_currentTheme || !m_isDirty) return;

    // Update theme colors from editor properties
    Theme* mutableTheme = const_cast<Theme*>(m_currentTheme);
    for (const auto& prop : m_properties) {
        mutableTheme->colors[prop.key] = ColorUtils::ColorRefToHex(prop.currentColor);
    }

    m_isDirty = false;
}

//=============================================================================
// Color Updates
//=============================================================================

void ColorEditor::SetColor(const std::string& propertyKey, COLORREF color) {
    ColorProperty* prop = FindProperty(propertyKey);
    if (!prop) return;

    prop->currentColor = color;
    UpdateHexInput(propertyKey);
    UpdatePreview(propertyKey);
    MarkDirty();

    // Notify callback of color change
    if (m_colorChangeCallback) {
        m_colorChangeCallback(propertyKey, color);
    }
}

COLORREF ColorEditor::GetColor(const std::string& propertyKey) const {
    for (const auto& prop : m_properties) {
        if (prop.key == propertyKey) {
            return prop.currentColor;
        }
    }
    return RGB(0, 0, 0);
}

//=============================================================================
// Event Handlers
//=============================================================================

void ColorEditor::OnSize(int width, int height) {
    UpdateScrollInfo();
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void ColorEditor::OnScroll(int scrollCode, int pos) {
    int oldPos = m_scrollPos;
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    int pageSize = rcClient.bottom;

    switch (scrollCode) {
        case SB_LINEUP:   m_scrollPos -= ROW_HEIGHT; break;
        case SB_LINEDOWN: m_scrollPos += ROW_HEIGHT; break;
        case SB_PAGEUP:   m_scrollPos -= pageSize; break;
        case SB_PAGEDOWN: m_scrollPos += pageSize; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            m_scrollPos = pos;
            break;
    }

    // Clamp scroll position
    int maxScroll = max(0, m_totalHeight - rcClient.bottom);
    m_scrollPos = max(0, min(m_scrollPos, maxScroll));

    if (m_scrollPos != oldPos) {
        ::SetScrollPos(m_hwnd, SB_VERT, m_scrollPos, TRUE);
        ScrollWindowEx(m_hwnd, 0, oldPos - m_scrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
        UpdateWindow(m_hwnd);
    }
}

void ColorEditor::OnPaint(HDC hdc) {
    // Paint section headers
    const int sectionYPositions[] = {20, 20 + 4 * ROW_HEIGHT + SECTION_HEADER_HEIGHT,
                                     20 + 6 * ROW_HEIGHT + 2 * SECTION_HEADER_HEIGHT,
                                     20 + 7 * ROW_HEIGHT + 3 * SECTION_HEADER_HEIGHT,
                                     20 + 9 * ROW_HEIGHT + 4 * SECTION_HEADER_HEIGHT,
                                     20 + 11 * ROW_HEIGHT + 5 * SECTION_HEADER_HEIGHT};
    const wchar_t* sectionNames[] = {L"Window", L"Buttons", L"Headers", L"Scrollbars", L"Menubar", L"Statusbar"};

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0xCC, 0xCC, 0xCC));
    SelectObject(hdc, m_hBoldFont);

    for (int i = 0; i < 6; i++) {
        DrawSectionHeader(hdc, sectionNames[i], sectionYPositions[i] - SECTION_HEADER_HEIGHT - m_scrollPos);
    }
}

void ColorEditor::OnHexInputChange(const std::string& propertyKey) {
    ColorProperty* prop = FindProperty(propertyKey);
    if (!prop) return;

    // Get hex text from input
    wchar_t hexText[8] = {0};
    GetWindowTextW(prop->hHexInput, hexText, 8);
    std::wstring hex(hexText);

    // Validate and parse
    COLORREF newColor;
    if (ColorUtils::HexToColorRef(hex, newColor)) {
        // Valid hex - update color
        prop->currentColor = newColor;
        UpdatePreview(propertyKey);
        MarkDirty();

        // Remove error styling
        SendMessage(prop->hHexInput, EM_SETSEL, 0, 0);
    } else {
        // Invalid hex - show error (red text)
        // Note: Full error styling would require custom edit control
    }
}

void ColorEditor::OnPreviewClick(const std::string& propertyKey) {
    // Open color picker dialog
    ColorProperty* prop = FindProperty(propertyKey);
    if (!prop) return;

    ColorPicker picker;
    COLORREF selectedColor = picker.Show(m_hwnd, prop->currentColor);

    // If user clicked OK (returned color != original), update
    if (selectedColor != prop->currentColor) {
        SetColor(propertyKey, selectedColor);
    }
}

void ColorEditor::OnPickerButtonClick(const std::string& propertyKey) {
    // Same as preview click - open color picker
    OnPreviewClick(propertyKey);
}

//=============================================================================
// Drawing
//=============================================================================

void ColorEditor::DrawColorPreview(HDC hdc, const ColorProperty& prop) {
    if (!prop.hPreview) return;

    RECT rc;
    GetClientRect(prop.hPreview, &rc);

    // Fill with color
    HBRUSH hBrush = CreateSolidBrush(prop.currentColor);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Draw border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0x55, 0x55, 0x55));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void ColorEditor::DrawSectionHeader(HDC hdc, const std::wstring& text, int y) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    rc.top = y;
    rc.bottom = y + SECTION_HEADER_HEIGHT;
    rc.left = LEFT_PADDING;

    DrawTextW(hdc, text.c_str(), -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Draw separator line
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0x3E, 0x3E, 0x40));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, rc.left, rc.bottom - 4, NULL);
    LineTo(hdc, rc.right - LEFT_PADDING, rc.bottom - 4);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

//=============================================================================
// Helpers
//=============================================================================

ColorProperty* ColorEditor::FindProperty(const std::string& key) {
    for (auto& prop : m_properties) {
        if (prop.key == key) {
            return &prop;
        }
    }
    return nullptr;
}

void ColorEditor::UpdatePreview(const std::string& propertyKey) {
    ColorProperty* prop = FindProperty(propertyKey);
    if (!prop || !prop->hPreview) return;

    InvalidateRect(prop->hPreview, NULL, TRUE);
}

void ColorEditor::UpdateHexInput(const std::string& propertyKey) {
    ColorProperty* prop = FindProperty(propertyKey);
    if (!prop || !prop->hHexInput) return;

    std::wstring hex = ColorUtils::ColorRefToHexW(prop->currentColor);
    SetWindowTextW(prop->hHexInput, hex.c_str());
}

void ColorEditor::MarkDirty() {
    m_isDirty = true;
    // TODO: Notify parent window of dirty state
}

int ColorEditor::GetScrollPos() const {
    return m_scrollPos;
}

void ColorEditor::SetScrollPos(int pos) {
    m_scrollPos = pos;
}

void ColorEditor::UpdateScrollInfo() {
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);

    SCROLLINFO si = {};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = m_totalHeight;
    si.nPage = rcClient.bottom;
    si.nPos = m_scrollPos;

    SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
}

//=============================================================================
// Window Procedure
//=============================================================================

LRESULT CALLBACK ColorEditor::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ColorEditor* pThis = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<ColorEditor*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<ColorEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT ColorEditor::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_VSCROLL:
            OnScroll(LOWORD(wParam), HIWORD(wParam));
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            OnPaint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* pDIS = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
            int index = pDIS->CtlID - IDC_PREVIEW_BASE;
            if (index >= 0 && index < static_cast<int>(m_properties.size())) {
                DrawColorPreview(pDIS->hDC, m_properties[index]);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND: {
            WORD notifCode = HIWORD(wParam);
            WORD ctrlID = LOWORD(wParam);

            // Hex input change
            if (notifCode == EN_CHANGE && ctrlID >= IDC_HEX_INPUT_BASE && ctrlID < IDC_HEX_INPUT_BASE + 100) {
                int index = ctrlID - IDC_HEX_INPUT_BASE;
                if (index >= 0 && index < static_cast<int>(m_properties.size())) {
                    OnHexInputChange(m_properties[index].key);
                }
            }

            // Picker button click
            if (notifCode == BN_CLICKED && ctrlID >= IDC_PICKER_BTN_BASE && ctrlID < IDC_PICKER_BTN_BASE + 100) {
                int index = ctrlID - IDC_PICKER_BTN_BASE;
                if (index >= 0 && index < static_cast<int>(m_properties.size())) {
                    OnPickerButtonClick(m_properties[index].key);
                }
            }

            // Preview click (static control with SS_NOTIFY sends STN_CLICKED)
            if (notifCode == STN_CLICKED && ctrlID >= IDC_PREVIEW_BASE && ctrlID < IDC_PREVIEW_BASE + 100) {
                int index = ctrlID - IDC_PREVIEW_BASE;
                if (index >= 0 && index < static_cast<int>(m_properties.size())) {
                    OnPreviewClick(m_properties[index].key);
                }
            }
            break;
        }

        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            OnScroll(delta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace Shades
