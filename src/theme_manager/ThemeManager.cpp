#include "ThemeManager.h"
#include "../../libs/json/json.hpp"
#include <fstream>
#include <shlobj.h>

using json = nlohmann::json;

namespace Shades {

ThemeManager::ThemeManager()
    : m_activeThemeIndex(-1) {
    InitThemesDirectory();
}

ThemeManager::~ThemeManager() {}

void ThemeManager::InitThemesDirectory() {
    // Use a "themes" subdirectory next to the executable
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) *(lastSlash + 1) = L'\0';
    m_themesDirectory = std::wstring(exePath) + L"themes\\";

    // Create themes directory if it doesn't exist
    CreateDirectoryW(m_themesDirectory.c_str(), NULL);
}

void ThemeManager::LoadThemesFromDirectory(const std::wstring& dir) {
    if (!dir.empty()) {
        m_themesDirectory = dir;
        if (m_themesDirectory.back() != L'\\')
            m_themesDirectory += L'\\';
        CreateDirectoryW(m_themesDirectory.c_str(), NULL);
    }

    m_themes.clear();
    m_activeThemeIndex = -1;

    // Search for JSON files in themes directory
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = m_themesDirectory + L"*.json";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::wstring filePath = m_themesDirectory + findData.cFileName;
                Theme theme;
                if (LoadThemeFromFile(filePath, theme)) {
                    m_themes.push_back(theme);
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    // Also check for a theme.json in the executable directory
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) *(lastSlash + 1) = L'\0';
    std::wstring rootTheme = std::wstring(exePath) + L"theme.json";

    // Check if root theme.json exists and isn't already loaded
    if (GetFileAttributesW(rootTheme.c_str()) != INVALID_FILE_ATTRIBUTES) {
        bool alreadyLoaded = false;
        for (const auto& t : m_themes) {
            if (t.filePath == rootTheme) {
                alreadyLoaded = true;
                break;
            }
        }
        if (!alreadyLoaded) {
            Theme theme;
            if (LoadThemeFromFile(rootTheme, theme)) {
                if (theme.name == L"Untitled") {
                    theme.name = L"Default";
                }
                m_themes.insert(m_themes.begin(), theme);
            }
        }
    }

    // Set the first theme as active by default
    if (!m_themes.empty() && m_activeThemeIndex < 0) {
        m_activeThemeIndex = 0;
        m_themes[0].isActive = true;
    }
}

void ThemeManager::ReloadThemes() {
    int previousActive = m_activeThemeIndex;
    std::wstring previousActiveName;
    if (previousActive >= 0 && previousActive < (int)m_themes.size()) {
        previousActiveName = m_themes[previousActive].name;
    }

    LoadThemesFromDirectory(m_themesDirectory);

    // Restore active theme by name
    if (!previousActiveName.empty()) {
        for (int i = 0; i < (int)m_themes.size(); i++) {
            if (m_themes[i].name == previousActiveName) {
                SetActiveTheme(i);
                break;
            }
        }
    }
}

const Theme* ThemeManager::GetTheme(int index) const {
    if (index < 0 || index >= (int)m_themes.size()) return nullptr;
    return &m_themes[index];
}

Theme* ThemeManager::GetTheme(int index) {
    if (index < 0 || index >= (int)m_themes.size()) return nullptr;
    return &m_themes[index];
}

