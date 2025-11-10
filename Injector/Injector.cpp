#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <cstring>

// Version information
#define APP_VERSION "1.1.0"
#define APP_NAME "Event Viewer Themer"

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
    std::cout << "A utility to apply dark themes to Windows Event Viewer\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Injector.exe [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help              Display this help message\n";
    std::cout << "  --version           Display version information\n";
    std::cout << "  --status            Check if theme is currently active\n";
    std::cout << "  --config <path>     Use custom theme.json file (not yet implemented)\n";
    std::cout << "  --silent            Run without console output\n";
    std::cout << "  --disable           Disable theme (close mutex, not yet implemented)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  Injector.exe                    # Enable theme with default settings\n";
    std::cout << "  Injector.exe --status           # Check if theme is active\n";
    std::cout << "  Injector.exe --silent           # Enable theme without console output\n";
    std::cout << "  Injector.exe --help             # Show this help message\n\n";
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
                Print("Note: --config flag is recognized but custom theme path not yet implemented.");
            } else {
                PrintError("Error: --config requires a file path argument");
                g_helpRequested = true;
                return;
            }
        }
        else if (arg == "--disable") {
            Print("Note: --disable flag is recognized but not yet implemented.");
            Print("To disable the theme, simply close the injector window.");
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


int main(int argc, char* argv[]) {
    // Parse command-line arguments
    ParseArguments(argc, argv);

    // If help/version was requested, exit cleanly
    if (g_helpRequested) {
        return 0;
    }

    Print(std::string(APP_NAME) + " v" + APP_VERSION);

    // Create the mutex that signals the theme should be active.
    const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
    HANDLE hMutex = CreateMutexW(NULL, TRUE, mutexName);
    if (hMutex == NULL) {
        PrintError("Error: Could not create the theme mutex.");
        return 1;
    }

    // Check if mutex already existed (theme already active)
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Print("Warning: Theme mutex already exists. Another instance may be running.");
        Print("Proceeding with injection anyway...");
    }

    Print("Searching for Event Viewer (mmc.exe) process...");

    // 1. Find the target process
    std::vector<DWORD> mmcPids = GetPidsByName(L"mmc.exe");
    if (mmcPids.empty()) {
        PrintError("Error: No mmc.exe process found.");
        PrintError("Please open Event Viewer (eventvwr.msc) first.");
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
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
        PrintError("Error: Found mmc.exe, but none are hosting 'Event Viewer'.");
        PrintError("Please make sure Event Viewer is running.");
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    Print("Found Event Viewer process with PID: " + std::to_string(targetPid));

    // 2. Define the path to our DLL
    // This path assumes the DLL is in the same directory as the injector.
    char dllPath[MAX_PATH];
    GetFullPathNameA("ThemeEngine.dll", MAX_PATH, dllPath, NULL);
    Print("Injecting DLL: " + std::string(dllPath));

    // 3. Get a handle to the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
    if (hProcess == NULL) {
        PrintError("Error: Could not open process. Try running as administrator.");
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 4. Allocate memory in the target process for the DLL path
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteMem == NULL) {
        PrintError("Error: Could not allocate memory in target process.");
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 5. Write the DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath, sizeof(dllPath), NULL)) {
        PrintError("Error: Could not write to target process memory.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 6. Get the address of the LoadLibraryA function
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        PrintError("Error: Could not find LoadLibraryA address.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 7. Create a remote thread in the target process to load our DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemoteMem, 0, NULL);
    if (hRemoteThread == NULL) {
        PrintError("Error: Could not create remote thread.");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    Print("Injection successful. Waiting for remote thread to finish...");
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Check if DLL was actually loaded
    DWORD exitCode = 0;
    GetExitCodeThread(hRemoteThread, &exitCode);
    if (exitCode == 0) {
        PrintError("Error: DLL injection failed - LoadLibraryA returned NULL");
        PrintError("The DLL may not be in the correct location or failed to initialize");
        PrintError("Check C:\\ThemeEngine_debug.log for details");
        CloseHandle(hRemoteThread);
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
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
    Print("Check C:\\ThemeEngine_debug.log for detailed initialization status");

    // Force Event Viewer window to refresh
    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        Print("Refreshing Event Viewer window...");
        InvalidateRect(hEventViewer, NULL, TRUE);
        RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        Print("Window refreshed. The theme should now be visible.");
    }

    Print("Keep this window open to maintain the theme.");
    Print("Closing this window will deactivate the theme.");

    // We keep the injector running to hold the mutex open.
    // When this injector is closed, the mutex is destroyed, and the theme will
    // no longer be applied on the next paint cycle.
    if (!g_silent) {
        system("pause");
    } else {
        // In silent mode, wait indefinitely (user must close via Task Manager or Ctrl+C)
        Print("Running in silent mode. Press Ctrl+C to exit.");
        WaitForSingleObject(hMutex, INFINITE); // Wait forever
    }

    // Clean up
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return 0;
}
