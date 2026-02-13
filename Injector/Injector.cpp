#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>

// Version information
#define APP_VERSION "1.2.0"
#define APP_NAME "SHADES - Event Viewer Themer"

// Global flags
bool g_silent = false;
bool g_helpRequested = false;
std::string g_customThemePath = "";

// Function to print messages (respects --silent flag)
void Print(const std::string& message) {
    if (!g_silent) {
        std::cout << message << std::endl;
    }
}

void PrintError(const std::string& message) {
    if (!g_silent) {
        std::cerr << message << std::endl;
    }
}

// Function to display help message
void ShowHelp() {
    std::cout << APP_NAME << " v" << APP_VERSION << "\n";
    std::cout << "A utility to apply custom themes to Windows Event Viewer\n\n";
    std::cout << "Usage:\n";
    std::cout << "  SHADES.exe [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help              Display this help message\n";
    std::cout << "  --version           Display version information\n";
    std::cout << "  --status            Check if theme is currently active\n";
    std::cout << "  --config <path>     Use custom theme.json file\n";
    std::cout << "  --silent            Run without console output\n";
    std::cout << "  --disable           Shut down running SHADES instance\n\n";
    std::cout << "Examples:\n";
    std::cout << "  SHADES.exe                      # Enable theme with default settings\n";
    std::cout << "  SHADES.exe --status             # Check if theme is active\n";
    std::cout << "  SHADES.exe --silent             # Enable theme without console output\n";
    std::cout << "  SHADES.exe --help               # Show this help message\n\n";
    std::cout << "Notes:\n";
    std::cout << "  - SHADES auto-launches Event Viewer if not already running\n";
    std::cout << "  - Requires Administrator privileges (auto-elevated via manifest)\n";
    std::cout << "  - Runs as a system tray application\n";
    std::cout << "  - Theme files: ThemeEngine.dll and theme.json (same directory)\n\n";
    std::cout << "For more information, visit: https://github.com/azm0de/shades\n";
}

// Function to display version
void ShowVersion() {
    std::cout << APP_NAME << " version " << APP_VERSION << "\n";
    std::cout << "Copyright (c) 2025 azm0de\n";
    std::cout << "Licensed under the MIT License\n";
}

// Function to check if theme mutex exists (theme is active)
bool IsThemeActive() {
    const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, mutexName);
    if (hMutex != NULL) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

// Function to get the Process ID (PID) for all processes with a given name.
std::vector<DWORD> GetPidsByName(const std::wstring& processName) {
    std::vector<DWORD> pids;
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return pids;
    }

    if (Process32FirstW(hSnapshot, &processEntry)) {
        do {
            if (processName == processEntry.szExeFile) {
                pids.push_back(processEntry.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return pids;
}

// Structure to pass data to our window enumeration callback.
struct EnumData {
    DWORD pid;
    HWND targetHwnd;
};

// Callback function for EnumWindows. It checks if a window belongs to our target PID
// and has the correct title.
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    EnumData* pData = (EnumData*)lParam;
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);

    if (windowPid == pData->pid) {
        wchar_t windowTitle[256];
        if (GetWindowTextW(hwnd, windowTitle, 256) > 0) {
            if (std::wstring(windowTitle) == L"Event Viewer") {
                pData->targetHwnd = hwnd;
                return FALSE; // Stop enumerating
            }
        }
    }
    return TRUE; // Continue enumerating
}

// Parse command-line arguments
void ParseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h" || arg == "/?") {
            ShowHelp();
            g_helpRequested = true;
            return;
        }
        else if (arg == "--version" || arg == "-v") {
            ShowVersion();
            g_helpRequested = true;
            return;
        }
        else if (arg == "--status") {
            if (IsThemeActive()) {
                std::cout << "Theme Status: ACTIVE\n";
                std::cout << "The Event Viewer theme is currently enabled.\n";
            } else {
                std::cout << "Theme Status: INACTIVE\n";
                std::cout << "The Event Viewer theme is not currently active.\n";
            }
            g_helpRequested = true;
            return;
        }
        else if (arg == "--silent" || arg == "-s") {
            g_silent = true;
        }
        else if (arg == "--config") {
            if (i + 1 < argc) {
                g_customThemePath = argv[i + 1];
                i++; // Skip next argument
                Print("Using custom theme path: " + g_customThemePath);
            } else {
                PrintError("Error: --config requires a file path argument");
                g_helpRequested = true;
                return;
            }
        }
        else if (arg == "--disable") {
            // Find running SHADES tray window and send WM_CLOSE
            HWND hTrayWnd = FindWindowA("ShadesTrayWnd", NULL);
            if (hTrayWnd) {
                SendMessage(hTrayWnd, WM_CLOSE, 0, 0);
                Print("Theme disabled. SHADES instance shut down.");
            } else {
                PrintError("No active SHADES instance found.");
            }
            g_helpRequested = true;
            return;
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Use --help to see available options.\n";
            g_helpRequested = true;
            return;
        }
    }
}


