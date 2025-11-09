# Project Plan: Event Viewer Themer - Future Enhancements

## Current Status

**Core Project**: ✅ 100% Complete (see PROJECT_PLAN_COMPLETED.md)
**Version**: 1.0 (Distribution ready)
**This Plan**: Future enhancements and improvements for v2.0+

---

## Phase 4: Testing & Quality Assurance

**Objective**: Ensure reliability and compatibility across different Windows environments and configurations.

### Task 4.1: Cross-Platform Windows Testing

**Priority**: High

**Testing Scope**:
- [ ] Windows 10 (versions 21H1, 21H2, 22H2)
- [ ] Windows 11 (versions 21H2, 22H2, 23H2)
- [ ] Windows Server 2019
- [ ] Windows Server 2022
- [ ] Both fresh installs and heavily customized systems

**Validation Criteria**:
- Theme applies correctly on all tested platforms
- No crashes or memory leaks during extended runtime (24+ hours)
- Proper cleanup when injector is closed
- No interference with Event Viewer functionality

### Task 4.2: Edge Case & Error Handling

**Priority**: Medium

**Test Scenarios**:
- [ ] Event Viewer opened multiple times simultaneously
- [ ] Injector run multiple times (should handle gracefully)
- [ ] Malformed theme.json files (missing keys, invalid colors)
- [ ] Very long Event Viewer sessions (stress testing)
- [ ] Rapid enable/disable toggling

**Improvements Needed**:
- Add comprehensive error messages to console
- Implement theme.json validation with fallback to defaults
- Prevent multiple injection attempts on same process
- Add logging system for debugging issues

### Task 4.3: Security & Code Review

**Priority**: High

**Actions**:
- [ ] Static code analysis with Codacy or SonarQube
- [ ] Memory leak detection with Valgrind or Dr. Memory
- [ ] Code signing certificate acquisition for executables
- [ ] Security audit of injection mechanism
- [ ] Verify proper privilege handling (Administrator requirements)

---

## Phase 5: Enhanced User Experience

**Objective**: Make the application more user-friendly and accessible to non-technical users.

### Task 5.1: GUI Configuration Tool

**Priority**: High

**Features to Implement**:
- [ ] Native Windows GUI (Win32 or Qt)
- [ ] Visual theme editor with color picker
- [ ] Real-time preview of color changes
- [ ] Theme preset library (Dark, Dracula, Monokai, Nord, etc.)
- [ ] Import/Export custom themes
- [ ] "Apply" and "Reset" buttons

**Technical Requirements**:
- New executable: ThemeConfigurator.exe
- Reads and writes theme.json
- Can launch Injector.exe with proper parameters
- Integrates with existing command-line injector

### Task 5.2: System Tray Integration

**Priority**: Medium

**Features to Implement**:
- [ ] System tray icon for Injector.exe
- [ ] Right-click context menu with options:
  - "Enable Theme"
  - "Disable Theme"
  - "Configure Theme..."
  - "About"
  - "Exit"
- [ ] Visual indicator for theme status (enabled/disabled)
- [ ] Minimize to tray instead of taskbar
- [ ] Start minimized option

**Technical Requirements**:
- Modify Injector.cpp to create hidden window
- Implement WM_TRAYICON message handling
- Add icon resources (16x16, 32x32, 48x48)
- Persist tray icon while theme is active

### Task 5.3: Command-Line Arguments ✅ COMPLETED

**Priority**: Medium

**Completion**: Full command-line argument support implemented in Injector.cpp v1.1.0.

**Arguments Implemented**:
```
Injector.exe [options]

✅ Implemented:
  --help, -h, /?    Display help message
  --version, -v     Display version information
  --status          Check if theme is currently active (via mutex detection)
  --silent, -s      Run without console output

⏳ Recognized (not yet implemented):
  --config <path>   Use custom theme.json file (placeholder)
  --disable         Disable theme (placeholder - currently just close window)
```

**Technical Implementation**:
- ✅ Custom lightweight argument parser (no external dependencies)
- ✅ Modified main() function to parse argc/argv
- ✅ IsThemeActive() function checks mutex via OpenMutexW()
- ✅ Silent mode flag that suppresses all console output
- ✅ Comprehensive help text with examples
- ✅ Version display with copyright and license info
- ✅ Improved error messages throughout
- ✅ Warning for duplicate injector instances

**Features Added**:
- Version bumped to 1.1.0
- Print() and PrintError() functions respect --silent flag
- Better user guidance in error messages
- Multiple argument aliases (-h, -v, -s, /?)
- Exit codes for scripting support

