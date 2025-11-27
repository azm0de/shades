#include "ButtonBar.h"
#include "commands.h"
#include <windowsx.h>
#include <commctrl.h>

namespace Shades {

//=============================================================================
// Static members
//=============================================================================
const wchar_t* ButtonBar::CLASS_NAME = L"ShadesButtonBar";
bool ButtonBar::s_classRegistered = false;

//=============================================================================
// Constructor
//=============================================================================
ButtonBar::ButtonBar()
    : m_hwnd(NULL)
    , m_hwndParent(NULL)
    , m_clientWidth(0)
    , m_clientHeight(0)
    , m_hoveredButton(nullptr)
    , m_pressedButton(nullptr)
{
    // Initialize button definitions
    m_buttons[0] = ButtonInfo(L"Apply Theme", ButtonStyle::Primary, ID_TOOLS_APPLY);
    m_buttons[1] = ButtonInfo(L"Save Theme", ButtonStyle::Secondary, ID_FILE_SAVE);
    m_buttons[2] = ButtonInfo(L"\u2699 Settings", ButtonStyle::Secondary, ID_TOOLS_SETTINGS);  // ⚙ gear icon
    m_buttons[3] = ButtonInfo(L"? Help", ButtonStyle::Secondary, ID_HELP_DOCUMENTATION);
}

//=============================================================================
// Destructor
//=============================================================================
ButtonBar::~ButtonBar() {
    Destroy();
}

//=============================================================================
// Create
//=============================================================================
bool ButtonBar::Create(HWND hwndParent, int x, int y, int width, int height) {
    m_hwndParent = hwndParent;

    RegisterWindowClass();

    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"",
        WS_CHILD | WS_VISIBLE,
        x, y, width, height,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        this  // Pass 'this' pointer for WM_CREATE
    );

    if (!m_hwnd) {
        return false;
    }

    m_clientWidth = width;
    m_clientHeight = height;

    CreateButtons();
    LayoutButtons();

    return true;
}

//=============================================================================
// Destroy
//=============================================================================
void ButtonBar::Destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

//=============================================================================
// RegisterWindowClass
//=============================================================================
void ButtonBar::RegisterWindowClass() {
    if (s_classRegistered) {
        return;
    }

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BACKGROUND);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);
    s_classRegistered = true;
}

//=============================================================================
// StaticWndProc
//=============================================================================
LRESULT CALLBACK ButtonBar::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ButtonBar* pThis = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<ButtonBar*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<ButtonBar*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//=============================================================================
// WndProc
//=============================================================================
LRESULT ButtonBar::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            OnPaint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            OnSize(width, height);
            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnMouseMove(x, y);

            // Track mouse leave
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            return 0;
        }

        case WM_MOUSELEAVE: {
            OnMouseLeave();
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnLButtonDown(x, y);
            SetCapture(hwnd);
            return 0;
        }

        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnLButtonUp(x, y);
            ReleaseCapture();
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;  // We handle background in WM_PAINT
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//=============================================================================
// CreateButtons
//=============================================================================
void ButtonBar::CreateButtons() {
    // Button HWNDs are not needed since we're owner-drawing
    // Everything is handled in OnPaint
}

//=============================================================================
// LayoutButtons
//=============================================================================
void ButtonBar::LayoutButtons() {
    // Calculate button widths
    const int BUTTON_WIDTHS[4] = { 120, 120, 110, 80 };  // Apply, Save, Settings, Help

    // Calculate total width
    int totalWidth = 0;
    for (int i = 0; i < 4; i++) {
        totalWidth += BUTTON_WIDTHS[i];
    }
    totalWidth += BUTTON_SPACING * 3;  // 3 gaps between 4 buttons

    // Calculate starting X (centered)
    int startX = (m_clientWidth - totalWidth) / 2;
    int y = (m_clientHeight - BUTTON_HEIGHT) / 2;

    // Set button bounds
    int x = startX;
    for (int i = 0; i < 4; i++) {
        RECT rc;
        rc.left = x;
        rc.top = y;
        rc.right = x + BUTTON_WIDTHS[i];
        rc.bottom = y + BUTTON_HEIGHT;

        m_buttons[i].hwnd = (HWND)(INT_PTR)(i + 1);  // Fake HWND for identification
        ::SetRect(&rc, x, y, x + BUTTON_WIDTHS[i], y + BUTTON_HEIGHT);

        // Store bounds in user data (we'll use a local array)
        // For now, we'll calculate bounds on the fly in hit testing

        x += BUTTON_WIDTHS[i] + BUTTON_SPACING;
    }
}