bool ThemeManager::CreateNewTheme(const std::wstring& name, const std::wstring& author) {
    Theme theme;
    theme.name = name;
    theme.author = author;
    theme.version = L"1.0";
    theme.colors = GetDefaultColors();
    theme.isActive = false;
    theme.isPackage = false;

    // Generate a safe filename
    std::wstring safeName = name;
    for (auto& c : safeName) {
        if (c == L' ' || c == L'\\' || c == L'/' || c == L':' || c == L'*' ||
            c == L'?' || c == L'"' || c == L'<' || c == L'>' || c == L'|') {
            c = L'_';
        }
    }

    std::wstring filePath = m_themesDirectory + safeName + L".json";

    // Check if file already exists
    if (GetFileAttributesW(filePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        // Append a number
        int counter = 1;
        do {
            wchar_t suffix[16];
            swprintf_s(suffix, L"_%d", counter++);
            filePath = m_themesDirectory + safeName + suffix + L".json";
        } while (GetFileAttributesW(filePath.c_str()) != INVALID_FILE_ATTRIBUTES);
    }

    theme.filePath = filePath;

    if (!WriteThemeToFile(theme, filePath)) {
        return false;
    }

    return true;
}

bool ThemeManager::DeleteTheme(int index) {
    if (index < 0 || index >= (int)m_themes.size()) {
        m_lastError = L"Invalid theme index.";
        return false;
    }

    const Theme& theme = m_themes[index];

    // Delete the file
    if (!DeleteFileW(theme.filePath.c_str())) {
        m_lastError = L"Failed to delete theme file: " + theme.filePath;
        return false;
    }

    m_themes.erase(m_themes.begin() + index);

    // Adjust active theme index
    if (m_activeThemeIndex == index) {
        m_activeThemeIndex = -1;
    } else if (m_activeThemeIndex > index) {
        m_activeThemeIndex--;
    }

    return true;
}

bool ThemeManager::DuplicateTheme(int index) {
    if (index < 0 || index >= (int)m_themes.size()) {
        m_lastError = L"Invalid theme index.";
        return false;
    }

    Theme copy = m_themes[index];
    copy.name = L"Copy of " + copy.name;
    copy.isActive = false;

    return CreateNewTheme(copy.name, copy.author);
}

void ThemeManager::SetActiveTheme(int index) {
    // Clear previous active
    for (auto& theme : m_themes) {
        theme.isActive = false;
    }

    if (index >= 0 && index < (int)m_themes.size()) {
        m_themes[index].isActive = true;
        m_activeThemeIndex = index;
    } else {
        m_activeThemeIndex = -1;
    }
}

bool ThemeManager::SaveModifiedTheme(int index) {
    if (index < 0 || index >= (int)m_themes.size()) {
        m_lastError = L"Invalid theme index.";
        return false;
    }

    return WriteThemeToFile(m_themes[index], m_themes[index].filePath);
}

bool ThemeManager::SaveTheme(const Theme& theme, const std::wstring& filePath) {
    return WriteThemeToFile(theme, filePath);
}

bool ThemeManager::LoadThemeFromFile(const std::wstring& filePath, Theme& theme) {
    // Convert to narrow string for ifstream
    char narrowPath[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, narrowPath, MAX_PATH, NULL, NULL);

    std::ifstream f(narrowPath);
    if (!f.is_open()) {
        m_lastError = L"Could not open file: " + filePath;
        return false;
    }

    try {
        json data = json::parse(f);

        theme.filePath = filePath;
        theme.isPackage = false;
        theme.isActive = false;

        // Read metadata if present
        if (data.contains("name")) {
            std::string name = data["name"];
            theme.name = std::wstring(name.begin(), name.end());
        } else {
            // Derive name from filename
            size_t lastSlash = filePath.find_last_of(L"\\/");
            size_t lastDot = filePath.find_last_of(L".");
            if (lastSlash != std::wstring::npos && lastDot != std::wstring::npos) {
                theme.name = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
            }
        }

        if (data.contains("author")) {
            std::string author = data["author"];
            theme.author = std::wstring(author.begin(), author.end());
        }

        if (data.contains("version")) {
            std::string ver = data["version"];
            theme.version = std::wstring(ver.begin(), ver.end());
        }

        // Read colors
        theme.colors.clear();
        if (data.contains("colors") && data["colors"].is_object()) {
            for (auto& [key, value] : data["colors"].items()) {
                if (value.is_string()) {
                    theme.colors[key] = value.get<std::string>();
                }
            }
        }

        // Get file modification time
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;
        if (GetFileAttributesExW(filePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
            theme.modifiedDate = fileInfo.ftLastWriteTime;
        }

        return true;
    } catch (const std::exception& e) {
        std::string err = e.what();
        m_lastError = L"JSON parse error: " + std::wstring(err.begin(), err.end());
        return false;
    }
}

bool ThemeManager::WriteThemeToFile(const Theme& theme, const std::wstring& filePath) {
    try {
        json data;

        // Write metadata (narrow wstring to string for JSON)
        auto narrow = [](const std::wstring& ws) -> std::string {
            std::string s;
            s.reserve(ws.size());
            for (wchar_t wc : ws) s.push_back(static_cast<char>(wc));
            return s;
        };
        std::string name = narrow(theme.name);
        std::string author = narrow(theme.author);
        std::string ver = narrow(theme.version);
        data["name"] = name;
        data["author"] = author;
        data["version"] = ver;

        // Write colors
        json colors;
        for (const auto& [key, value] : theme.colors) {
            colors[key] = value;
        }
        data["colors"] = colors;

        // Write to file
        char narrowPath[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, narrowPath, MAX_PATH, NULL, NULL);

        std::ofstream f(narrowPath);
        if (!f.is_open()) {
            m_lastError = L"Could not write to file: " + filePath;
            return false;
        }

        f << data.dump(2);
        f.close();
        return true;
    } catch (const std::exception& e) {
        std::string err = e.what();
        m_lastError = L"Failed to write theme: " + std::wstring(err.begin(), err.end());
        return false;
    }
}

std::map<std::string, std::string> ThemeManager::GetDefaultColors() const {
    return {
        {"window_bg",      "#1E1E1E"},
        {"window_text",    "#D4D4D4"},
        {"highlight_bg",   "#2A2D2E"},
        {"highlight_text", "#FFFFFF"},
        {"button_face",    "#333333"},
        {"button_text",    "#D4D4D4"},
        {"header_bg",      "#333333"},
        {"scrollbar_bg",   "#1E1E1E"},
        {"scrollbar_thumb","#424242"},
        {"menubar_bg",     "#333333"},
        {"menubar_text",   "#D4D4D4"},
        {"statusbar_bg",   "#007ACC"},
        {"statusbar_text", "#FFFFFF"}
    };
}

} // namespace Shades