### Task 5.4: Auto-Start with Windows

**Priority**: Low

**Features to Implement**:
- [ ] Registry integration for startup
- [ ] Option to "Start with Windows" in GUI/tray menu
- [ ] Start minimized to tray on boot
- [ ] Auto-inject when Event Viewer opens

**Technical Requirements**:
- Add registry key: `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
- Implement registry write/delete functions
- Add permission checks (may require elevation)
- Provide uninstall/disable option

---

## Phase 6: Extended Functionality

**Objective**: Expand beyond Event Viewer to support additional Windows applications.

### Task 6.1: Multi-Application Support

**Priority**: Medium

**Target Applications**:
- [ ] Performance Monitor (perfmon.msc)
- [ ] Device Manager (devmgmt.msc)
- [ ] Disk Management (diskmgmt.msc)
- [ ] Services (services.msc)
- [ ] Task Scheduler (taskschd.msc)
- [ ] Computer Management (compmgmt.msc)

**Technical Approach**:
- Generalize process detection (search for multiple window titles)
- Allow multiple theme profiles per application
- Single Injector instance managing multiple processes
- Configuration file extension for multi-app support

### Task 6.2: Theme Marketplace & Sharing

**Priority**: Low

**Features to Implement**:
- [ ] Online theme repository (GitHub-based or custom)
- [ ] One-click theme import from URL
- [ ] Community-contributed themes
- [ ] Theme rating and reviews
- [ ] Screenshot preview for themes

**Technical Requirements**:
- Web API for theme hosting
- JSON schema validation for uploaded themes
- GUI integration for browsing themes
- Local theme cache management

### Task 6.3: Advanced Customization

**Priority**: Low

**Features to Implement**:
- [ ] Font family and size customization
- [ ] Custom icon sets for Event Viewer
- [ ] Transparency/opacity controls
- [ ] Column width and layout persistence
- [ ] Conditional formatting (e.g., color-code errors red)

**Technical Requirements**:
- Hook additional GDI functions (SetTextColor, SelectObject)
- Extended theme.json schema with font properties
- Icon resource replacement mechanism
- Per-item color logic based on event type

---

## Phase 7: Distribution & Community

**Objective**: Professional distribution and open-source community building.

### Task 7.1: Installer Creation

**Priority**: High

**Installer Features**:
- [ ] Professional installer using NSIS or WiX Toolset
- [ ] Start menu shortcuts
- [ ] Desktop shortcut (optional)
- [ ] Automatic dependency detection (Windows SDK, VC++ Runtime)
- [ ] Uninstaller with complete cleanup
- [ ] Silent install option for enterprise deployment

**Technical Requirements**:
- NSIS script or WiX XML configuration
- Version checking and upgrade support
- Registry cleanup on uninstall
- File association for .theme files

### Task 7.2: Code Signing & Release Management

**Priority**: High

**Actions Needed**:
- [ ] Acquire code signing certificate (EV certificate preferred)
- [ ] Sign all executables and DLLs
- [ ] Set up GitHub Releases with automatic builds
- [ ] Create release notes template
- [ ] Implement semantic versioning (v2.0.0, v2.1.0, etc.)

**Benefits**:
- Eliminates Windows SmartScreen warnings
- Builds user trust and legitimacy
- Professional appearance in Windows

### Task 7.3: Documentation & Community

**Priority**: Medium

**Documentation to Create**:
- [ ] Comprehensive README.md (see Task 8.1)
- [ ] CONTRIBUTING.md with development guidelines
- [ ] CODE_OF_CONDUCT.md for community standards
- [ ] GitHub issue templates (bug report, feature request)
- [ ] GitHub pull request template
- [ ] Wiki pages with tutorials and FAQ
- [ ] Video tutorial/demo on YouTube

**Community Building**:
- [ ] Set up GitHub Discussions for Q&A
- [ ] Create Discord server for real-time support
- [ ] Reddit community (r/EventViewerThemer or similar)
- [ ] Twitter/X account for updates

### Task 7.4: CI/CD Pipeline

**Priority**: Low

**Automation Goals**:
- [ ] GitHub Actions workflow for automated builds
- [ ] Automated testing on Windows runners
- [ ] Build artifacts for each commit
- [ ] Automatic release creation on version tags
- [ ] Cross-compilation matrix (MinGW, MSVC)

**Workflow Steps**:
1. Code pushed to repository
2. GitHub Actions triggers build
3. Compile with both MSVC and MinGW
4. Run automated tests
5. Package distribution ZIP
6. Upload artifacts
7. Create release if tagged

---

## Phase 8: Documentation ✅ COMPLETED

**Objective**: Create essential documentation for GitHub repository visibility and user onboarding.

### Task 8.1: Create Comprehensive README.md ✅ COMPLETED

**Priority**: CRITICAL (Immediate)

**Completion**: Professional README.md created with dual-audience content covering quick start, customization, build instructions, troubleshooting, FAQ, and technical architecture.

### Task 8.2: Add LICENSE File ✅ COMPLETED

**Priority**: CRITICAL (Immediate)

**Completion**: MIT License added matching dependency licenses (Microsoft Detours and nlohmann/json).

---

## Phase 9: Professional Installer ✅ COMPLETED

**Objective**: Create a professional Windows installer for easy distribution and installation.

### Task 9.1: Installer Technology Selection ✅ COMPLETED

**Priority**: CRITICAL (Immediate)

**Completion**: Selected NSIS (Nullsoft Scriptable Install System) for fast development and lightweight installer.

**Options Analysis**:

**Option A: NSIS (Nullsoft Scriptable Install System)**
- ✅ Pros: Lightweight, widely used, simple scripting, small installer size
- ✅ Pros: Easy to learn, extensive documentation and examples
- ✅ Pros: Supports custom UI, silent install, uninstaller generation
- ❌ Cons: Scripting language is less modern than WiX XML

**Option B: WiX Toolset (Windows Installer XML)**
- ✅ Pros: Industry standard, MSI format, built-in Windows support
- ✅ Pros: Better integration with Windows Installer features
- ✅ Pros: Supports upgrades, patching, component management
- ❌ Cons: Steeper learning curve, more complex setup

**Recommended**: **NSIS** for this project
- Faster development time
- Simpler for single-application installer
- Smaller installer footprint
- Easier customization for our use case

### Task 9.2: Installer Requirements Specification ✅ COMPLETED

**Priority**: CRITICAL (Immediate)

**Completion**: Comprehensive installer specification created with all required features.

**Installation Features**:
- ✅ Install files to `%ProgramFiles%\EventViewerThemer\`
- ✅ Create Start Menu shortcuts
- ✅ Optional Desktop shortcut
- ✅ Install default `theme.json` configuration
- ✅ Create uninstaller with complete cleanup
- ✅ Support silent installation (`/S` flag)
- ✅ Display license agreement during installation
- ✅ Professional installer dialogs (Welcome, License, Directory, Components, Install, Finish)

**Installer Dialogs/Screens**:
1. Welcome screen with project description
2. License agreement (MIT License)
3. Installation directory selection (default: `%ProgramFiles%\EventViewerThemer\`)
4. Shortcuts selection (Start Menu, Desktop)
5. Installation progress
6. Completion screen with "Launch now" option

**Files to Install**:
- `Injector.exe` (162KB)
- `ThemeEngine.dll` (1.6MB)
- `theme.json` (default configuration)
- `README.txt` (quick start guide)
- `LICENSE.txt` (MIT License)
- `Uninstall.exe` (auto-generated by NSIS)

**Registry Entries**:
- Uninstaller registry key: `HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\EventViewerThemer`
- Installation path: `HKLM\Software\EventViewerThemer\InstallPath`

**Start Menu Structure**:
```
Start Menu\Programs\Event Viewer Themer\
├── Event Viewer Themer.lnk (launches Injector.exe as admin)
├── Configure Theme.lnk (opens theme.json in notepad)
├── README.lnk (opens README.txt)
└── Uninstall.lnk
```

### Task 9.3: NSIS Script Development ✅ COMPLETED

**Priority**: CRITICAL (Immediate)

**Completion**: Full NSIS installer script created and compiled successfully. Installer size: 312 KB.

**Implementation Tasks**:
- ✅ Install NSIS on development system (v3.11 via winget)
- ✅ Create `installer.nsi` script file
- ✅ Configure installer metadata (name, version, publisher, icon)
- ✅ Define installation sections and file copying
- ✅ Implement Start Menu and Desktop shortcut creation
- ✅ Add registry entries for uninstaller
- ✅ Create uninstaller section with cleanup logic
- ✅ Add UAC elevation for installer (requires admin for Program Files)
- ✅ Configure installer with MUI2 modern interface
- ✅ Compiled installer: `EventViewerThemer-Setup.exe` (312 KB)

**NSIS Script Structure**:
```nsis
# Installer metadata
Name "Event Viewer Themer"
OutFile "EventViewerThemer-Setup.exe"
InstallDir "$PROGRAMFILES64\EventViewerThemer"

