# Project Plan: Event Viewer Themer (COMPLETED)

## Project Status: 100% COMPLETE

**Completion Date**: November 2025
**Final Build**: Distribution package v1.0 (EventViewerThemer.zip)
**Repository**: https://github.com/azm0de/shades

### Achievement Summary

This project successfully implemented a Windows utility that applies customizable dark themes to the Windows Event Viewer (eventvwr.msc) using advanced DLL injection and API hooking techniques. All planned phases have been completed and the application is production-ready.

**Key Accomplishments**:
- ✅ Fully functional theming engine with API hooks for GetSysColor, GetSysColorBrush, and DrawThemeBackground
- ✅ ListView custom drawing implementation for granular control over event list appearance
- ✅ Process injection system that targets Event Viewer with precision
- ✅ JSON-based configuration system for customizable color schemes
- ✅ IPC mutex-based toggle mechanism for real-time theme activation/deactivation
- ✅ Cross-platform build support (Windows MSVC + Linux MinGW-w64)
- ✅ Distribution package ready with compiled binaries (Injector.exe 162KB, ThemeEngine.dll 1.6MB)

**Technical Stack**:
- C++ with Windows API (user32, gdi32, uxtheme, kernel32)
- Microsoft Detours 4.0+ for API hooking
- nlohmann/json for configuration parsing
- CMake build system with MinGW-w64 cross-compilation support

---

## Original Project Plan

This project was divided into three primary phases. We built the core theming engine (the DLL), then the application that injects it, and finally added advanced features like custom theme files and real-time toggling.

## Phase 0: Prerequisites & Project Setup ✅ COMPLETED

**Objective**: Prepare the development environment and all necessary third-party libraries before writing any project-specific code.

### Task 0.1: Environment Setup ✅

- **Action**: Configure VS Code with a C++ compiler toolchain (MSVC is recommended).
- **Action**: Install the latest Windows SDK, which provides necessary headers and tools like Inspect.exe.
- **Status**: Completed with MinGW-w64 GCC 13 toolchain for cross-compilation

### Task 0.2: Compile the Microsoft Detours Library ✅

**Objective**: We need a robust API hooking library. Microsoft Detours is the ideal choice.

**Action**:
- Clone the official Detours repository from GitHub.
- Follow its documentation to compile the library using nmake. This will produce a static library file (e.g., detours.lib).
- Create both 32-bit and 64-bit builds of the library, as our final solution must match the architecture of the target mmc.exe process.
- **Status**: Completed - Compiled 64-bit library (libdetours.a) located in `libs/detours/lib.x64/`

## Phase 1: The Payload (ThemeEngine.dll) ✅ COMPLETED

**Objective**: Create the Dynamic-Link Library that, once injected into mmc.exe, will perform the actual UI modifications.

### Chunk 1.1: Create the Basic DLL Project ✅

**Objective**: Establish the foundational structure of our theming engine.

**Files Created**:
- ThemeEngine/ThemeEngine.cpp: The main source file (202 lines)
- ThemeEngine/CMakeLists.txt: Build configuration

**Implementation**:
- Created standard Win32 DLL project
- Implemented DllMain entry point with DLL_PROCESS_ATTACH and DLL_PROCESS_DETACH handling
- Direct initialization in DllMain (no separate thread needed for this implementation)
- **Status**: Completed and functional

### Chunk 1.2: Integrate Detours and Hook GetSysColor ✅

**Objective**: Implement our first and most impactful hook to change the main background and text colors.

**Files Modified**: ThemeEngine.cpp, CMakeLists.txt

**Implementation**:
- Configured project to link against libdetours.a
- Included detours.h header
- Defined pointer to original GetSysColor function
- Created DetouredGetSysColor with color mapping for:
  - COLOR_WINDOW → Dark background from theme.json
  - COLOR_WINDOWTEXT → Light text from theme.json
  - COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT, COLOR_BTNFACE, COLOR_BTNTEXT
- Used Detours transaction model for hook attachment
- **Status**: Completed with dynamic color loading from JSON

### Chunk 1.3: Hook GDI & UxTheme Drawing Functions ✅

**Objective**: Theme elements that are drawn directly, not using system colors. This includes themed headers and backgrounds.

**Files Modified**: ThemeEngine.cpp

**Implementation**:
- Hooked GetSysColorBrush to return custom dark brushes
- Hooked DrawThemeBackground for themed controls (especially headers)
- Brush caching system to avoid resource leaks
- **Status**: Completed with proper resource cleanup

### Chunk 1.4: Implement Window Subclassing for the ListView ✅

**Objective**: Gain granular control over the main event list (SysListView32) to theme individual rows, selections, and text colors.

**Files Modified**: ThemeEngine.cpp

