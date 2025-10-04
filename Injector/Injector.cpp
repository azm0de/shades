#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

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


int main() {
    std::cout << "Event Viewer Themer" << std::endl;

    // Create the mutex that signals the theme should be active.
    const wchar_t* mutexName = L"Global\\EventViewerThemeActive";
    HANDLE hMutex = CreateMutexW(NULL, TRUE, mutexName);
    if (hMutex == NULL) {
        std::cerr << "Error: Could not create the theme mutex." << std::endl;
        return 1;
    }

    std::cout << "Searching for Event Viewer (mmc.exe) process..." << std::endl;

    // 1. Find the target process
    std::vector<DWORD> mmcPids = GetPidsByName(L"mmc.exe");
    if (mmcPids.empty()) {
        std::cerr << "Error: No mmc.exe process found." << std::endl;
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
        std::cerr << "Error: Found mmc.exe, but none are hosting 'Event Viewer'." << std::endl;
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    std::cout << "Found Event Viewer process with PID: " << targetPid << std::endl;

    // 2. Define the path to our DLL
    // This path assumes the DLL is in the same directory as the injector.
    char dllPath[MAX_PATH];
    GetFullPathNameA("ThemeEngine.dll", MAX_PATH, dllPath, NULL);
    std::cout << "Injecting DLL: " << dllPath << std::endl;

    // 3. Get a handle to the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
    if (hProcess == NULL) {
        std::cerr << "Error: Could not open process. Try running as administrator." << std::endl;
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 4. Allocate memory in the target process for the DLL path
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteMem == NULL) {
        std::cerr << "Error: Could not allocate memory in target process." << std::endl;
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 5. Write the DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath, sizeof(dllPath), NULL)) {
        std::cerr << "Error: Could not write to target process memory." << std::endl;
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
        std::cerr << "Error: Could not find LoadLibraryA address." << std::endl;
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    // 7. Create a remote thread in the target process to load our DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemoteMem, 0, NULL);
    if (hRemoteThread == NULL) {
        std::cerr << "Error: Could not create remote thread." << std::endl;
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    std::cout << "Injection successful. Waiting for remote thread to finish..." << std::endl;
    WaitForSingleObject(hRemoteThread, INFINITE);

    // 8. Clean up handles
    std::cout << "Cleaning up handles." << std::endl;
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    std::cout << "Process complete. Theme is active." << std::endl;

    // Force Event Viewer window to refresh
    HWND hEventViewer = FindWindowW(L"MMCMainFrame", L"Event Viewer");
    if (hEventViewer) {
        std::cout << "Refreshing Event Viewer window..." << std::endl;
        InvalidateRect(hEventViewer, NULL, TRUE);
        RedrawWindow(hEventViewer, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        std::cout << "Window refreshed. The theme should now be visible." << std::endl;
    }

    std::cout << "Keep this window open to maintain the theme." << std::endl;
    std::cout << "Closing this window will deactivate the theme." << std::endl;

    // We keep the injector running to hold the mutex open.
    // When this injector is closed, the mutex is destroyed, and the theme will
    // no longer be applied on the next paint cycle.
    system("pause");

    // Clean up
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return 0;
}
