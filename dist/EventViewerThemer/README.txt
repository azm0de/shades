========================================
SHADES - Event Viewer Dark Theme v1.2
========================================

Apply a customizable dark theme to Windows Event Viewer.

FILES:
------
  SHADES.exe        - Main application (run this as Administrator)
  ThemeEngine.dll   - Theming engine (must be in same folder)
  theme.json        - Color configuration (editable)

HOW TO USE:
-----------
1. Right-click SHADES.exe -> "Run as administrator"
2. If Event Viewer isn't open, SHADES will offer to launch it
3. The dark theme applies immediately
4. A system tray icon appears -- right-click it for options

SYSTEM TRAY:
------------
  - Toggle Theme: enable/disable the theme
  - Configure: open the theme configurator
  - Exit: shut down SHADES and restore default appearance

HOT-RELOAD:
-----------
Edit theme.json while SHADES is running. Changes apply
automatically within 500ms -- no restart needed.

COMMAND LINE:
-------------
  SHADES.exe --help         Show help
  SHADES.exe --status       Check if theme is active
  SHADES.exe --silent       Run without console output
  SHADES.exe --config path  Use a custom theme.json
  SHADES.exe --disable      Shut down running SHADES instance

CUSTOMIZE COLORS:
-----------------
Edit theme.json with any text editor. Use #RRGGBB hex format:

{
  "colors": {
    "window_bg": "#1E1E1E",
    "window_text": "#D4D4D4",
    "highlight_bg": "#2A2D2E",
    "highlight_text": "#FFFFFF",
    "button_face": "#333333",
    "button_text": "#D4D4D4",
    "header_bg": "#333333"
  }
}

TROUBLESHOOTING:
----------------
  - "Could not open process" -> Run as Administrator
  - "No mmc.exe process found" -> Open Event Viewer first
  - Theme not applying -> Ensure all 3 files are in the same folder

Project: https://github.com/azm0de/shades
License: MIT