**Implementation**:
- Used FindWindowEx to locate SysListView32 control
- Implemented SetWindowLongPtr with GWLP_WNDPROC for window subclassing
- Created CustomListViewProc to intercept WM_NOTIFY messages
- Handled NM_CUSTOMDRAW notifications with proper draw stage handling:
  - CDDS_PREPAINT → Return CDRF_NOTIFYITEMDRAW
  - CDDS_ITEMPREPAINT → Set custom colors and return CDRF_NEWFONT
- Applied custom text and background colors from theme.json
- **Status**: Completed with full custom drawing support

## Phase 2: The Injector (ThemeManager.exe) ✅ COMPLETED

**Objective**: Create the user-facing executable that finds the Event Viewer process and injects our ThemeEngine.dll.

### Chunk 2.1: Find the Target Process ✅

**Objective**: Reliably identify the specific mmc.exe process that is hosting the Event Viewer.

**Files Created**: Injector/Injector.cpp (189 lines), Injector/CMakeLists.txt

**Implementation**:
- Created console application with proper error handling
- Used CreateToolhelp32Snapshot, Process32First, and Process32Next for process enumeration
- Filtered for mmc.exe processes
- Enumerated top-level windows to find "Event Viewer" window title
- Stored correct Process ID for injection
- **Status**: Completed with robust process discovery

### Chunk 2.2: Implement DLL Injection ✅

**Objective**: Inject ThemeEngine.dll into the target process identified in the previous step.

**Files Modified**: Injector.cpp

**Implementation**:
- Used OpenProcess to get process handle with PROCESS_ALL_ACCESS
- Used VirtualAllocEx to allocate memory in target process
- Used WriteProcessMemory to write DLL path to allocated memory
- Retrieved LoadLibraryA address from kernel32.dll
- Used CreateRemoteThread to inject DLL via LoadLibrary
- Added InvalidateRect and RedrawWindow to force UI refresh
- Proper handle cleanup with CloseHandle and VirtualFreeEx
- **Status**: Completed with reliable injection mechanism

## Phase 3: Configuration & Final Touches ✅ COMPLETED

**Objective**: Decouple the theme from the code and provide a way for the user to control the theming without restarting the application.

### Chunk 3.1: External JSON Theme File ✅

**Objective**: Allow users to define their own colors by reading them from a configuration file.

**Files Created**: theme.json

**Files Modified**: ThemeEngine.cpp

**Implementation**:
- Integrated nlohmann/json header-only library
- Created theme.json with comprehensive color definitions:
  - window_bg, window_text, highlight_bg, highlight_text
  - button_face, button_text, header_bg
- Implemented JSON parsing in DLL initialization
- Created hex-to-RGB color conversion function
- Stored colors in global ThemeConfig struct
- All hook handlers use dynamic colors from configuration
- **Status**: Completed with full customization support

### Chunk 3.2: IPC for Toggling the Theme ✅

**Objective**: Create a simple on/off switch for the theme that can be controlled by the injector.

**Files Modified**: Injector.cpp, ThemeEngine.cpp

**Implementation**:
- Created named mutex "Global\\EventViewerThemeActive" in Injector
- Mutex remains alive while Injector.exe is running
- Mutex destroyed when Injector exits (automatic theme disable)
- All detour functions check mutex existence before applying theme
- Immediate fallback to original functions when mutex not present
- **Status**: Completed with real-time toggle capability
- **Note**: Command-line arguments (--enable/--disable) not implemented; current design uses process lifetime for control

---

## Final Deliverables

### Distribution Package (dist/EventViewerThemer.zip)
- **Injector.exe** (162KB) - Main executable
- **ThemeEngine.dll** (1.6MB) - Theming engine
- **theme.json** - Default dark theme configuration
- **README.txt** - End-user instructions

### Build Artifacts
- ThemeEngine/build/ - DLL build directory
- Injector/build/ - Executable build directory
- Both components compiled for Windows x64 (PE32+)

### Documentation
- SETUP_INSTRUCTIONS.md - Build and development guide
- PROJECT_PLAN.md (this file) - Complete project roadmap

---

## Lessons Learned

1. **Cross-compilation Success**: Successfully built Windows binaries from Linux using MinGW-w64, resolving case-sensitivity issues with Windows headers
2. **Detours Integration**: Microsoft Detours proved highly reliable for API hooking with minimal overhead
3. **Mutex-based IPC**: Simple and effective approach for theme toggling without complex inter-process communication
4. **Custom Drawing**: ListView NM_CUSTOMDRAW provides complete control over item appearance
5. **Resource Management**: Proper brush caching and cleanup prevents resource leaks during extended runtime

## Future Enhancement Ideas

While the project is complete as planned, potential future enhancements could include:
- GUI configuration tool for theme.json editing
- Command-line arguments for --enable/--disable flags
- System tray icon for easier control
- Support for additional MMC snap-ins beyond Event Viewer
- Auto-start with Windows option
- Multiple theme presets
- Real-time theme reload without restarting
- Installer with NSIS or WiX Toolset
- Code signing for executables

---

**Project Completed Successfully** - All phases delivered as specified with production-ready binaries.
