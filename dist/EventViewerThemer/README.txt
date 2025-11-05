========================================
Event Viewer Dark Theme - Quick Start
========================================

This package contains everything you need to apply a dark theme to Windows Event Viewer.

FILES INCLUDED:
---------------
1. Injector.exe      - Main application (run this)
2. ThemeEngine.dll   - Theming engine (must be in same folder)
3. theme.json        - Color configuration (customizable)
4. README.txt        - This file

HOW TO USE:
-----------
1. Extract all files to a folder (keep them together!)
2. Open Windows Event Viewer first
   - Press Windows Key and type "Event Viewer"
   - Or run: eventvwr.msc
3. Right-click Injector.exe → "Run as administrator"
4. The dark theme will apply immediately!

IMPORTANT:
----------
- Both Injector.exe and ThemeEngine.dll must be in the same folder
- theme.json must also be in the same folder
- Always run Injector.exe as Administrator
- Keep the Injector window open to maintain the theme
- Close the Injector window to disable the theme

CUSTOMIZE COLORS:
-----------------
Edit theme.json to change colors. Use hex color codes:

{
  "colors": {
    "window_bg": "#1E1E1E",      // Main background
    "window_text": "#D4D4D4",    // Text color
    "highlight_bg": "#2A2D2E",   // Selection background
    "highlight_text": "#FFFFFF",  // Selection text
    "button_face": "#333333",    // Button color
    "button_text": "#D4D4D4",    // Button text
    "header_bg": "#333333"       // Column headers
  }
}

TROUBLESHOOTING:
----------------
- "Could not open process" → Run as Administrator
- "No mmc.exe process found" → Open Event Viewer first
- Theme not applying → Check all 3 files are in same folder
- Theme disappears → Keep Injector.exe window open

TECHNICAL DETAILS:
------------------
- Built: November 2025
- Architecture: x64 (64-bit Windows)
- Injection Method: CreateRemoteThread + LoadLibrary
- Hooking Library: Microsoft Detours
- Compiler: MinGW-w64 GCC 13

Project: https://github.com/azm0de/shades
