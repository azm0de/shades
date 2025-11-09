# Event Viewer Themer

> A lightweight Windows utility that applies customizable dark themes to Windows Event Viewer using dynamic DLL injection and API hooking.

[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/azm0de/shades)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0-orange.svg)](https://github.com/azm0de/shades/releases)

---

## What is Event Viewer Themer?

Event Viewer Themer transforms Windows Event Viewer from its default bright theme into a beautiful, customizable dark theme - without modifying any system files. The theme is applied dynamically at runtime and can be enabled or disabled instantly.

**Key Highlights:**
- Dark theme for Windows Event Viewer (eventvwr.msc)
- Fully customizable colors via JSON configuration
- Completely reversible - no system file modifications
- Real-time toggle (enable/disable without restart)
- Lightweight (~2MB total size)

---

## Features

### For End Users
- **Easy to Use**: Just run `Injector.exe` and the theme applies instantly
- **Customizable Colors**: Edit `theme.json` to create your perfect color scheme
- **No Installation Required**: Standalone executable, no dependencies
- **Reversible**: Close the injector to restore original appearance
- **Safe**: No system files are modified or replaced

### For Developers
- **Clean C++ Codebase**: Well-structured, documented code
- **Modern Techniques**: Microsoft Detours for API hooking
- **Cross-Platform Build**: Supports Windows MSVC and Linux MinGW cross-compilation
- **Educational**: Learn about DLL injection, API hooking, and Windows internals
- **Open Source**: MIT licensed, free to use and modify

---

## Quick Start (For End Users)

### Requirements
- Windows 10, 11, or Server 2019/2022 (64-bit)
- Administrator privileges (required for DLL injection)
- Windows Event Viewer installed (included by default)

### Installation

1. **Download the latest release:**
   - Visit [Releases](https://github.com/azm0de/shades/releases)
   - Download `EventViewerThemer.zip`
   - Extract to any folder (e.g., `C:\Tools\EventViewerThemer\`)

2. **Run Event Viewer:**
   ```
   Press Win+R, type: eventvwr.msc
   ```

3. **Apply the theme:**
   - Right-click `Injector.exe`
   - Select "Run as administrator"
   - Keep the window open to maintain the theme

4. **Enjoy your themed Event Viewer!**

### Usage Tips

**Enable Theme:**
- Open Event Viewer first
- Run Injector.exe as Administrator
- Theme applies immediately

**Disable Theme:**
- Simply close the Injector.exe window
- Event Viewer returns to original appearance

**Keep Theme Active:**
- Leave Injector.exe running while using Event Viewer
- You can minimize the window to the taskbar

---

## Customization

### Editing Colors

The theme colors are defined in `theme.json`:

```json
{
  "colors": {
    "window_bg": "#1E1E1E",       // Main window background
    "window_text": "#D4D4D4",     // Text color
    "highlight_bg": "#2A2D2E",    // Selection background
    "highlight_text": "#FFFFFF",   // Selection text color
    "button_face": "#333333",      // Button background
    "button_text": "#D4D4D4",      // Button text
    "header_bg": "#333333"         // Column header background
  }
}
```

### Popular Color Schemes

**Dracula Theme:**
```json
{
  "colors": {
    "window_bg": "#282A36",
    "window_text": "#F8F8F2",
    "highlight_bg": "#44475A",
    "highlight_text": "#F8F8F2",
    "button_face": "#44475A",
    "button_text": "#F8F8F2",
    "header_bg": "#6272A4"
  }
}
```

**Nord Theme:**
```json
{
  "colors": {
    "window_bg": "#2E3440",
    "window_text": "#ECEFF4",
    "highlight_bg": "#3B4252",
    "highlight_text": "#ECEFF4",
    "button_face": "#4C566A",
    "button_text": "#D8DEE9",
    "header_bg": "#5E81AC"
  }
}
```

**Monokai Theme:**
```json
{
  "colors": {
    "window_bg": "#272822",
    "window_text": "#F8F8F2",
    "highlight_bg": "#49483E",
    "highlight_text": "#F8F8F2",
    "button_face": "#3E3D32",
    "button_text": "#F8F8F2",
    "header_bg": "#75715E"
  }
}
```

After editing `theme.json`, restart Injector.exe to apply changes.

---

## How It Works (Technical Overview)

Event Viewer Themer uses a two-component architecture:

### Component 1: ThemeEngine.dll (Theming Engine)

The core theming engine that modifies UI behavior through Windows API hooking:

**API Hooks Implemented:**
- `GetSysColor` - Returns custom colors for system color indices
- `GetSysColorBrush` - Returns custom brushes for backgrounds
- `DrawThemeBackground` - Intercepts theme drawing for headers

**Window Subclassing:**
- Subclasses the ListView control (SysListView32)
- Handles `WM_NOTIFY` messages with `NM_CUSTOMDRAW`
- Applies custom colors to individual list items and selections

**Configuration System:**
- Reads `theme.json` from disk using nlohmann/json
- Parses hex color codes (#1E1E1E) to Windows COLORREF format
- Stores colors in global configuration struct

### Component 2: Injector.exe (DLL Loader)

The user-facing application that injects ThemeEngine.dll into Event Viewer:

**Process Discovery:**
- Enumerates running processes using `CreateToolhelp32Snapshot`
- Identifies `mmc.exe` instances with "Event Viewer" window title
- Obtains target process ID

**DLL Injection:**
- Allocates memory in target process (`VirtualAllocEx`)
- Writes DLL path to target memory (`WriteProcessMemory`)
- Creates remote thread to load DLL (`CreateRemoteThread` + `LoadLibraryA`)

**IPC Toggle Mechanism:**
- Creates named mutex: `Global\EventViewerThemeActive`
- Mutex existence enables theming (checked by all hook functions)
- Mutex destroyed on exit, automatically disabling theme

### Why DLL Injection?

Traditional theming methods (like High Contrast themes or custom visual styles) either:
- Require system file modifications (risky, unsigned drivers)
- Affect all applications globally (not targeted)
- Don't support per-application customization

DLL injection allows us to:
- Target only Event Viewer process
- Make changes reversible (no system modifications)
- Provide granular control over UI elements
- Enable real-time toggling

---

## Building from Source (For Developers)

### Prerequisites

**Required Tools:**
- CMake 3.10 or higher
- C++ Compiler (MSVC 2019+ or MinGW-w64 GCC 13+)
- Windows SDK (for Windows API headers)
- Git

**Dependencies (Included):**
- Microsoft Detours 4.0+ (`libs/detours/`)
- nlohmann/json 3.x (`libs/json/json.hpp`)

### Build Instructions

#### Option 1: Windows with MSVC (Native)

```cmd
# Clone the repository
git clone https://github.com/azm0de/shades.git
cd shades

# Build ThemeEngine.dll
cd ThemeEngine
mkdir build && cd build
cmake ..
cmake --build . --config Release
cd ../..

# Build Injector.exe
cd Injector
mkdir build && cd build
cmake ..
cmake --build . --config Release
cd ../..

# Binaries will be in:
# - ThemeEngine/build/Release/ThemeEngine.dll
# - Injector/build/Release/Injector.exe
```

#### Option 2: Linux Cross-Compilation with MinGW

```bash
# Install MinGW-w64 toolchain
sudo apt-get install mingw-w64 cmake git

# Clone the repository
git clone https://github.com/azm0de/shades.git
cd shades

# Build ThemeEngine.dll
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-w64-toolchain.cmake \
      -S ThemeEngine -B ThemeEngine/build \
      -DCMAKE_BUILD_TYPE=Release
cmake --build ThemeEngine/build

# Build Injector.exe
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-w64-toolchain.cmake \
      -S Injector -B Injector/build \
      -DCMAKE_BUILD_TYPE=Release
cmake --build Injector/build

# Binaries will be in:
# - ThemeEngine/build/ThemeEngine.dll
# - Injector/build/Injector.exe
```

### Project Structure

```
shades/
├── Injector/                   # Phase 2: DLL Injector
│   ├── Injector.cpp           # Main injector logic (189 lines)
│   ├── CMakeLists.txt         # Build configuration
│   └── build/                 # Build artifacts
├── ThemeEngine/               # Phase 1: Theming Engine
│   ├── ThemeEngine.cpp        # Core theming hooks (202 lines)
│   ├── CMakeLists.txt         # Build configuration
│   └── build/                 # Build artifacts
├── libs/                      # Third-party libraries
│   ├── detours/               # Microsoft Detours
│   │   ├── include/detours.h
│   │   └── lib.x64/libdetours.a
│   └── json/                  # nlohmann JSON
│       └── json.hpp
├── dist/                      # Distribution package
│   └── EventViewerThemer.zip
├── theme.json                 # Default theme configuration
├── mingw-w64-toolchain.cmake  # Cross-compilation toolchain
├── PROJECT_PLAN.md            # Future enhancements roadmap
├── PROJECT_PLAN_COMPLETED.md  # Original completed plan
├── SETUP_INSTRUCTIONS.md      # Detailed build instructions
├── LICENSE                    # MIT License
└── README.md                  # This file
```

### Development Workflow

1. **Make code changes** in `ThemeEngine/ThemeEngine.cpp` or `Injector/Injector.cpp`
2. **Rebuild the modified component:**
   ```cmd
   cd ThemeEngine/build  # or Injector/build
   cmake --build . --config Release
   ```
3. **Test your changes:**
   - Copy new binaries to test directory
   - Run Event Viewer
   - Run Injector.exe as Administrator
   - Verify theme applies correctly

4. **Debug with Visual Studio Code:**
   - Install C/C++ extension
   - Configure launch.json for debugging
   - Attach to `mmc.exe` process for ThemeEngine debugging
   - Debug Injector.exe directly for injection logic

---

## Troubleshooting

### "Injection failed" Error

**Cause:** Event Viewer is not running or injector lacks permissions

**Solution:**
- Make sure Event Viewer (eventvwr.msc) is open before running injector
- Right-click Injector.exe and select "Run as administrator"

### Theme Not Applying

**Cause:** theme.json file is missing or malformed

**Solution:**
- Verify `theme.json` exists in the same directory as `Injector.exe`
- Check JSON syntax with a validator (https://jsonlint.com)
- Restore default theme.json from the distribution package

### Colors Are Wrong

**Cause:** Invalid hex color codes in theme.json

**Solution:**
- Use 6-digit hex format: `#RRGGBB` (e.g., `#1E1E1E`)
- Do NOT use 3-digit shorthand or RGB() format
- Use uppercase or lowercase hex digits (both work)

### Antivirus Blocking Injector.exe

**Cause:** DLL injection techniques trigger heuristic detection

**Solution:**
- Add exception for EventViewerThemer folder in your antivirus
- This is a false positive - code is open source and safe
- Build from source to verify there's no malware

### Multiple Event Viewer Windows

**Cause:** Current implementation targets first found Event Viewer

**Solution:**
- Close all Event Viewer instances
- Open only one Event Viewer
- Run injector
- Future versions will support multiple instances

---

## Security & Privacy

### Is This Safe?

**Yes, this application is safe when used responsibly:**

- **No System Modifications**: Does not change any Windows system files
- **Reversible**: Completely removed when injector is closed
- **Open Source**: All code is visible and auditable
- **Local Only**: No network connections, no data collection
- **MIT Licensed**: Free to inspect, modify, and verify

### Why Does It Need Administrator Privileges?

DLL injection requires elevated privileges to:
- Open remote process handles with `PROCESS_ALL_ACCESS`
- Allocate memory in remote process
- Create remote threads

These are legitimate Windows APIs used by many debugging tools, profilers, and utilities.

### Antivirus Detection

Some antivirus software may flag DLL injectors as potentially unwanted programs (PUPs) because:
- Malware can use similar techniques
- Heuristic analysis detects "suspicious" API calls

**This is a false positive.** The code is:
- Open source and fully auditable
- Only targets Event Viewer on your own system
- Non-persistent (no startup entries, no system changes)
- MIT licensed legitimate software

**Recommended:** Add an exception for the EventViewerThemer folder in your antivirus settings.

---

## Contributing

Contributions are welcome! Whether you're fixing bugs, adding features, or improving documentation, your help is appreciated.

### How to Contribute

1. **Fork the repository** on GitHub
2. **Create a feature branch:** `git checkout -b feature/my-new-feature`
3. **Make your changes** and test thoroughly
4. **Commit with descriptive messages:** `git commit -m "Add: GUI configuration tool"`
5. **Push to your fork:** `git push origin feature/my-new-feature`
6. **Open a Pull Request** with a clear description

### Development Guidelines

- Follow existing code style and formatting
- Add comments for complex logic
- Test on Windows 10 and 11 before submitting
- Update documentation if changing functionality
- Keep commits focused and atomic

### Reporting Issues

Found a bug or have a feature request?

1. Check [existing issues](https://github.com/azm0de/shades/issues) first
2. If new, create an issue with:
   - Clear, descriptive title
   - Steps to reproduce (for bugs)
   - Expected vs actual behavior
   - Your Windows version and system specs
   - Screenshots if applicable

---

## Roadmap

See [PROJECT_PLAN.md](PROJECT_PLAN.md) for the complete future enhancement roadmap.

### Coming Soon
- GUI configuration tool with visual color picker
- System tray integration for easy control
- Command-line arguments (--enable, --disable, --status)
- Code signing to eliminate antivirus warnings

### Long Term
- Support for other Windows management tools (perfmon, devmgmt, etc.)
- Theme marketplace for sharing custom themes
- Auto-start with Windows option
- Advanced customization (fonts, icons, conditional formatting)

---

## FAQ

### Q: Does this work on Windows 7/8?

**A:** Not tested. The application targets Windows 10+ (64-bit). It may work on Windows 7 x64, but this is not officially supported.

### Q: Can I theme other Windows applications?

**A:** Currently only Event Viewer is supported. Future versions may support additional MMC snap-ins. The architecture could be adapted for other applications with modification.

### Q: Will this break if Windows updates?

**A:** Unlikely. The application uses stable Windows APIs that have been consistent for many years. However, major Windows architecture changes could potentially affect functionality.

### Q: Can I use this on a work computer?

**A:** Check with your IT department first. While the software is legitimate, corporate policies may restrict DLL injection tools or software from unknown publishers.

### Q: Is there a GUI version?

**A:** Not yet. A GUI configuration tool is planned for v2.0 (see roadmap). The current version uses a simple command-line interface.

### Q: Why not just use High Contrast themes?

**A:** High Contrast themes affect all applications and don't provide fine-grained control. Event Viewer Themer only themes Event Viewer and allows complete color customization.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Third-Party Licenses

- **Microsoft Detours**: MIT License (Copyright (c) Microsoft Corporation)
- **nlohmann/json**: MIT License (Copyright (c) 2013-2022 Niels Lohmann)

All third-party dependencies use permissive open-source licenses compatible with this project.

---

## Acknowledgments

- **Microsoft Detours** - For the excellent API hooking library
- **Niels Lohmann** - For the fantastic header-only JSON library
- **Windows Internals Community** - For invaluable documentation and resources
- **All Contributors** - Thank you for your contributions and support!

---

## Support

- **Documentation**: Read this README and [SETUP_INSTRUCTIONS.md](SETUP_INSTRUCTIONS.md)
- **Issues**: Report bugs or request features on [GitHub Issues](https://github.com/azm0de/shades/issues)
- **Discussions**: Ask questions on [GitHub Discussions](https://github.com/azm0de/shades/discussions)
- **Email**: Contact the maintainer at [azm0de@github](mailto:azm0de@users.noreply.github.com)

---

## Project Status

**Status:** Active Development
**Version:** 1.0 (Release)
**Last Updated:** November 2025

Event Viewer Themer is actively maintained and accepting contributions. Star the repository to show your support!

---

<div align="center">

**Made with passion by [@azm0de](https://github.com/azm0de)**

If you find this project useful, please consider giving it a star on GitHub!

[![GitHub stars](https://img.shields.io/github/stars/azm0de/shades.svg?style=social&label=Star)](https://github.com/azm0de/shades)

</div>
