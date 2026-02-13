#ifndef SHADES_THEMEMANAGER_H
#define SHADES_THEMEMANAGER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <cstdio>

namespace Shades {

//=============================================================================
// Theme - Represents a single theme with colors and metadata
//=============================================================================
struct Theme {
    // Metadata
    std::wstring name;
    std::wstring author;
    std::wstring version;
    std::wstring filePath;
    FILETIME modifiedDate;
    bool isActive;
    bool isPackage;

    // Color properties (key -> hex color string, e.g. "window_bg" -> "#1E1E1E")
    std::map<std::string, std::string> colors;

    // Get color as COLORREF, with fallback default
    COLORREF GetColor(const std::string& key, COLORREF defaultColor) const {
        auto it = colors.find(key);
        if (it == colors.end()) return defaultColor;
        const std::string& hex = it->second;
        if (hex.size() < 6) return defaultColor;
        const char* p = hex.c_str();
        if (*p == '#') p++;
        unsigned int r = 0, g = 0, b = 0;
        if (sscanf(p, "%02x%02x%02x", &r, &g, &b) == 3) {
            return RGB(r, g, b);
        }
        return defaultColor;
    }

    Theme()
        : name(L"Untitled")
        , author(L"Unknown")
        , version(L"1.0")
        , modifiedDate({0, 0})
        , isActive(false)
        , isPackage(false) {}
};

//=============================================================================
// ThemeManager - Manages theme collection and persistence
//=============================================================================
class ThemeManager {
public:
    ThemeManager();
    ~ThemeManager();

    // Load all themes from the themes directory
    void LoadThemesFromDirectory(const std::wstring& dir = L"");

    // Reload themes from disk
    void ReloadThemes();

    // Get all themes
    const std::vector<Theme>& GetThemes() const { return m_themes; }

    // Get theme by index (returns nullptr if out of range)
    const Theme* GetTheme(int index) const;
    Theme* GetTheme(int index);

    // Get theme count
    size_t GetThemeCount() const { return m_themes.size(); }

    // Create a new theme with default colors
    bool CreateNewTheme(const std::wstring& name, const std::wstring& author = L"Unknown");

    // Delete theme by index
    bool DeleteTheme(int index);

    // Duplicate theme
    bool DuplicateTheme(int index);

    // Set active theme
    void SetActiveTheme(int index);

    // Save modified theme back to its file
    bool SaveModifiedTheme(int index);

    // Save theme to specific path
    bool SaveTheme(const Theme& theme, const std::wstring& filePath);

    // Get themes directory
    std::wstring GetThemesDirectory() const { return m_themesDirectory; }

    // Get last error message
    std::wstring GetLastError() const { return m_lastError; }

private:
    // Load a single theme from a JSON file
    bool LoadThemeFromFile(const std::wstring& filePath, Theme& theme);

    // Write a theme to a JSON file
    bool WriteThemeToFile(const Theme& theme, const std::wstring& filePath);

    // Get default colors for a new theme
    std::map<std::string, std::string> GetDefaultColors() const;

    // Determine themes directory path
    void InitThemesDirectory();

    std::vector<Theme> m_themes;
    std::wstring m_themesDirectory;
    std::wstring m_lastError;
    int m_activeThemeIndex;
};

} // namespace Shades

#endif // SHADES_THEMEMANAGER_H
