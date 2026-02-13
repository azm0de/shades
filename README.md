# SHADES - Event Viewer Dark Theme

> A lightweight Windows utility that applies customizable dark themes to Windows Event Viewer using DLL injection and API hooking.

[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/azm0de/shades)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-v1.2-orange.svg)](https://github.com/azm0de/shades/releases)

---

## What is SHADES?

SHADES transforms Windows Event Viewer from its default bright theme into a customizable dark theme -- without modifying any system files. The theme is applied dynamically at runtime via DLL injection and can be toggled on/off instantly from the system tray.

**Highlights:**
- Dark theme for Windows Event Viewer (eventvwr.msc)
- System tray icon with toggle, configure, and exit options
- Hot-reload: edit `theme.json` and see changes instantly
- Fully customizable colors via JSON
- CLI flags for scripting (`--config`, `--disable`, `--status`, `--silent`)
- Completely reversible -- no system files modified

---

## Quick Start

### Requirements
- Windows 10/11 or Server 2019/2022 (64-bit)
- Administrator privileges (required for DLL injection)

### Portable Install
1. Download `EventViewerThemer.zip` from [Releases](https://github.com/azm0de/shades/releases)
2. Extract to any folder
3. Right-click `SHADES.exe` -> "Run as administrator"
4. If Event Viewer isn't running, SHADES will prompt you to launch it
5. The tray icon appears -- right-click it for options

### Installer
1. Download `EventViewerThemer-Setup.exe` from [Releases](https://github.com/azm0de/shades/releases)
2. Run the installer and follow the wizard
3. Launch from Start Menu -> "Event Viewer Themer"

### Usage

**System Tray:** SHADES runs in the system tray. Right-click the icon to:
- Toggle the theme on/off
- Open the Theme Configurator
- View About info
- Exit

**Hot-Reload:** Edit `theme.json` while SHADES is running -- changes apply automatically within 500ms.

**Disable:** Right-click tray icon -> Exit, or run `SHADES.exe --disable`.

---

## Command-Line Arguments

```
SHADES.exe [options]
```

| Argument | Description |
|----------|-------------|
| `--help`, `-h` | Display help |
| `--version`, `-v` | Display version |
| `--status` | Check if theme is active |
| `--silent`, `-s` | Run without console output |
| `--config <path>` | Use a custom theme.json file |
| `--disable` | Shut down the running SHADES instance |

**Automation example:**
```cmd
schtasks /create /tn "SHADES" /tr "C:\Program Files\EventViewerThemer\SHADES.exe --silent" /sc onlogon /rl highest
```

---

## Customization

Edit `theme.json` to change colors:

```json
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
```

Changes are hot-reloaded automatically when the file is saved.

**Preset themes** (copy into your `theme.json`):

<details>
<summary>Dracula</summary>

```json
{
  "colors": {
    "window_bg": "#282A36", "window_text": "#F8F8F2",
    "highlight_bg": "#44475A", "highlight_text": "#F8F8F2",
    "button_face": "#44475A", "button_text": "#F8F8F2",
    "header_bg": "#6272A4"
  }
}
```
</details>

<details>
<summary>Nord</summary>

```json
{
  "colors": {
    "window_bg": "#2E3440", "window_text": "#ECEFF4",
    "highlight_bg": "#3B4252", "highlight_text": "#ECEFF4",
    "button_face": "#4C566A", "button_text": "#D8DEE9",
    "header_bg": "#5E81AC"
  }
}
```
</details>

<details>
<summary>Monokai</summary>

```json
{
  "colors": {
    "window_bg": "#272822", "window_text": "#F8F8F2",
    "highlight_bg": "#49483E", "highlight_text": "#F8F8F2",
    "button_face": "#3E3D32", "button_text": "#F8F8F2",
    "header_bg": "#75715E"
  }
}
```
</details>

---

## How It Works

SHADES uses a two-component architecture:

**ThemeEngine.dll** -- Injected into `mmc.exe` (Event Viewer). Hooks Windows APIs via Microsoft Detours:
- `GetSysColor` / `GetSysColorBrush` -- Custom system colors
- `DrawThemeBackground` -- Custom themed control drawing
- `DrawTextW` / `DrawTextExW` -- Text color overrides
- `CreateWindowExW` / `SetWindowTextW` -- Window creation hooks
- Window subclassing for ListView, TreeView, TabControl, and parent windows
- `NM_CUSTOMDRAW` handling via parent window subclass for ListView row theming
- 500ms polling timer for hot-reload via file timestamp checking

**SHADES.exe** -- The user-facing injector:
- Finds `mmc.exe` with "Event Viewer" window title
- Injects `ThemeEngine.dll` via `VirtualAllocEx` + `CreateRemoteThread` + `LoadLibraryA`
- Creates a named mutex (`Global\EventViewerThemeActive`) for theme on/off state
- Runs a system tray icon with toggle/configure/exit menu
- Supports `--config` (custom theme path via environment variable) and `--disable` (shuts down running instance)

---

## Building from Source

### Prerequisites
- MSVC 2019+ (Visual Studio Build Tools or full IDE)
- Windows SDK
- Git

Dependencies are included: Microsoft Detours (`libs/detours/`), nlohmann/json (`libs/json/`).

### Build Commands

```cmd
git clone https://github.com/azm0de/shades.git
cd shades

# Build ThemeEngine.dll
build_theme_engine.bat

# Build SHADES.exe
build_injector.bat

# Build GUI (v2.0-alpha)
test_build_gui.bat
```

Build scripts auto-detect Visual Studio 2019/2022 installations.

### Project Structure

```
shades/
+-- Injector/              # DLL injector + system tray
|   +-- Injector.cpp
|   +-- CMakeLists.txt
+-- ThemeEngine/           # Theming engine DLL
|   +-- ThemeEngine.cpp
|   +-- CMakeLists.txt
+-- src/gui/               # GUI v2.0 (color editor, preview, theme gallery)
|   +-- main.cpp
|   +-- ColorEditor.cpp/h
|   +-- PreviewPanel.cpp/h
|   +-- ButtonBar.cpp/h
|   +-- ColorPicker.cpp/h
|   +-- ColorUtils.cpp/h
|   +-- commands.h
|   +-- version.h
+-- src/theme_manager/     # Theme loading/saving
|   +-- ThemeManager.cpp/h
+-- Configurator/          # Standalone theme configurator
+-- libs/                  # Third-party (Detours, nlohmann/json)
+-- dist/                  # Distribution binaries
+-- theme.json             # Default theme
+-- installer.nsi          # NSIS installer script
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Injection failed" | Run SHADES.exe as Administrator |
| Theme not applying | Ensure `theme.json` and `ThemeEngine.dll` are next to `SHADES.exe` |
| Colors wrong | Use `#RRGGBB` hex format in `theme.json` |
| Antivirus blocks it | Add an exception -- DLL injection triggers heuristic detection (false positive) |
| Multiple Event Viewer windows | SHADES targets the first found instance |

---

## FAQ

**Q: Does this work on Windows 7/8?**
Not tested. Targets Windows 10+ (64-bit).

**Q: Can I theme other applications?**
Currently Event Viewer only. The architecture could be adapted for other MMC snap-ins.

**Q: Will Windows updates break it?**
Unlikely -- uses stable Win32 APIs. Major architecture changes could theoretically affect it.

**Q: Is there a GUI?**
Yes -- a v2.0 GUI with a visual color editor, live preview panel, and theme gallery is included (`test_build_gui.bat` to build).

---

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make changes and test on Windows 10/11
4. Open a Pull Request

Report issues at [GitHub Issues](https://github.com/azm0de/shades/issues).

---

## License

MIT License. See [LICENSE](LICENSE).

Third-party: Microsoft Detours (MIT), nlohmann/json (MIT).

---

**Version:** 1.2 | **Last Updated:** February 2026

Made by [@azm0de](https://github.com/azm0de)

[![GitHub stars](https://img.shields.io/github/stars/azm0de/shades.svg?style=social&label=Star)](https://github.com/azm0de/shades)
