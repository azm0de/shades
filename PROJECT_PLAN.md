# Project Plan: Event Viewer Themer

This project is divided into three primary phases. We will first build the core theming engine (the DLL), then the application that injects it, and finally, we'll add advanced features like custom theme files and real-time toggling.

## Phase 0: Prerequisites & Project Setup

**Objective**: Prepare the development environment and all necessary third-party libraries before writing any project-specific code.

### Task 0.1: Environment Setup

- **Action**: Configure VS Code with a C++ compiler toolchain (MSVC is recommended).
- **Action**: Install the latest Windows SDK, which provides necessary headers and tools like Inspect.exe.

### Task 0.2: Compile the Microsoft Detours Library

**Objective**: We need a robust API hooking library. Microsoft Detours is the ideal choice.

**Action**:
- Clone the official Detours repository from GitHub.
- Follow its documentation to compile the library using nmake. This will produce a static library file (e.g., detours.lib).
- Create both 32-bit and 64-bit builds of the library, as our final solution must match the architecture of the target mmc.exe process.

## Phase 1: The Payload (ThemeEngine.dll)

**Objective**: Create the Dynamic-Link Library that, once injected into mmc.exe, will perform the actual UI modifications.

### Chunk 1.1: Create the Basic DLL Project

**Objective**: Establish the foundational structure of our theming engine.

**Files to Create**:
- ThemeEngine.cpp: The main source file.
- CMakeLists.txt (or VS Project file): To manage the build.

**Logic Outline for AI Assistant**:
- Create a standard Win32 DLL project.
- Implement the DllMain entry point. This function is called by the OS when the DLL is loaded into a process (DLL_PROCESS_ATTACH).
- Inside the DLL_PROCESS_ATTACH case, create a new thread that will run our main initialization function. This prevents blocking the target application's loader lock.

### Chunk 1.2: Integrate Detours and Hook GetSysColor

**Objective**: Implement our first and most impactful hook to change the main background and text colors.

**Files to Modify**: ThemeEngine.cpp, CMakeLists.txt

**Logic Outline for AI Assistant**:
- Configure the project to link against the detours.lib file compiled in Phase 0.
- Include the detours.h header.
- Define a pointer to the original GetSysColor function.
- Create our detour function, DetouredGetSysColor, with the same signature as the original.
- Inside DetouredGetSysColor:
  - Use a switch statement on the color index parameter (nIndex).
  - If nIndex is COLOR_WINDOW, return a dark gray color (e.g., RGB(30, 30, 30)).
  - If nIndex is COLOR_WINDOWTEXT, return a light gray color (e.g., RGB(212, 212, 212)).
  - For any other nIndex, call the original GetSysColor function via the pointer.
- In the initialization function, use the Detours transaction model to attach the hook: DetourTransactionBegin(), DetourUpdateThread(GetCurrentThread()), DetourAttach(), and DetourTransactionCommit().

### Chunk 1.3: Hook GDI & UxTheme Drawing Functions

**Objective**: Theme elements that are drawn directly, not using system colors. This includes themed headers and backgrounds.

**Files to Modify**: ThemeEngine.cpp

**Logic Outline for AI Assistant**:
- Repeat the hooking process from Chunk 1.2 for the following functions:
  - FillRect (from user32.dll): In the detour, check the handle to the brush being used. If it's a system background brush (like COLOR_WINDOW), replace it with our custom dark brush before calling the original FillRect.
  - DrawThemeBackground (from uxtheme.dll): This is crucial for themed controls. In the detour, check the part and state IDs. If they correspond to a background we want to change, ignore the call to the original function and instead use FillRect with our custom dark brush.

### Chunk 1.4: Implement Window Subclassing for the ListView

**Objective**: Gain granular control over the main event list (SysListView32) to theme individual rows, selections, and text colors.

**Files to Modify**: ThemeEngine.cpp

