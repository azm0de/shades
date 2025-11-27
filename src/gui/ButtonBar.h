#ifndef SHADES_BUTTON_BAR_H
#define SHADES_BUTTON_BAR_H

#include <windows.h>
#include <functional>
#include <string>

namespace Shades {

//=============================================================================
// ButtonStyle - Visual style for buttons
//=============================================================================
enum class ButtonStyle {
    Primary,    // Accent background with white text
    Secondary   // Outlined style with subtle background
};

//=============================================================================
// ButtonState - Current state of a button
//=============================================================================
enum class ButtonState {
    Normal,
    Hover,
    Pressed,
    Disabled
};

//=============================================================================
// ButtonInfo - Button definition and state
//=============================================================================
struct ButtonInfo {
    std::wstring text;
    ButtonStyle style;
    ButtonState state;
    HWND hwnd;
    int commandID;
    bool enabled;

    ButtonInfo()
        : text(L""), style(ButtonStyle::Secondary), state(ButtonState::Normal),
          hwnd(NULL), commandID(0), enabled(true) {}

    ButtonInfo(const std::wstring& txt, ButtonStyle sty, int cmdID)
        : text(txt), style(sty), state(ButtonState::Normal),
          hwnd(NULL), commandID(cmdID), enabled(true) {}
};

//=============================================================================
// ButtonBar - Action button bar at bottom of window
//=============================================================================
class ButtonBar {
public:
    ButtonBar();
    ~ButtonBar();

    // Window creation
    bool Create(HWND hwndParent, int x, int y, int width, int height);
    void Destroy();
    HWND GetHandle() const { return m_hwnd; }

    // Button state management
    void EnableButton(int commandID, bool enable);
    bool IsButtonEnabled(int commandID) const;

    // Callbacks for button actions
    using ButtonClickCallback = std::function<void(int commandID)>;
    void SetButtonClickCallback(ButtonClickCallback callback) {
        m_buttonClickCallback = callback;
    }

    // Keyboard shortcuts
    bool HandleAccelerator(UINT vKey, bool ctrlPressed);

    // Window procedure
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Initialization
    void RegisterWindowClass();
    void CreateButtons();

    // Event handlers
    void OnPaint(HDC hdc);
    void OnSize(int width, int height);
    void OnMouseMove(int x, int y);
    void OnMouseLeave();
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);

    // Rendering
    void DrawButton(HDC hdc, const ButtonInfo& button);
    COLORREF GetButtonBackgroundColor(const ButtonInfo& button) const;
    COLORREF GetButtonTextColor(const ButtonInfo& button) const;
    COLORREF GetButtonBorderColor(const ButtonInfo& button) const;

    // Hit testing
    ButtonInfo* HitTestButton(int x, int y);
    void UpdateButtonStates(int mouseX, int mouseY);
    void ResetButtonStates();

    // Layout
    void LayoutButtons();

private:
    HWND m_hwnd;
    HWND m_hwndParent;
    int m_clientWidth;
    int m_clientHeight;

    // Buttons (4 buttons: Apply, Save, Settings, Help)
    ButtonInfo m_buttons[4];
    ButtonInfo* m_hoveredButton;
    ButtonInfo* m_pressedButton;

    // Callback
    ButtonClickCallback m_buttonClickCallback;

    // Layout constants
    static const int BAR_HEIGHT = 50;
    static const int BUTTON_HEIGHT = 32;
    static const int BUTTON_SPACING = 12;
    static const int SIDE_PADDING = 20;

    // Colors (using Windows modern dark theme palette)
    static const COLORREF COLOR_BACKGROUND = RGB(0x1E, 0x1E, 0x1E);  // #1E1E1E
    static const COLORREF COLOR_BORDER_TOP = RGB(0x3F, 0x3F, 0x46);   // #3F3F46

    static const COLORREF COLOR_PRIMARY_BG = RGB(0x0E, 0x7A, 0x90);        // #0E7A90 (teal accent)
    static const COLORREF COLOR_PRIMARY_BG_HOVER = RGB(0x15, 0x95, 0xB0);  // Lighter teal
    static const COLORREF COLOR_PRIMARY_BG_PRESSED = RGB(0x0A, 0x5F, 0x73); // Darker teal
    static const COLORREF COLOR_PRIMARY_TEXT = RGB(0xFF, 0xFF, 0xFF);      // White

    static const COLORREF COLOR_SECONDARY_BG = RGB(0x27, 0x27, 0x2A);       // #27272A
    static const COLORREF COLOR_SECONDARY_BG_HOVER = RGB(0x3F, 0x3F, 0x46); // #3F3F46
    static const COLORREF COLOR_SECONDARY_BG_PRESSED = RGB(0x1E, 0x1E, 0x1E); // #1E1E1E
    static const COLORREF COLOR_SECONDARY_TEXT = RGB(0xE4, 0xE4, 0xE7);     // #E4E4E7
    static const COLORREF COLOR_SECONDARY_BORDER = RGB(0x52, 0x52, 0x5B);   // #52525B

    static const COLORREF COLOR_DISABLED_BG = RGB(0x1E, 0x1E, 0x1E);
    static const COLORREF COLOR_DISABLED_TEXT = RGB(0x71, 0x71, 0x7A);      // #71717A (gray)
    static const COLORREF COLOR_DISABLED_BORDER = RGB(0x3F, 0x3F, 0x46);

    // Class name
    static const wchar_t* CLASS_NAME;
    static bool s_classRegistered;
};

} // namespace Shades

#endif // SHADES_BUTTON_BAR_H
