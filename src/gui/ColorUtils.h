#ifndef SHADES_COLORUTILS_H
#define SHADES_COLORUTILS_H

#include <windows.h>
#include <string>

namespace Shades {
namespace ColorUtils {

    // Convert hex string "#RRGGBB" to COLORREF (narrow string version)
    COLORREF HexToColorRef(const std::string& hex);

    // Convert hex wstring L"#RRGGBB" to COLORREF, returns true on success
    bool HexToColorRef(const std::wstring& hex, COLORREF& outColor);

    // Convert COLORREF to hex string "#RRGGBB" (narrow)
    std::string ColorRefToHex(COLORREF color);

    // Convert COLORREF to hex wstring L"#RRGGBB" (wide)
    std::wstring ColorRefToHexW(COLORREF color);

    // Validate hex color format ("#RRGGBB" or "RRGGBB")
    bool IsValidHexColor(const std::string& hex);

    // Linearly interpolate between two colors (t: 0.0 = a, 1.0 = b)
    COLORREF LerpColor(COLORREF a, COLORREF b, double t);

} // namespace ColorUtils
} // namespace Shades

#endif // SHADES_COLORUTILS_H