# Version information
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "Event Viewer Themer"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "LegalCopyright" "MIT License"

# Sections: Default installation, Shortcuts, Uninstaller
# Pages: Welcome, License, Directory, InstFiles, Finish
```

### Task 9.4: Installer Testing & Validation ⏳ READY FOR TESTING

**Priority**: HIGH (Next step)

**Status**: Installer compiled and ready for user testing

**Test Scenarios**:
- [ ] Fresh installation on Windows 10
- [ ] Fresh installation on Windows 11
- [ ] Installation to custom directory
- [ ] Silent installation (`/S` flag)
- [ ] Upgrade over existing installation
- [ ] Complete uninstallation (verify no leftover files/registry)
- [ ] Install without admin privileges (should fail gracefully)
- [ ] Multiple install/uninstall cycles

**Validation Checklist**:
- [ ] All files copied correctly
- [ ] Shortcuts work and launch with admin privileges
- [ ] Uninstaller removes all files and registry entries
- [ ] Installation size matches expectations (~2MB)
- [ ] Installer is digitally signed (future: after code signing cert acquired)

### Task 9.5: Installer Distribution ⏳ READY FOR RELEASE

**Priority**: MEDIUM (Immediate)

**Status**: Files ready for GitHub Release

**Distribution Channels**:
- ⏳ Upload `EventViewerThemer-Setup.exe` to GitHub Releases (pending user action)
- ✅ Update README.md with installer download link
- ✅ Create installation instructions for installer version
- ✅ Provide checksums (SHA256) for installer verification in `SHA256SUMS.txt`
- ⏳ Tag release as v1.1 (installer release) (pending user action)

**Documentation Updates**:
- ✅ Add "Installation via Installer" section to README
- ✅ Update Quick Start guide to reference installer
- ✅ Document uninstallation process
- ✅ Add installer troubleshooting to README

**Files Ready for Distribution**:
- `dist/EventViewerThemer-Setup.exe` (312 KB)
- `dist/SHA256SUMS.txt` (checksum verification)
- `dist/EventViewerThemer.zip` (portable version)
- `installer.nsi` (source script for reproducible builds)

---

## Timeline & Milestones

### Immediate (Current Sprint) - INSTALLER COMPLETE ✅
- ✅ Archive completed project plan
- ✅ Create comprehensive README.md (Task 8.1)
- ✅ Add MIT LICENSE file (Task 8.2)
- ✅ Publish to GitHub with complete documentation
- ✅ **Task 9.1-9.3: Professional NSIS Installer Creation (COMPLETED)**
  - ✅ Installed NSIS v3.11
  - ✅ Created installer.nsi script with MUI2 interface
  - ✅ Compiled EventViewerThemer-Setup.exe (312 KB)
  - ✅ Generated SHA256 checksums
  - ✅ Updated README with installer instructions
- ⏳ **Task 9.4-9.5: Testing & GitHub Release (NEXT)**
  - User testing on Windows 10/11
  - Create GitHub Release v1.1

### Short Term (1-2 months)
- ✅ Task 5.3: Command-line arguments implementation (COMPLETED)
- Task 4.1: Windows platform testing
- Task 4.3: Security audit and code signing

### Medium Term (3-6 months)
- Task 5.1: GUI configuration tool
- Task 5.2: System tray integration
- Task 4.2: Enhanced error handling
- Task 7.2: GitHub Releases automation

### Long Term (6-12 months)
- Task 6.1: Multi-application support
- Task 5.4: Auto-start functionality
- Task 6.3: Advanced customization features
- Task 7.3: Community building

### Future Roadmap
- Task 6.2: Theme marketplace
- Task 7.4: Full CI/CD pipeline
- Major version 3.0 with architectural improvements

---

## Success Metrics

### User Adoption
- GitHub stars: 100+ (6 months), 500+ (1 year)
- Downloads: 1,000+ (6 months), 5,000+ (1 year)
- Active forks: 10+ (6 months)

### Quality Metrics
- Zero critical bugs reported
- Average issue resolution time: < 7 days
- Code coverage: > 70%
- Security vulnerabilities: 0

### Community Health
- Active contributors: 5+ (6 months)
- Pull requests merged: 10+ (6 months)
- Documentation completeness: 100%
- User satisfaction: 4.5+ stars average

---

## Contributing to This Plan

This project plan is a living document. If you're a contributor with ideas for new features or improvements:

1. Open a GitHub Discussion to propose the feature
2. Get community feedback on the idea
3. Update this plan via pull request
4. Implement the feature with proper testing
5. Update documentation accordingly

**Let's build something amazing together!**