//=============================================================================
// OnPaint
//=============================================================================
void ButtonBar::OnPaint(HDC hdc) {
    // Draw background
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);

    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BACKGROUND);
    FillRect(hdc, &rcClient, hBrushBg);
    DeleteObject(hBrushBg);

    // Draw top border
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_TOP);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, 0, 0, NULL);
    LineTo(hdc, rcClient.right, 0);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);

    // Draw buttons
    for (int i = 0; i < 4; i++) {
        DrawButton(hdc, m_buttons[i]);
    }
}

//=============================================================================
// DrawButton
//=============================================================================
void ButtonBar::DrawButton(HDC hdc, const ButtonInfo& button) {
    // Calculate button bounds
    const int BUTTON_WIDTHS[4] = { 120, 120, 110, 80 };
    int buttonIndex = -1;
    for (int i = 0; i < 4; i++) {
        if (&m_buttons[i] == &button) {
            buttonIndex = i;
            break;
        }
    }
    if (buttonIndex < 0) return;

    int totalWidth = 0;
    for (int i = 0; i < 4; i++) {
        totalWidth += BUTTON_WIDTHS[i];
    }
    totalWidth += BUTTON_SPACING * 3;

    int startX = (m_clientWidth - totalWidth) / 2;
    int y = (m_clientHeight - BUTTON_HEIGHT) / 2;
    int x = startX;
    for (int i = 0; i < buttonIndex; i++) {
        x += BUTTON_WIDTHS[i] + BUTTON_SPACING;
    }

    RECT rc;
    rc.left = x;
    rc.top = y;
    rc.right = x + BUTTON_WIDTHS[buttonIndex];
    rc.bottom = y + BUTTON_HEIGHT;

    // Get colors based on state
    COLORREF bgColor = GetButtonBackgroundColor(button);
    COLORREF textColor = GetButtonTextColor(button);
    COLORREF borderColor = GetButtonBorderColor(button);

    // Draw background
    HBRUSH hBrush = CreateSolidBrush(bgColor);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Draw border for secondary buttons or pressed state
    if (button.style == ButtonStyle::Secondary || button.state == ButtonState::Pressed) {
        HPEN hPen = CreatePen(PS_SOLID, 1, borderColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }

    // Draw text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);

    HFONT hFont = CreateFontW(
        -14,  // Height
        0,    // Width
        0,    // Escapement
        0,    // Orientation
        button.style == ButtonStyle::Primary ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    DrawTextW(hdc, button.text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

//=============================================================================
// GetButtonBackgroundColor
//=============================================================================
COLORREF ButtonBar::GetButtonBackgroundColor(const ButtonInfo& button) const {
    if (!button.enabled) {
        return COLOR_DISABLED_BG;
    }

    if (button.style == ButtonStyle::Primary) {
        switch (button.state) {
            case ButtonState::Pressed: return COLOR_PRIMARY_BG_PRESSED;
            case ButtonState::Hover:   return COLOR_PRIMARY_BG_HOVER;
            default:                   return COLOR_PRIMARY_BG;
        }
    } else {
        switch (button.state) {
            case ButtonState::Pressed: return COLOR_SECONDARY_BG_PRESSED;
            case ButtonState::Hover:   return COLOR_SECONDARY_BG_HOVER;
            default:                   return COLOR_SECONDARY_BG;
        }
    }
}

//=============================================================================
// GetButtonTextColor
//=============================================================================
COLORREF ButtonBar::GetButtonTextColor(const ButtonInfo& button) const {
    if (!button.enabled) {
        return COLOR_DISABLED_TEXT;
    }

    if (button.style == ButtonStyle::Primary) {
        return COLOR_PRIMARY_TEXT;
    } else {
        return COLOR_SECONDARY_TEXT;
    }
}

//=============================================================================
// GetButtonBorderColor
//=============================================================================
COLORREF ButtonBar::GetButtonBorderColor(const ButtonInfo& button) const {
    if (!button.enabled) {
        return COLOR_DISABLED_BORDER;
    }

    if (button.style == ButtonStyle::Secondary) {
        return COLOR_SECONDARY_BORDER;
    }

    return COLOR_PRIMARY_BG;
}

//=============================================================================
// OnSize
//=============================================================================
void ButtonBar::OnSize(int width, int height) {
    m_clientWidth = width;
    m_clientHeight = height;
    LayoutButtons();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

//=============================================================================
// OnMouseMove
//=============================================================================
void ButtonBar::OnMouseMove(int x, int y) {
    UpdateButtonStates(x, y);
}

//=============================================================================
// OnMouseLeave
//=============================================================================
void ButtonBar::OnMouseLeave() {
    ResetButtonStates();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

//=============================================================================
// OnLButtonDown
//=============================================================================
void ButtonBar::OnLButtonDown(int x, int y) {
    ButtonInfo* button = HitTestButton(x, y);
    if (button && button->enabled) {
        m_pressedButton = button;
        button->state = ButtonState::Pressed;
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

//=============================================================================
// OnLButtonUp
//=============================================================================
void ButtonBar::OnLButtonUp(int x, int y) {
    if (m_pressedButton) {
        ButtonInfo* button = HitTestButton(x, y);
        if (button == m_pressedButton && button->enabled) {
            // Button clicked!
            if (m_buttonClickCallback) {
                m_buttonClickCallback(button->commandID);
            }
        }

        m_pressedButton->state = ButtonState::Normal;
        m_pressedButton = nullptr;

        UpdateButtonStates(x, y);
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

//=============================================================================
// HitTestButton
//=============================================================================
ButtonInfo* ButtonBar::HitTestButton(int x, int y) {
    const int BUTTON_WIDTHS[4] = { 120, 120, 110, 80 };

    int totalWidth = 0;
    for (int i = 0; i < 4; i++) {
        totalWidth += BUTTON_WIDTHS[i];
    }
    totalWidth += BUTTON_SPACING * 3;

    int startX = (m_clientWidth - totalWidth) / 2;
    int buttonY = (m_clientHeight - BUTTON_HEIGHT) / 2;
    int buttonX = startX;

    for (int i = 0; i < 4; i++) {
        RECT rc;
        rc.left = buttonX;
        rc.top = buttonY;
        rc.right = buttonX + BUTTON_WIDTHS[i];
        rc.bottom = buttonY + BUTTON_HEIGHT;

        if (x >= rc.left && x < rc.right && y >= rc.top && y < rc.bottom) {
            return &m_buttons[i];
        }

        buttonX += BUTTON_WIDTHS[i] + BUTTON_SPACING;
    }

    return nullptr;
}

//=============================================================================
// UpdateButtonStates
//=============================================================================
void ButtonBar::UpdateButtonStates(int mouseX, int mouseY) {
    ButtonInfo* hovered = HitTestButton(mouseX, mouseY);

    bool stateChanged = false;
    if (hovered != m_hoveredButton) {
        stateChanged = true;
    }

    // Reset all buttons to normal (except pressed button)
    for (int i = 0; i < 4; i++) {
        if (&m_buttons[i] != m_pressedButton) {
            m_buttons[i].state = ButtonState::Normal;
        }
    }

    // Set hovered state
    if (hovered && hovered != m_pressedButton) {
        hovered->state = ButtonState::Hover;
        SetCursor(LoadCursor(NULL, IDC_HAND));
    } else {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }

    m_hoveredButton = hovered;

    if (stateChanged) {
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

//=============================================================================
// ResetButtonStates
//=============================================================================
void ButtonBar::ResetButtonStates() {
    for (int i = 0; i < 4; i++) {
        m_buttons[i].state = ButtonState::Normal;
    }
    m_hoveredButton = nullptr;
    m_pressedButton = nullptr;
}

//=============================================================================
// EnableButton
//=============================================================================
void ButtonBar::EnableButton(int commandID, bool enable) {
    for (int i = 0; i < 4; i++) {
        if (m_buttons[i].commandID == commandID) {
            m_buttons[i].enabled = enable;
            InvalidateRect(m_hwnd, NULL, FALSE);
            return;
        }
    }
}

//=============================================================================
// IsButtonEnabled
//=============================================================================
bool ButtonBar::IsButtonEnabled(int commandID) const {
    for (int i = 0; i < 4; i++) {
        if (m_buttons[i].commandID == commandID) {
            return m_buttons[i].enabled;
        }
    }
    return false;
}

//=============================================================================
// HandleAccelerator
//=============================================================================
bool ButtonBar::HandleAccelerator(UINT vKey, bool ctrlPressed) {
    if (!ctrlPressed) {
        // F1 for Help
        if (vKey == VK_F1) {
            if (m_buttonClickCallback && m_buttons[3].enabled) {
                m_buttonClickCallback(ID_HELP_DOCUMENTATION);
                return true;
            }
        }
        return false;
    }

    // Ctrl+A for Apply
    if (vKey == 'A' && m_buttons[0].enabled) {
        if (m_buttonClickCallback) {
            m_buttonClickCallback(ID_TOOLS_APPLY);
        }
        return true;
    }

    // Ctrl+S for Save
    if (vKey == 'S' && m_buttons[1].enabled) {
        if (m_buttonClickCallback) {
            m_buttonClickCallback(ID_FILE_SAVE);
        }
        return true;
    }

    return false;
}

} // namespace Shades
