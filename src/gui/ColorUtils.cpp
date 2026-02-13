#include "ColorUtils.h"
#include <cstdio>
#include <algorithm>

namespace Shades {
namespace ColorUtils {

COLORREF HexToColorRef(const std::string& hex) {
    if (!IsValidHexColor(hex)) {
        return RGB(0, 0, 0);
    }

    const char* p = hex.c_str();
    if (*p == '#') p++;

    unsigned int r = 0, g = 0, b = 0;
    sscanf(p, "%02x%02x%02x", &r, &g, &b);
    return RGB(r, g, b);
}

bool HexToColorRef(const std::wstring& hex, COLORREF& outColor) {
    if (hex.empty()) return false;

    // Convert wide string to narrow for parsing
    std::string narrow;
    narrow.reserve(hex.size());
    for (wchar_t wc : hex) {
        narrow.push_back(static_cast<char>(wc));
    }

    if (!IsValidHexColor(narrow)) return false;

    outColor = HexToColorRef(narrow);
    return true;
}

std::string ColorRefToHex(COLORREF color) {
    char buf[8];
    sprintf(buf, "#%02X%02X%02X",
            GetRValue(color), GetGValue(color), GetBValue(color));
    return std::string(buf);
}

std::wstring ColorRefToHexW(COLORREF color) {
    wchar_t buf[8];
    swprintf(buf, 8, L"#%02X%02X%02X",
             GetRValue(color), GetGValue(color), GetBValue(color));
    return std::wstring(buf);
}

bool IsValidHexColor(const std::string& hex) {
    if (hex.empty()) return false;

    size_t start = 0;
    if (hex[0] == '#') start = 1;

    if (hex.length() - start != 6) return false;

    for (size_t i = start; i < hex.length(); i++) {
        char c = hex[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
            return false;
    }
    return true;
}

COLORREF LerpColor(COLORREF a, COLORREF b, double t) {
    if (t <= 0.0) return a;
    if (t >= 1.0) return b;
    int r = (int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t);
    int g = (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t);
    int bl = (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t);
    return RGB(r, g, bl);
}

} // namespace ColorUtils
} // namespace Shades