// Global variables for Tray Icon
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_TOGGLE 1003
#define ID_TRAY_CONFIG 1004
#define ID_TRAY_ABOUT 1005
#define ID_TRAY_AUTOSTART 1006
#define ID_TRAY_THEME_BASE 2000
#define ID_TRAY_THEME_MAX 2100

NOTIFYICONDATA nid;
HWND g_hTrayWnd = NULL;
HMENU g_hTrayMenu = NULL;
bool g_themeEnabled = true;
char g_exeDir[MAX_PATH] = {0};
DWORD g_targetPid = 0; // PID of the injected mmc.exe process

// Store discovered theme filenames for the quick-switch submenu
std::vector<std::string> g_themeFiles;

// Forward declarations
LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitTrayIcon(HWND hWnd);
void RemoveTrayIcon();
void ShowContextMenu(HWND hWnd, POINT pt);
void ToggleTheme();
void ShowTrayNotification(const char* title, const char* message);
bool IsAutoStartEnabled();
void ToggleAutoStart();
void ApplyQuickTheme(int index);

// Get the directory containing SHADES.exe
void InitExeDir() {
    GetModuleFileNameA(NULL, g_exeDir, MAX_PATH);
    char* lastSlash = strrchr(g_exeDir, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
}

// Show a tray balloon notification
void ShowTrayNotification(const char* title, const char* message) {
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    strcpy_s(nid.szInfoTitle, title);
    strcpy_s(nid.szInfo, message);
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 3000;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

// Update tray tooltip to reflect current state
void UpdateTrayTooltip() {
    if (g_themeEnabled) {
        strcpy_s(nid.szTip, "SHADES - Theme Active");
    } else {
        strcpy_s(nid.szTip, "SHADES - Theme Disabled");
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

// Check if SHADES auto-start task exists
bool IsAutoStartEnabled() {
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};
    char cmd[] = "schtasks /query /tn \"SHADES\"";
    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 5000);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return (exitCode == 0);
    }
    return false;
}

// Toggle auto-start with Windows via Task Scheduler
void ToggleAutoStart() {
    if (IsAutoStartEnabled()) {
        char cmd[] = "schtasks /delete /tn \"SHADES\" /f";
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        ShowTrayNotification("SHADES", "Auto-start disabled");
    } else {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char cmd[1024];
        sprintf_s(cmd, "schtasks /create /tn \"SHADES\" /tr \"\\\"%s\\\" --silent\" /sc onlogon /rl highest /f", exePath);
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        ShowTrayNotification("SHADES", "Will start automatically on login");
    }
}

// Apply a theme from the quick-switch submenu
void ApplyQuickTheme(int index) {
    if (index < 0 || index >= (int)g_themeFiles.size()) return;

    char srcPath[MAX_PATH];
    sprintf_s(srcPath, "%sthemes\\%s", g_exeDir, g_themeFiles[index].c_str());

    char dstPath[MAX_PATH];
    sprintf_s(dstPath, "%stheme.json", g_exeDir);

    if (CopyFileA(srcPath, dstPath, FALSE)) {
        // Extract display name (filename without .json)
        std::string name = g_themeFiles[index];
        size_t dot = name.rfind('.');
        if (dot != std::string::npos) name = name.substr(0, dot);
        std::string msg = "Switched to " + name + " theme";
        ShowTrayNotification("SHADES", msg.c_str());
    }
}

// Window Procedure for the hidden window handling tray messages
LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            ShowContextMenu(hWnd, pt);
        }
        else if (lParam == WM_LBUTTONDBLCLK) {
             ToggleTheme();
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) >= ID_TRAY_THEME_BASE && LOWORD(wParam) < ID_TRAY_THEME_MAX) {
            ApplyQuickTheme(LOWORD(wParam) - ID_TRAY_THEME_BASE);
            break;
        }
        switch (LOWORD(wParam)) {
        case ID_TRAY_TOGGLE:
            ToggleTheme();
            break;
        case ID_TRAY_CONFIG:
            {
                char guiPath[MAX_PATH];
                sprintf_s(guiPath, "%sSHADES_GUI.exe", g_exeDir);
                // Fall back to ThemeConfig.exe if GUI not found
                if (GetFileAttributesA(guiPath) == INVALID_FILE_ATTRIBUTES) {
                    sprintf_s(guiPath, "%sThemeConfig.exe", g_exeDir);
                }
                ShellExecuteA(NULL, "open", guiPath, NULL, NULL, SW_SHOW);
            }
            break;
        case ID_TRAY_AUTOSTART:
            ToggleAutoStart();
            break;
        case ID_TRAY_ABOUT:
            MessageBox(hWnd, "SHADES - Event Viewer Themer\nVersion " APP_VERSION "\n\nCreated by azm0de\nhttps://github.com/azm0de/shades", "About", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_TRAY_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;

    case WM_DESTROY:
        RemoveTrayIcon();
        // Terminate the Event Viewer process we injected into (unloads ThemeEngine.dll)
        if (g_targetPid != 0) {
            HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, g_targetPid);
            if (hProc) {
                TerminateProcess(hProc, 0);
                CloseHandle(hProc);
            }
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void InitTrayIcon(HWND hWnd) {
    memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = ID_TRAY_APP_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(nid.szTip, "SHADES - Theme Active");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd, POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        if (g_themeEnabled) {
            AppendMenu(hMenu, MF_STRING, ID_TRAY_TOGGLE, "Disable Theme");
        }
        else {
            AppendMenu(hMenu, MF_STRING, ID_TRAY_TOGGLE, "Enable Theme");
        }

        // Build quick-theme submenu from themes/ directory
        HMENU hThemesMenu = CreatePopupMenu();
        g_themeFiles.clear();
        if (hThemesMenu) {
            char searchPath[MAX_PATH];
            sprintf_s(searchPath, "%sthemes\\*.json", g_exeDir);
            WIN32_FIND_DATAA fd;
            HANDLE hFind = FindFirstFileA(searchPath, &fd);
            int themeId = ID_TRAY_THEME_BASE;
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    g_themeFiles.push_back(fd.cFileName);
                    // Display name = filename without .json
                    char displayName[256];
                    strcpy_s(displayName, fd.cFileName);
                    char* dot = strrchr(displayName, '.');
                    if (dot) *dot = '\0';
                    AppendMenuA(hThemesMenu, MF_STRING, themeId++, displayName);
                } while (FindNextFileA(hFind, &fd) && themeId < ID_TRAY_THEME_MAX);
                FindClose(hFind);
            }
            if (g_themeFiles.empty()) {
                AppendMenuA(hThemesMenu, MF_STRING | MF_GRAYED, 0, "(No themes found)");
            }
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hThemesMenu, "Themes");
        }

        AppendMenu(hMenu, MF_STRING, ID_TRAY_CONFIG, "Theme Editor");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, IsAutoStartEnabled() ? MF_STRING | MF_CHECKED : MF_STRING,
                   ID_TRAY_AUTOSTART, "Start with Windows");
        AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, "About");
        AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");

        SetForegroundWindow(hWnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}

