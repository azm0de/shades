# SHADES - Project Roadmap

## Current Status

**Core (Injector + ThemeEngine):** v1.2.0 -- Stable, all features working
**GUI:** v2.0.0-alpha -- Compiles, core features implemented
**Installer:** v1.2.0 -- NSIS installer ready

---

## Completed

- ThemeEngine DLL with API hooking (GetSysColor, GetSysColorBrush, DrawThemeBackground, DrawText, CreateWindowEx)
- Window subclassing: ListView, TreeView, TabControl, parent windows
- ListView NM_CUSTOMDRAW via parent window subclass
- Hot-reload via 500ms file timestamp polling
- DLL injector targeting mmc.exe (Event Viewer)
- System tray icon with toggle/configure/exit menu
- Auto-launch Event Viewer prompt
- CLI arguments: --help, --version, --status, --silent, --config, --disable
- IPC via named mutex (Global\EventViewerThemeActive)
- JSON theme configuration with nlohmann/json
- GUI v2.0: color editor, preview panel, button bar, theme gallery
- ThemeManager: load/save/create/delete/duplicate themes
- NSIS installer with Start Menu shortcuts and uninstaller
- Build scripts with Visual Studio auto-detection

---

## Remaining Work

### Testing & QA

- [ ] Full manual test pass on Windows 10 (theme apply, persist, toggle, hot-reload)
- [ ] Windows 11 testing
- [ ] 24-hour soak test for memory leaks
- [ ] Rapid toggle stress test
- [ ] Installer install/uninstall cycle testing
- [ ] Antivirus/SmartScreen compatibility testing

### Polish

- [ ] Code signing certificate (eliminates SmartScreen warnings)
- [ ] GitHub Actions CI/CD (auto-build on push, release on tag)
- [ ] Regenerate SHA256SUMS.txt with current binaries
- [ ] Create GitHub Release v1.2

### Future Enhancements

- [ ] Multi-application support (perfmon, devmgmt, services, etc.)
- [ ] Auto-start with Windows (registry integration)
- [ ] Theme marketplace / sharing
- [ ] Font and icon customization
- [ ] .shadetheme package format (ZIP with theme.json + metadata)
- [ ] Settings dialog in GUI (theme directory, auto-save, animations)
- [ ] Update checker via GitHub API
