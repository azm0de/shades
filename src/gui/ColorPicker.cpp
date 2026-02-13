#include "ColorPicker.h"

namespace Shades {

ColorPicker::ColorPicker() {
    for (int i = 0; i < 16; i++) {
        m_customColors[i] = RGB(255, 255, 255);
    }
}

bool ColorPicker::ShowDialog(HWND hwndParent, COLORREF& color) {
    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(CHOOSECOLORW);
    cc.hwndOwner = hwndParent;
    cc.lpCustColors = m_customColors;
    cc.rgbResult = color;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorW(&cc)) {
        color = cc.rgbResult;
        return true;
    }
    return false;
}

COLORREF ColorPicker::Show(HWND hwndParent, COLORREF currentColor) {
    COLORREF color = currentColor;
    if (ShowDialog(hwndParent, color)) {
        return color;
    }
    return currentColor;
}

} // namespace Shades
