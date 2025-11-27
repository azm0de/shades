#ifndef SHADES_COLOR_EDITOR_H
#define SHADES_COLOR_EDITOR_H

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <functional>
#include "ColorUtils.h"
#include "../theme_manager/ThemeManager.h"

namespace Shades {

//=============================================================================
// Color Property Definition
//=============================================================================

struct ColorProperty {
    std::wstring label;         // Display name (e.g., "Window Background")
    std::string key;            // Theme key (e.g., "window_bg")
    std::wstring tooltip;       // Tooltip description
    COLORREF currentColor;      // Current color value

    // Control handles
    HWND hPreview;              // Color preview square (custom control)
    HWND hHexInput;             // Hex input textbox
    HWND hPickerBtn;            // Picker button

    // Layout coordinates
    int y;                      // Y position in scrollable area

    ColorProperty(const std::wstring& lbl, const std::string& k, const std::wstring& tip)
        : label(lbl), key(k), tooltip(tip), currentColor(RGB(0, 0, 0)),
          hPreview(NULL), hHexInput(NULL), hPickerBtn(NULL), y(0) {}
};

//=============================================================================
// ColorEditor Class - Manages the color property editor panel
//=============================================================================

class ColorEditor {
public:
    ColorEditor();
    ~ColorEditor();

    // Window creation and management
    bool Create(HWND hwndParent, int x, int y, int width, int height);
    void Destroy();
    HWND GetHandle() const { return m_hwnd; }

    // Theme binding
    void LoadTheme(const Theme* theme);
    void ClearTheme();
    bool HasUnsavedChanges() const { return m_isDirty; }
    void SaveChanges();

    // Color updates
    void SetColor(const std::string& propertyKey, COLORREF color);
    COLORREF GetColor(const std::string& propertyKey) const;

    // Callbacks
    using ColorChangeCallback = std::function<void(const std::string&, COLORREF)>;
    void SetColorChangeCallback(ColorChangeCallback callback) {
        m_colorChangeCallback = callback;
    }

    // Window procedure
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Initialization
    void RegisterWindowClass();
    void CreateControls();
    void CreateColorProperty(ColorProperty& prop, int index);

    // Event handlers
    void OnSize(int width, int height);
    void OnScroll(int scrollCode, int pos);
    void OnPaint(HDC hdc);
    void OnHexInputChange(const std::string& propertyKey);
    void OnPreviewClick(const std::string& propertyKey);
    void OnPickerButtonClick(const std::string& propertyKey);

    // Drawing
    void DrawColorPreview(HDC hdc, const ColorProperty& prop);
    void DrawSectionHeader(HDC hdc, const std::wstring& text, int y);

    // Helpers
    ColorProperty* FindProperty(const std::string& key);
    void UpdatePreview(const std::string& propertyKey);
    void UpdateHexInput(const std::string& propertyKey);
    void MarkDirty();
    int GetScrollPos() const;
    void SetScrollPos(int pos);
    void UpdateScrollInfo();

private:
    HWND m_hwnd;                    // Main window handle
    HWND m_hwndParent;              // Parent window
    const Theme* m_currentTheme;    // Currently loaded theme
    bool m_isDirty;                 // Has unsaved changes

    // Color properties (13 total)
    std::vector<ColorProperty> m_properties;

    // Layout constants
    static const int LABEL_WIDTH = 150;
    static const int PREVIEW_WIDTH = 32;
    static const int PREVIEW_HEIGHT = 24;
    static const int HEX_INPUT_WIDTH = 80;
    static const int PICKER_BTN_WIDTH = 24;
    static const int ROW_HEIGHT = 32;
    static const int SECTION_HEADER_HEIGHT = 24;
    static const int LEFT_PADDING = 12;
    static const int CONTROL_SPACING = 8;

    // Scrolling
    int m_scrollPos;
    int m_totalHeight;

    // Fonts
    HFONT m_hFont;
    HFONT m_hBoldFont;

    // Callback
    ColorChangeCallback m_colorChangeCallback;

    // Class name
    static const wchar_t* CLASS_NAME;
    static bool s_classRegistered;
};

} // namespace Shades

#endif // SHADES_COLOR_EDITOR_H
