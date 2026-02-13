#include "PreviewPanel.h"
#include <windowsx.h>
#include <commctrl.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

namespace Shades {

// Static members
const wchar_t* PreviewPanel::CLASS_NAME = L"ShadesPreviewPanel";
bool PreviewPanel::s_classRegistered = false;

// Timer IDs
#define TIMER_ANIMATION 1001

//=============================================================================
// Constructor / Destructor
//=============================================================================

PreviewPanel::PreviewPanel()
    : m_hwnd(NULL)
    , m_hwndParent(NULL)
    , m_hwndTooltip(NULL)
    , m_currentTheme(NULL)
    , m_gdiplusToken(0)
    , m_backBuffer(NULL)
    , m_backBufferGraphics(NULL)
    , m_zoomLevel(1.0)
    , m_clientWidth(0)
    , m_clientHeight(0)
    , m_isAnimating(false)
    , m_animationTimer(0)
    , m_hoveredRegion(NULL)
    , m_animationStartTime(0)
{
}

PreviewPanel::~PreviewPanel() {
    ShutdownGDIPlus();
}

//=============================================================================
// Window Creation
//=============================================================================

void PreviewPanel::RegisterWindowClass() {
    if (s_classRegistered) return;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0x1E, 0x1E, 0x1E));
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);
    s_classRegistered = true;
}

bool PreviewPanel::Create(HWND hwndParent, int x, int y, int width, int height) {
    m_hwndParent = hwndParent;
    m_clientWidth = width;
    m_clientHeight = height;

    RegisterWindowClass();
    InitializeGDIPlus();

    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Preview Panel",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        this
    );

    if (!m_hwnd) {
        return false;
    }

    // Create tooltip
    m_hwndTooltip = CreateWindowExW(
        0,
        TOOLTIPS_CLASSW,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        m_hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Set tooltip delay
    SendMessage(m_hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 10000);

    // Create back buffer
    if (width > 0 && height > 0) {
        m_backBuffer = new Bitmap(width, height, PixelFormat32bppARGB);
        m_backBufferGraphics = Graphics::FromImage(m_backBuffer);
        m_backBufferGraphics->SetSmoothingMode(SmoothingModeAntiAlias);
        m_backBufferGraphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    }

    return true;
}

void PreviewPanel::Destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

//=============================================================================
// GDI+ Initialization
//=============================================================================

void PreviewPanel::InitializeGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

void PreviewPanel::ShutdownGDIPlus() {
    if (m_backBufferGraphics) {
        delete m_backBufferGraphics;
        m_backBufferGraphics = NULL;
    }
    if (m_backBuffer) {
        delete m_backBuffer;
        m_backBuffer = NULL;
    }
    if (m_gdiplusToken) {
        GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
    }
}

//=============================================================================
// Theme Binding
//=============================================================================

