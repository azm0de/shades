# Build Instructions

## Prerequisites

- **MSVC 2019 or 2022** (Build Tools or full Visual Studio)
- **Windows SDK** (included with Visual Studio)
- **Git**

All third-party dependencies are included in the repo (`libs/detours/`, `libs/json/`).

## Quick Build

Open a terminal in the project root and run:

```cmd
:: Build ThemeEngine.dll
build_theme_engine.bat

:: Build SHADES.exe (injector)
build_injector.bat

:: Build GUI v2.0
test_build_gui.bat
```

The build scripts auto-detect Visual Studio 2019/2022 (BuildTools, Community, or Professional).

Built binaries are automatically copied to `dist\EventViewerThemer\`.

## CMake Build (Alternative)

```cmd
:: ThemeEngine
cd ThemeEngine
mkdir build && cd build
cmake ..
cmake --build . --config Release

:: Injector
cd ..\..\Injector
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Testing

1. Copy `SHADES.exe`, `ThemeEngine.dll`, and `theme.json` into the same folder
2. Open Event Viewer (`Win+R` -> `eventvwr.msc`)
3. Right-click `SHADES.exe` -> Run as administrator
4. The dark theme applies immediately; a tray icon appears

## Compiling the Detours Library (if needed)

The precompiled Detours library is already in `libs/detours/`. If you need to recompile:

1. Clone: `git clone https://github.com/microsoft/Detours.git`
2. Open a "x64 Native Tools Command Prompt"
3. Run `cd Detours && nmake`
4. Copy `include/detours.h` to `libs/detours/include/`
5. Copy `lib.X64/detours.lib` to `libs/detours/lib.x64/`

## Building the Installer

Requires [NSIS](https://nsis.sourceforge.io/) (v3.x):

```cmd
makensis installer.nsi
```

Output: `dist/EventViewerThemer-Setup.exe`
