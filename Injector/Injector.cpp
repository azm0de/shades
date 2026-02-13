#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <cstring>

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
    std::cout << "  --config <path>     Use custom theme.json file (not yet implemented)\n";
    std::cout << "  --silent            Run without console output\n";
    std::cout << "  --disable           Disable theme (close mutex, not yet implemented)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  SHADES.exe                      # Enable theme with default settings\n";
    std::cout << "  SHADES.exe --status             # Check if theme is active\n";
    std::cout << "  SHADES.exe --silent             # Enable theme without console output\n";
    std::cout << "  SHADES.exe --help               # Show this help message\n\n";
    std::cout << "Notes:\n";
    std::cout << "  - Event Viewer must be running before launching the injector\n";
    std::cout << "  - Requires Administrator privileges for DLL injection\n";
    std::cout << "  - Keep injector running to maintain theme activation\n";
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

NOTIFYICONDATA nid;
HWND g_hTrayWnd = NULL;
HMENU g_hTrayMenu = NULL;
bool g_themeEnabled = true;

// Forward declarations
LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitTrayIcon(HWND hWnd);
void RemoveTrayIcon();
void ShowContextMenu(HWND hWnd, POINT pt);
void ToggleTheme();

// ... (Keep existing helper functions: Print, PrintError, ShowHelp, ShowVersion, IsThemeActive, GetPidsByName, EnumWindowsCallback, ParseArguments) ...

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
        switch (LOWORD(wParam)) {
        case ID_TRAY_TOGGLE:
            ToggleTheme();
            break;
        case ID_TRAY_CONFIG:
            ShellExecute(NULL, "open", "ThemeConfig.exe", NULL, NULL, SW_SHOW);
            break;
        case ID_TRAY_ABOUT:
            MessageBox(hWnd, "SHADES - Event Viewer Themer\nVersion " APP_VERSION "\n\nCreated by azm0de", "About", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_TRAY_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;

    case WM_DESTROY:
        RemoveTrayIcon();
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
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Use default app icon for now
    strcpy_s(nid.szTip, "SHADES - Event Viewer Themer");
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
        AppendMenu(hMenu, MF_STRING, ID_TRAY_CONFIG, "Configure Theme (Open JSON)");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, "About");
        AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");

        SetForegroundWindow(hWnd); // Required for menu to close when clicking outside
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
    // Parse command-line arguments
    ParseArguments(argc, argv);

    // If help/version was requested, exit cleanly
    if (g_helpRequested) {
        return 0;
    }

    // Hide console window if --silent or just by default for tray app?
    // For now, we keep console unless --silent is passed, but maybe we should hide it if it's a tray app.
    // Let's respect the flag for now.
    if (g_silent) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    Print(std::string(APP_NAME) + " v" + APP_VERSION);

    // Create the mutex that signals the theme should be active.
    const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
    g_hMutex = CreateMutexW(NULL, TRUE, mutexName);
    if (g_hMutex == NULL) {
        PrintError("Error: Could not create the theme mutex.");
        return 1;
    }

    // Check if mutex already existed (theme already active)
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Print("Warning: Theme mutex already exists. Another instance may be running.");
        Print("Proceeding with injection anyway...");
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
        // Event Viewer not running - ask user if they want to launch it
        int result = MessageBoxW(NULL,
            L"Event Viewer is not running.\n\n"
            L"Would you like to launch it now?",
            L"SHADES - Event Viewer Not Found",
            MB_YESNO | MB_ICONQUESTION);

        if (result == IDYES) {
            Print("Launching Event Viewer...");
            // Launch Event Viewer
            HINSTANCE hResult = ShellExecuteW(NULL, L"open", L"eventvwr.msc", NULL, NULL, SW_SHOW);
            if ((INT_PTR)hResult <= 32) {
                MessageBoxW(NULL,
                    L"Failed to launch Event Viewer.\n\n"
                    L"Please open it manually and try again.",
                    L"SHADES - Launch Failed",
                    MB_OK | MB_ICONERROR);
                ReleaseMutex(g_hMutex);
                CloseHandle(g_hMutex);
                return 1;
            }

            // Wait for Event Viewer to start
            Print("Waiting for Event Viewer to start...");
            Sleep(3000);

            // Re-check for mmc.exe process
            mmcPids = GetPidsByName(L"mmc.exe");
            if (mmcPids.empty()) {
                MessageBoxW(NULL,
                    L"Event Viewer did not start in time.\n\n"
                    L"Please try running SHADES again.",
                    L"SHADES - Timeout",
                    MB_OK | MB_ICONWARNING);
                ReleaseMutex(g_hMutex);
                CloseHandle(g_hMutex);
                return 1;
            }
            Print("Event Viewer started successfully!");
        } else {
            // User chose not to launch
            Print("User cancelled - exiting.");
            ReleaseMutex(g_hMutex);
            CloseHandle(g_hMutex);
            return 1;
        }
    }

    DWORD targetPid = 0;
    for (DWORD pid : mmcPids) {
        EnumData data = { pid, NULL };
        EnumWindows(EnumWindowsCallback, (LPARAM)&data);
        if (data.targetHwnd != NULL) {
            targetPid = pid;
            break;
        }
    }

    if (targetPid == 0) {
        MessageBoxW(NULL,
            L"Found mmc.exe process, but could not locate Event Viewer window.\n\n"
            L"This can happen if Event Viewer was just launched.\n"
            L"Please wait a moment and try again.",
            L"SHADES - Event Viewer Window Not Found",
            MB_OK | MB_ICONWARNING);
        PrintError("Error: Found mmc.exe, but none are hosting 'Event Viewer'.");
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    Print("Found Event Viewer process with PID: " + std::to_string(targetPid));

    // 2. Define the path to our DLL
    char dllPath[MAX_PATH];
    GetFullPathNameA("ThemeEngine.dll", MAX_PATH, dllPath, NULL);
    Print("Injecting DLL: " + std::string(dllPath));

    // 3. Get a handle to the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
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
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Check if DLL was actually loaded
    DWORD exitCode = 0;
    GetExitCodeThread(hRemoteThread, &exitCode);
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

    Print("Process complete. Theme is active.");
    
    // Force Event Viewer window to refresh
    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        Print("Refreshing Event Viewer window...");
        InvalidateRect(hEventViewer, NULL, TRUE);
        RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        Print("Window refreshed. The theme should now be visible.");
    }

    Print("Minimizing to system tray...");
    
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
