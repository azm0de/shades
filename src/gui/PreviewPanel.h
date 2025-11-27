#ifndef SHADES_PREVIEW_PANEL_H
#define SHADES_PREVIEW_PANEL_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <map>
#include <functional>
#include "ColorUtils.h"
#include "../theme_manager/ThemeManager.h"

namespace Shades {

//=============================================================================
// HitTestRegion - Maps screen rectangles to theme color properties
//=============================================================================
struct HitTestRegion {
    RECT bounds;
    std::string propertyKey;
    std::wstring displayName;

    HitTestRegion() : bounds({0, 0, 0, 0}), propertyKey(""), displayName(L"") {}
    HitTestRegion(const RECT& rc, const std::string& key, const std::wstring& name)
        : bounds(rc), propertyKey(key), displayName(name) {}
};

//=============================================================================
// PreviewPanel - Live preview of Event Viewer with current theme
//=============================================================================
class PreviewPanel {
public:
    PreviewPanel();
    ~PreviewPanel();

    // Window creation
    bool Create(HWND hwndParent, int x, int y, int width, int height);
    void Destroy();
    HWND GetHandle() const { return m_hwnd; }

    // Theme binding
    void LoadTheme(const Theme* theme);
    void ClearTheme();
    void OnColorChanged(const std::string& propertyKey, COLORREF newColor);

    // Zoom control
    void SetZoom(double zoomPercent);  // 0.5 to 2.0 (50% to 200%)
    double GetZoom() const { return m_zoomLevel; }

    // Callbacks
    using ColorPropertySelectedCallback = std::function<void(const std::string&)>;
    void SetPropertySelectedCallback(ColorPropertySelectedCallback callback) {
        m_propertySelectedCallback = callback;
    }

    // Window procedure
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Initialization
    void RegisterWindowClass();
    void InitializeGDIPlus();
    void ShutdownGDIPlus();
    void BuildHitTestRegions();

    // Event handlers
    void OnPaint(HDC hdc);
    void OnSize(int width, int height);
    void OnMouseMove(int x, int y);
    void OnLButtonDown(int x, int y);
    void OnMouseLeave();
    void OnTimer(UINT_PTR timerId);

    // Rendering (double-buffered)
    void Render();
    void DrawMockup(Gdiplus::Graphics* g);

    // Mockup components
    void DrawMenubar(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds);
    void DrawToolbar(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds);
    void DrawTreeView(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds);
    void DrawListView(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds);
    void DrawScrollbar(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds, bool vertical);
    void DrawStatusbar(Gdiplus::Graphics* g, const Gdiplus::RectF& bounds);

    // Helpers
    Gdiplus::Color GdipColor(COLORREF color, BYTE alpha = 255);
    COLORREF GetThemeColor(const std::string& key) const;
    HitTestRegion* HitTest(int x, int y);
    void ShowTooltip(const std::wstring& text);
    void HideTooltip();

private:
    HWND m_hwnd;
    HWND m_hwndParent;
    HWND m_hwndTooltip;
    const Theme* m_currentTheme;

    // GDI+ resources
    ULONG_PTR m_gdiplusToken;
    Gdiplus::Bitmap* m_backBuffer;
    Gdiplus::Graphics* m_backBufferGraphics;

    // Rendering state
    double m_zoomLevel;
    int m_clientWidth;
    int m_clientHeight;
    bool m_isAnimating;
    UINT_PTR m_animationTimer;

    // Hit testing
    std::vector<HitTestRegion> m_hitTestRegions;
    HitTestRegion* m_hoveredRegion;

    // Callback
    ColorPropertySelectedCallback m_propertySelectedCallback;

    // Animation state
    std::map<std::string, COLORREF> m_animatingColors;
    std::map<std::string, COLORREF> m_targetColors;
    DWORD m_animationStartTime;
    static const DWORD ANIMATION_DURATION = 200;  // milliseconds

    // Mockup layout constants (in preview space, before zoom)
    static const int MOCKUP_WIDTH = 600;
    static const int MOCKUP_HEIGHT = 400;
    static const int MENUBAR_HEIGHT = 24;
    static const int TOOLBAR_HEIGHT = 32;
    static const int STATUSBAR_HEIGHT = 22;
    static const int TREE_WIDTH = 180;
    static const int SCROLLBAR_WIDTH = 16;

    // Class name
    static const wchar_t* CLASS_NAME;
    static bool s_classRegistered;
};

} // namespace Shades

#endif // SHADES_PREVIEW_PANEL_H