// Global mutex handle to manage lifetime
HANDLE g_hMutex = NULL;

void ToggleTheme() {
    if (g_themeEnabled) {
        // Disable theme: Release and close mutex
        if (g_hMutex) {
            ReleaseMutex(g_hMutex);
            CloseHandle(g_hMutex);
            g_hMutex = NULL;
        }
        g_themeEnabled = false;
        Print("Theme disabled.");
        UpdateTrayTooltip();
        ShowTrayNotification("SHADES", "Theme disabled");

        // Force refresh to show unthemed state
        HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
        if (hEventViewer) {
            InvalidateRect(hEventViewer, NULL, TRUE);
            RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        }
    }
    else {
        // Enable theme: Create mutex
        const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
        g_hMutex = CreateMutexW(NULL, TRUE, mutexName);
        if (g_hMutex) {
             g_themeEnabled = true;
             Print("Theme enabled.");
             UpdateTrayTooltip();
             ShowTrayNotification("SHADES", "Theme enabled");

             // Force refresh to show themed state
             HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
             if (hEventViewer) {
                InvalidateRect(hEventViewer, NULL, TRUE);
                RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
             }
        } else {
            PrintError("Failed to create mutex. Theme cannot be enabled.");
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize exe directory path (used throughout)
    InitExeDir();

    // Parse command-line arguments
    ParseArguments(argc, argv);

    // If help/version was requested, exit cleanly
    if (g_helpRequested) {
        return 0;
    }

    // Hide console window by default (this is a tray app)
    HWND hConsole = GetConsoleWindow();
    if (hConsole) {
        ShowWindow(hConsole, SW_HIDE);
    }

    Print(std::string(APP_NAME) + " v" + APP_VERSION);

    // Create the mutex that signals the theme should be active.
    const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
    g_hMutex = CreateMutexW(NULL, TRUE, mutexName);
    if (g_hMutex == NULL) {
        PrintError("Error: Could not create the theme mutex.");
        return 1;
    }

    // Check if mutex already existed (another instance running)
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(g_hMutex);
        g_hMutex = NULL;

        HWND hExisting = FindWindowA("ShadesTrayWnd", NULL);
        if (hExisting) {
            MessageBoxW(NULL,
                L"SHADES is already running!\n\n"
                L"Look for the SHADES icon in your system tray\n"
                L"(bottom-right of your taskbar).\n\n"
                L"Right-click the tray icon for options.",
                L"SHADES - Already Running",
                MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        // Stale mutex (no tray window found) -- take ownership
        g_hMutex = CreateMutexW(NULL, TRUE, mutexName);
        if (g_hMutex == NULL) {
            PrintError("Error: Could not create the theme mutex.");
            return 1;
        }
    }

    g_themeEnabled = true;

    // Set custom theme path environment variable if specified
    if (!g_customThemePath.empty()) {
        SetEnvironmentVariableA("SHADES_THEME_PATH", g_customThemePath.c_str());
        Print("Custom theme path set: " + g_customThemePath);
    }

    Print("Searching for Event Viewer (mmc.exe) process...");

    // 1. Find the target process
    std::vector<DWORD> mmcPids = GetPidsByName(L"mmc.exe");
    if (mmcPids.empty()) {
        // Auto-launch Event Viewer
        Print("Event Viewer not running. Launching automatically...");
        HINSTANCE hResult = ShellExecuteW(NULL, L"open", L"eventvwr.msc", NULL, NULL, SW_SHOW);
        if ((INT_PTR)hResult <= 32) {
            MessageBoxW(NULL,
                L"Failed to launch Event Viewer.\n\n"
                L"Please open Event Viewer manually and try again.",
                L"SHADES - Launch Failed",
                MB_OK | MB_ICONERROR);
            ReleaseMutex(g_hMutex);
            CloseHandle(g_hMutex);
            return 1;
        }

        // Poll for Event Viewer with 15-second timeout
        Print("Waiting for Event Viewer to start...");
        const int MAX_WAIT_MS = 15000;
        const int POLL_INTERVAL_MS = 500;
        int elapsed = 0;
        while (elapsed < MAX_WAIT_MS) {
            Sleep(POLL_INTERVAL_MS);
            elapsed += POLL_INTERVAL_MS;
            mmcPids = GetPidsByName(L"mmc.exe");
            if (!mmcPids.empty()) {
                // Verify the Event Viewer window exists (not just mmc.exe starting)
                for (DWORD pid : mmcPids) {
                    EnumData data = { pid, NULL };
                    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
                    if (data.targetHwnd != NULL) {
                        goto ev_found;
                    }
                }
            }
        }
        // Timeout
        MessageBoxW(NULL,
            L"Event Viewer did not start in time.\n\n"
            L"Please try running SHADES again.",
            L"SHADES - Timeout",
            MB_OK | MB_ICONWARNING);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;

    ev_found:
        Print("Event Viewer started successfully!");
    }

    for (DWORD pid : mmcPids) {
        EnumData data = { pid, NULL };
        EnumWindows(EnumWindowsCallback, (LPARAM)&data);
        if (data.targetHwnd != NULL) {
            g_targetPid = pid;
            break;
        }
    }

    if (g_targetPid == 0) {
        MessageBoxW(NULL,
            L"Found mmc.exe but could not locate Event Viewer window.\n\n"
            L"Event Viewer may still be loading.\n"
            L"Please wait a moment and try again.",
            L"SHADES",
            MB_OK | MB_ICONWARNING);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    Print("Found Event Viewer process with PID: " + std::to_string(g_targetPid));

    // 2. Define the path to our DLL (relative to exe, not CWD)
    char dllPath[MAX_PATH];
    sprintf_s(dllPath, "%sThemeEngine.dll", g_exeDir);

    // Pre-flight check: verify DLL exists
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
        char errMsg[512];
        sprintf_s(errMsg, "ThemeEngine.dll not found!\n\nExpected location:\n%s\n\nMake sure ThemeEngine.dll is in the same folder as SHADES.exe.", dllPath);
        MessageBoxA(NULL, errMsg, "SHADES - Missing File", MB_OK | MB_ICONERROR);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    Print("Injecting DLL: " + std::string(dllPath));

    // 3. Get a handle to the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_targetPid);
    if (hProcess == NULL) {
        MessageBoxW(NULL,
            L"Could not open Event Viewer process.\n\n"
            L"Make sure you are running SHADES.exe as Administrator.\n\n"
            L"Right-click SHADES.exe and select 'Run as administrator'.",
            L"SHADES - Access Denied",
            MB_OK | MB_ICONERROR);
        PrintError("Error: Could not open process. Try running as administrator.");
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    // 4. Allocate memory in the target process for the DLL path
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteMem == NULL) {
        PrintError("Error: Could not allocate memory in target process.");
        CloseHandle(hProcess);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    // 5. Write the DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath, sizeof(dllPath), NULL)) {
        PrintError("Error: Could not write to target process memory.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    // 6. Get the address of the LoadLibraryA function
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        PrintError("Error: Could not find LoadLibraryA address.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    // 7. Create a remote thread in the target process to load our DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemoteMem, 0, NULL);
    if (hRemoteThread == NULL) {
        PrintError("Error: Could not create remote thread.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    Print("Injection successful. Waiting for remote thread to finish...");
    DWORD waitResult = WaitForSingleObject(hRemoteThread, 30000); // 30s timeout

    // Check if DLL was actually loaded
    DWORD exitCode = 0;
    if (waitResult == WAIT_TIMEOUT) {
        Print("Warning: Remote thread timed out, but proceeding...");
        exitCode = 1; // Assume success if timeout (DLL may already be loaded)
    } else {
        GetExitCodeThread(hRemoteThread, &exitCode);
    }
    if (exitCode == 0) {
        MessageBoxW(NULL,
            L"DLL injection failed.\n\n"
            L"Make sure ThemeEngine.dll is in the same folder as SHADES.exe.\n\n"
            L"Current folder: " _CRT_WIDE(__FILE__) L"\\..\\..",
            L"SHADES - DLL Injection Failed",
            MB_OK | MB_ICONERROR);
        PrintError("Error: DLL injection failed - LoadLibraryA returned NULL");
        PrintError("Make sure ThemeEngine.dll is in the same directory as SHADES.exe");
        CloseHandle(hRemoteThread);
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    char successMsg[256];
    sprintf_s(successMsg, 256, "DLL loaded successfully. Module handle: 0x%lX", exitCode);
    Print(successMsg);

    // 8. Clean up handles
    Print("Cleaning up handles.");
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    Print("Process complete. Waiting for theme engine to initialize...");

    // The DLL's DllMain spawns a worker thread that sleeps 200ms then
    // initializes hooks/subclasses. Wait for it to finish before refreshing.
    Sleep(500);

    // Force Event Viewer window to refresh
    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        Print("Refreshing Event Viewer window...");
        InvalidateRect(hEventViewer, NULL, TRUE);
        RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        Print("Window refreshed. The theme should now be visible.");
    }

    Print("Running in system tray...");

    // Create a hidden window to handle tray messages
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ShadesTrayWnd";
    RegisterClassEx(&wc);

    g_hTrayWnd = CreateWindowEx(0, "ShadesTrayWnd", "Shades Tray Window", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);

    if (g_hTrayWnd) {
        InitTrayIcon(g_hTrayWnd);
        ShowTrayNotification("SHADES", "Dark theme applied to Event Viewer");

        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } else {
        PrintError("Failed to create tray window.");
    }

    // Clean up
    if (g_hMutex) {
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
    }

    return 0;
}