**Logic Outline for AI Assistant**:
- In the initialization function, after hooks are applied, use FindWindowEx to get the handle (HWND) of the SysListView32 control within the Event Viewer window.
- Use SetWindowLongPtr with GWLP_WNDPROC to replace the ListView's window procedure with our custom CustomListViewProc. Store the original procedure pointer.
- In CustomListViewProc:
  - Intercept the WM_NOTIFY message.
  - Check if the notification code is NM_CUSTOMDRAW.
  - Handle the different draw stages (CDDS_PREPAINT, CDDS_ITEMPREPAINT, etc.).
  - At the CDDS_ITEMPREPAINT stage, set the text and background colors of the NMCUSTOMDRAW struct to our custom theme colors.
  - Pass all other unhandled messages to the original window procedure using CallWindowProc.

## Phase 2: The Injector (ThemeManager.exe)

**Objective**: Create the user-facing executable that finds the Event Viewer process and injects our ThemeEngine.dll.

### Chunk 2.1: Find the Target Process

**Objective**: Reliably identify the specific mmc.exe process that is hosting the Event Viewer.

**Files to Create**: Injector.cpp, CMakeLists.txt

**Logic Outline for AI Assistant**:
- Create a basic C++ console application.
- Use the CreateToolhelp32Snapshot, Process32First, and Process32Next functions to enumerate all running processes.
- For each process, check if its name is mmc.exe.
- If it is, enumerate its top-level windows. Check if any window has the title "Event Viewer".
- Store the Process ID (PID) of the correct process.

### Chunk 2.2: Implement DLL Injection

**Objective**: Inject ThemeEngine.dll into the target process identified in the previous step.

**Files to Modify**: Injector.cpp

**Logic Outline for AI Assistant**:
- Use OpenProcess with the target PID to get a handle to the process.
- Use VirtualAllocEx to allocate memory inside the target process, large enough to hold the full path to ThemeEngine.dll.
- Use WriteProcessMemory to write the DLL's path into that allocated memory.
- Get the address of the LoadLibraryA function from kernel32.dll.
- Use CreateRemoteThread to start a new thread in the target process. Set its start address to LoadLibraryA and its parameter to the memory address containing the DLL path.
- Clean up by closing all opened handles.

## Phase 3: Configuration & Final Touches

**Objective**: Decouple the theme from the code and provide a way for the user to control the theming without restarting the application.

### Chunk 3.1: External JSON Theme File

**Objective**: Allow users to define their own colors by reading them from a configuration file.

**Files to Create**: theme.json

**Files to Modify**: ThemeEngine.cpp

**Logic Outline for AI Assistant**:
- Add a lightweight C++ JSON library (like nlohmann/json) to the ThemeEngine.dll project.
- Create a theme.json file with key-value pairs for colors (e.g., "window_bg": "#1E1E1E").
- In the DLL's initialization function, read and parse this JSON file.
- Store the loaded colors in global variables or a struct.
- Modify all hook handlers to use these variables instead of hardcoded RGB values.

### Chunk 3.2: IPC for Toggling the Theme

**Objective**: Create a simple on/off switch for the theme that can be controlled by the injector.

**Files to Modify**: Injector.cpp, ThemeEngine.cpp

**Logic Outline for AI Assistant**:
- In Injector.exe: Add command-line argument parsing.
  - If the argument is --enable, create a named mutex (e.g., "Global\\EventViewerThemeActive").
  - If the argument is --disable, find and close the handle to that mutex.
- In ThemeEngine.dll: At the beginning of every detour function (e.g., DetouredGetSysColor), check for the existence of the named mutex.
  - If the mutex exists, proceed with the custom theming logic.
  - If it does not exist, immediately call the original function and return its result, bypassing the theme.

---

With this plan, we can systematically build the application. We'll start with the core functionality in the DLL and then build the loader to activate it. Let me know when you're ready to proceed with the first chunk.