void PreviewPanel::LoadTheme(const Theme* theme) {
    m_currentTheme = theme;
    BuildHitTestRegions();
    Render();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void PreviewPanel::ClearTheme() {
    m_currentTheme = NULL;
    m_hitTestRegions.clear();
    Render();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void PreviewPanel::OnColorChanged(const std::string& propertyKey, COLORREF newColor) {
    if (!m_currentTheme) return;

    // Start animation to new color
    m_targetColors[propertyKey] = newColor;
    m_animationStartTime = GetTickCount();

    if (!m_isAnimating) {
        m_isAnimating = true;
        SetTimer(m_hwnd, TIMER_ANIMATION, 33, NULL);  // ~30 FPS
    }
}

//=============================================================================
// Zoom Control
//=============================================================================

void PreviewPanel::SetZoom(double zoomPercent) {
    m_zoomLevel = max(0.5, min(2.0, zoomPercent));
    Render();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

//=============================================================================
// Event Handlers
//=============================================================================

void PreviewPanel::OnPaint(HDC hdc) {
    if (m_backBuffer) {
        Graphics g(hdc);
        g.DrawImage(m_backBuffer, 0, 0);
    }
}

void PreviewPanel::OnSize(int width, int height) {
    m_clientWidth = width;
    m_clientHeight = height;

    // Recreate back buffer
    if (m_backBufferGraphics) {
        delete m_backBufferGraphics;
        m_backBufferGraphics = NULL;
    }
    if (m_backBuffer) {
        delete m_backBuffer;
        m_backBuffer = NULL;
    }

    if (width > 0 && height > 0) {
        m_backBuffer = new Bitmap(width, height, PixelFormat32bppARGB);
        m_backBufferGraphics = Graphics::FromImage(m_backBuffer);
        m_backBufferGraphics->SetSmoothingMode(SmoothingModeAntiAlias);
        m_backBufferGraphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        Render();
    }
}

void PreviewPanel::OnMouseMove(int x, int y) {
    HitTestRegion* region = HitTest(x, y);

    if (region != m_hoveredRegion) {
        m_hoveredRegion = region;

        if (region) {
            // Show tooltip
            COLORREF color = GetThemeColor(region->propertyKey);
            std::wstring tooltip = region->displayName + L": " + ColorUtils::ColorRefToHexW(color);
            SetCursor(LoadCursor(NULL, IDC_HAND));
        } else {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void PreviewPanel::OnLButtonDown(int x, int y) {
    HitTestRegion* region = HitTest(x, y);
    if (region && m_propertySelectedCallback) {
        m_propertySelectedCallback(region->propertyKey);
    }
}

void PreviewPanel::OnMouseLeave() {
    m_hoveredRegion = NULL;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void PreviewPanel::OnTimer(UINT_PTR timerId) {
    if (timerId == TIMER_ANIMATION) {
        DWORD elapsed = GetTickCount() - m_animationStartTime;

        if (elapsed >= ANIMATION_DURATION) {
            // Animation complete
            m_isAnimating = false;
            KillTimer(m_hwnd, TIMER_ANIMATION);
            m_animatingColors.clear();
            m_targetColors.clear();
        }

        Render();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

//=============================================================================
// Rendering
//=============================================================================

void PreviewPanel::Render() {
    if (!m_backBufferGraphics) return;

    // Clear background
    m_backBufferGraphics->Clear(GdipColor(RGB(0x1E, 0x1E, 0x1E)));

    if (!m_currentTheme) {
        // Draw "No theme selected" message
        SolidBrush textBrush(Color(128, 204, 204, 204));
        Font font(L"Segoe UI", 14, FontStyleRegular, UnitPixel);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        RectF bounds(0, 0, (REAL)m_clientWidth, (REAL)m_clientHeight);
        m_backBufferGraphics->DrawString(
            L"Select a theme to preview",
            -1,
            &font,
            bounds,
            &format,
            &textBrush
        );
        return;
    }

    // Calculate centered mockup bounds with zoom
    REAL mockupW = MOCKUP_WIDTH * m_zoomLevel;
    REAL mockupH = MOCKUP_HEIGHT * m_zoomLevel;
    REAL mockupX = (m_clientWidth - mockupW) / 2;
    REAL mockupY = (m_clientHeight - mockupH) / 2;

    // Apply zoom transform
    m_backBufferGraphics->TranslateTransform(mockupX, mockupY);
    m_backBufferGraphics->ScaleTransform((REAL)m_zoomLevel, (REAL)m_zoomLevel);

    // Draw mockup
    RectF mockupBounds(0, 0, (REAL)MOCKUP_WIDTH, (REAL)MOCKUP_HEIGHT);
    DrawMockup(m_backBufferGraphics);

    // Reset transform
    m_backBufferGraphics->ResetTransform();

    // Draw hover highlight
    if (m_hoveredRegion) {
        Pen highlightPen(Color(200, 0, 122, 204), 2);
        RECT rc = m_hoveredRegion->bounds;
        m_backBufferGraphics->DrawRectangle(&highlightPen, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top);
    }
}

void PreviewPanel::DrawMockup(Graphics* g) {
    REAL y = 0;

    // Menubar
    RectF menubarBounds(0, y, MOCKUP_WIDTH, MENUBAR_HEIGHT);
    DrawMenubar(g, menubarBounds);
    y += MENUBAR_HEIGHT;

    // Toolbar
    RectF toolbarBounds(0, y, MOCKUP_WIDTH, TOOLBAR_HEIGHT);
    DrawToolbar(g, toolbarBounds);
    y += TOOLBAR_HEIGHT;

    // Content area (tree + list)
    REAL contentHeight = MOCKUP_HEIGHT - y - STATUSBAR_HEIGHT;

    // Tree view (left)
    RectF treeBounds(0, y, TREE_WIDTH, contentHeight);
    DrawTreeView(g, treeBounds);

    // Vertical scrollbar for tree
    RectF treeScrollBounds(TREE_WIDTH, y, SCROLLBAR_WIDTH, contentHeight);
    DrawScrollbar(g, treeScrollBounds, true);

    // List view (right)
    REAL listX = TREE_WIDTH + SCROLLBAR_WIDTH;
    REAL listW = MOCKUP_WIDTH - listX - SCROLLBAR_WIDTH;
    RectF listBounds(listX, y, listW, contentHeight);
    DrawListView(g, listBounds);

    // Vertical scrollbar for list
    RectF listScrollBounds(MOCKUP_WIDTH - SCROLLBAR_WIDTH, y, SCROLLBAR_WIDTH, contentHeight);
    DrawScrollbar(g, listScrollBounds, true);

    y += contentHeight;

    // Statusbar
    RectF statusBounds(0, y, MOCKUP_WIDTH, STATUSBAR_HEIGHT);
    DrawStatusbar(g, statusBounds);
}

void PreviewPanel::DrawMenubar(Graphics* g, const RectF& bounds) {
    Color bg = GdipColor(GetThemeColor("menubar_bg"));
    Color fg = GdipColor(GetThemeColor("menubar_text"));

    SolidBrush bgBrush(bg);
    g->FillRectangle(&bgBrush, bounds);

    // Draw menu items
    Font font(L"Segoe UI", 9, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(fg);

    const wchar_t* menuItems[] = {L"File", L"Action", L"View", L"Help"};
    REAL x = 8;

    for (int i = 0; i < 4; i++) {
        PointF pt(x, bounds.Y + 4);
        g->DrawString(menuItems[i], -1, &font, pt, &textBrush);
        x += 50;
    }
}

void PreviewPanel::DrawToolbar(Graphics* g, const RectF& bounds) {
    Color bg = GdipColor(GetThemeColor("header_bg"));

    SolidBrush bgBrush(bg);
    g->FillRectangle(&bgBrush, bounds);

    // Draw separator line
    Pen separatorPen(Color(100, 60, 60, 60));
    g->DrawLine(&separatorPen, bounds.X, bounds.GetBottom() - 1,
        bounds.GetRight(), bounds.GetBottom() - 1);

    // Draw toolbar icon placeholders
    REAL x = 8;
    for (int i = 0; i < 4; i++) {
        RectF iconBounds(x, bounds.Y + 6, 20, 20);
        Pen iconPen(GdipColor(GetThemeColor("window_text"), 150));
        g->DrawRectangle(&iconPen, iconBounds);
        x += 28;
    }
}

void PreviewPanel::DrawTreeView(Graphics* g, const RectF& bounds) {
    Color bg = GdipColor(GetThemeColor("window_bg"));
    Color fg = GdipColor(GetThemeColor("window_text"));
    Color selectedBg = GdipColor(GetThemeColor("highlight_bg"));
    Color selectedFg = GdipColor(GetThemeColor("highlight_text"));

    SolidBrush bgBrush(bg);
    g->FillRectangle(&bgBrush, bounds);

    Font font(L"Segoe UI", 9, FontStyleRegular, UnitPixel);
    SolidBrush normalBrush(fg);
    SolidBrush selectedBrush(selectedFg);

    const wchar_t* items[] = {L"\u25BC Event Viewer", L"  \u25B6 Windows Logs", L"  \u25CF System", L"  \u25B6 Application", L"  \u25B6 Security"};
    REAL y = bounds.Y + 4;
    REAL lineHeight = 18;

    for (int i = 0; i < 5; i++) {
        RectF itemBounds(bounds.X, y, bounds.Width, lineHeight);

        if (i == 2) {  // "System" is selected
            SolidBrush selBgBrush(selectedBg);
            g->FillRectangle(&selBgBrush, itemBounds);
            g->DrawString(items[i], -1, &font, PointF(bounds.X + 4, y + 2), &selectedBrush);
        } else {
            g->DrawString(items[i], -1, &font, PointF(bounds.X + 4, y + 2), &normalBrush);
        }

        y += lineHeight;
    }
}

void PreviewPanel::DrawListView(Graphics* g, const RectF& bounds) {
    Color bg = GdipColor(GetThemeColor("window_bg"));
    Color fg = GdipColor(GetThemeColor("window_text"));
    Color headerBg = GdipColor(GetThemeColor("header_bg"));
    Color selectedBg = GdipColor(GetThemeColor("highlight_bg"));
    Color selectedFg = GdipColor(GetThemeColor("highlight_text"));

    SolidBrush bgBrush(bg);
    g->FillRectangle(&bgBrush, bounds);

    // Draw header
    REAL headerHeight = 20;
    RectF headerBounds(bounds.X, bounds.Y, bounds.Width, headerHeight);
    SolidBrush headerBrush(headerBg);
    g->FillRectangle(&headerBrush, headerBounds);

    Font headerFont(L"Segoe UI", 9, FontStyleBold, UnitPixel);
    SolidBrush headerTextBrush(fg);

    const wchar_t* headers[] = {L"Name", L"Level", L"Date/Time"};
    REAL colWidths[] = {120, 60, 100};
    REAL x = bounds.X + 4;

    for (int i = 0; i < 3; i++) {
        g->DrawString(headers[i], -1, &headerFont, PointF(x, bounds.Y + 2), &headerTextBrush);
        x += colWidths[i];
    }

    // Draw rows
    Font font(L"Segoe UI", 9, FontStyleRegular, UnitPixel);
    SolidBrush normalBrush(fg);
    SolidBrush selectedBrush(selectedFg);

    const wchar_t* rows[][3] = {
        {L"Application", L"Info", L"11/27/2025 2:15 PM"},
        {L"Application", L"Warning", L"11/27/2025 2:14 PM"},
        {L"System", L"Error", L"11/27/2025 2:13 PM"},
        {L"System", L"Info", L"11/27/2025 2:12 PM"},
        {L"Application", L"Info", L"11/27/2025 2:11 PM"}
    };

    REAL y = bounds.Y + headerHeight + 2;
    REAL rowHeight = 18;

    for (int i = 0; i < 5; i++) {
        RectF rowBounds(bounds.X, y, bounds.Width, rowHeight);

        if (i == 1) {  // Second row selected
            SolidBrush selBgBrush(selectedBg);
            g->FillRectangle(&selBgBrush, rowBounds);

            x = bounds.X + 4;
            for (int col = 0; col < 3; col++) {
                g->DrawString(rows[i][col], -1, &font, PointF(x, y + 2), &selectedBrush);
                x += colWidths[col];
            }
        } else {
            x = bounds.X + 4;
            for (int col = 0; col < 3; col++) {
                g->DrawString(rows[i][col], -1, &font, PointF(x, y + 2), &normalBrush);
                x += colWidths[col];
            }
        }

        y += rowHeight;
    }
}

void PreviewPanel::DrawScrollbar(Graphics* g, const RectF& bounds, bool vertical) {
    Color trackBg = GdipColor(GetThemeColor("scrollbar_bg"));
    Color thumbColor = GdipColor(GetThemeColor("scrollbar_thumb"));

    SolidBrush trackBrush(trackBg);
    g->FillRectangle(&trackBrush, bounds);

    // Draw thumb
    RectF thumbBounds;
    if (vertical) {
        thumbBounds = RectF(bounds.X + 2, bounds.Y + 20, bounds.Width - 4, 40);
    } else {
        thumbBounds = RectF(bounds.X + 20, bounds.Y + 2, 40, bounds.Height - 4);
    }

    SolidBrush thumbBrush(thumbColor);
    g->FillRectangle(&thumbBrush, thumbBounds);
}

void PreviewPanel::DrawStatusbar(Graphics* g, const RectF& bounds) {
    Color bg = GdipColor(GetThemeColor("statusbar_bg"));
    Color fg = GdipColor(GetThemeColor("statusbar_text"));

    SolidBrush bgBrush(bg);
    g->FillRectangle(&bgBrush, bounds);

    Font font(L"Segoe UI", 9, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(fg);

    g->DrawString(L"5 items", -1, &font, PointF(bounds.X + 4, bounds.Y + 4), &textBrush);
}

//=============================================================================
// Helpers
//=============================================================================

Color PreviewPanel::GdipColor(COLORREF color, BYTE alpha) {
    return Color(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF PreviewPanel::GetThemeColor(const std::string& key) const {
    if (!m_currentTheme) {
        return RGB(128, 128, 128);  // Gray fallback
    }

    // Check if animating
    auto targetIt = m_targetColors.find(key);
    if (targetIt != m_targetColors.end()) {
        DWORD elapsed = GetTickCount() - m_animationStartTime;
        double t = min(1.0, elapsed / (double)ANIMATION_DURATION);

        COLORREF current = m_currentTheme->GetColor(key, RGB(0, 0, 0));
        COLORREF target = targetIt->second;

        return ColorUtils::LerpColor(current, target, t);
    }

    return m_currentTheme->GetColor(key, RGB(128, 128, 128));
}

void PreviewPanel::BuildHitTestRegions() {
    m_hitTestRegions.clear();

    if (!m_currentTheme) return;

    // Calculate mockup position with zoom
    REAL mockupW = MOCKUP_WIDTH * m_zoomLevel;
    REAL mockupH = MOCKUP_HEIGHT * m_zoomLevel;
    int mockupX = (m_clientWidth - mockupW) / 2;
    int mockupY = (m_clientHeight - mockupH) / 2;

    auto addRegion = [&](int x, int y, int w, int h, const std::string& key, const std::wstring& name) {
        RECT rc;
        rc.left = mockupX + x * m_zoomLevel;
        rc.top = mockupY + y * m_zoomLevel;
        rc.right = rc.left + w * m_zoomLevel;
        rc.bottom = rc.top + h * m_zoomLevel;
        m_hitTestRegions.push_back(HitTestRegion(rc, key, name));
    };

    int y = 0;

    // Menubar
    addRegion(0, y, MOCKUP_WIDTH, MENUBAR_HEIGHT, "menubar_bg", L"Menubar Background");
    y += MENUBAR_HEIGHT;

    // Toolbar
    addRegion(0, y, MOCKUP_WIDTH, TOOLBAR_HEIGHT, "header_bg", L"Header Background");
    y += TOOLBAR_HEIGHT;

    // Tree view
    int contentH = MOCKUP_HEIGHT - y - STATUSBAR_HEIGHT;
    addRegion(0, y, TREE_WIDTH, contentH, "window_bg", L"Window Background (Tree)");
    addRegion(TREE_WIDTH, y, SCROLLBAR_WIDTH, contentH, "scrollbar_bg", L"Scrollbar Track");

    // List view
    int listX = TREE_WIDTH + SCROLLBAR_WIDTH;
    int listW = MOCKUP_WIDTH - listX - SCROLLBAR_WIDTH;
    addRegion(listX, y, listW, contentH, "window_bg", L"Window Background (List)");
    addRegion(MOCKUP_WIDTH - SCROLLBAR_WIDTH, y, SCROLLBAR_WIDTH, contentH, "scrollbar_bg", L"Scrollbar Track");

    y += contentH;

    // Statusbar
    addRegion(0, y, MOCKUP_WIDTH, STATUSBAR_HEIGHT, "statusbar_bg", L"Statusbar Background");
}

HitTestRegion* PreviewPanel::HitTest(int x, int y) {
    for (auto& region : m_hitTestRegions) {
        if (x >= region.bounds.left && x < region.bounds.right &&
            y >= region.bounds.top && y < region.bounds.bottom) {
            return &region;
        }
    }
    return nullptr;
}

//=============================================================================
// Window Procedure
//=============================================================================

LRESULT CALLBACK PreviewPanel::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    PreviewPanel* pThis = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PreviewPanel*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PreviewPanel*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT PreviewPanel::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            OnPaint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_LBUTTONDOWN:
            OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MOUSELEAVE:
            OnMouseLeave();
            return 0;

        case WM_TIMER:
            OnTimer(wParam);
            return 0;

        case WM_ERASEBKGND:
            return 1;  // Prevent flicker
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace Shades
