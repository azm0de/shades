#ifndef SHADES_COLORPICKER_H
#define SHADES_COLORPICKER_H

#include <windows.h>

namespace Shades {

class ColorPicker {
public:
    ColorPicker();

    // Show the color picker dialog. Returns true if user selected a color.
    // On success, 'color' is updated with the chosen value.
    bool ShowDialog(HWND hwndParent, COLORREF& color);

    // Show the color picker and return selected color.
    // Returns the new color if user clicked OK, or currentColor if cancelled.
    COLORREF Show(HWND hwndParent, COLORREF currentColor);

private:
    COLORREF m_customColors[16];
};

} // namespace Shades

#endif // SHADES_COLORPICKER_H
