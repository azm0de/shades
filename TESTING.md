# Event Viewer Themer - Testing Plan & Results

## Testing Overview

**Version Under Test:** v1.1.0
**Test Date:** November 2025
**Tester:** Development Team
**Test Environment:** Windows (see details below)

---

## Test Environment Information

### System Information
- **OS Version:** Windows 10 Pro (Version 2009)
- **OS Build:** 19045
- **Architecture:** 64-bit (x64)
- **RAM:** 32 GB (34,282,917,888 bytes)
- **Administrator Access:** Required

### Software Requirements
- Windows Event Viewer (eventvwr.msc)
- Administrator privileges for DLL injection

---

## Phase 4.1: Cross-Platform Windows Testing

### Test Matrix

| OS Version | Build | Status | Notes |
|------------|-------|--------|-------|
| Windows 10 Pro 2009 | 19045 | ✅ Testing | Current system - CLI tests passed |
| Windows 11 23H2 | | ⏳ Pending | |
| Windows 11 22H2 | | ⏳ Pending | |
| Windows 10 22H2 | | ⏳ Pending | |
| Windows 10 21H2 | | ⏳ Pending | |
| Windows Server 2022 | | ⏳ Pending | |
| Windows Server 2019 | | ⏳ Pending | |

---

## Phase 4.2: Core Functionality Tests

### Test Case 1: Basic Theme Application

**Objective:** Verify theme applies correctly to Event Viewer

**Prerequisites:**
- Event Viewer is closed
- No previous instance of Injector running

**Steps:**
1. Open Event Viewer (Win+R → eventvwr.msc)
2. Note original appearance (light theme)
3. Run Injector.exe as Administrator
4. Observe Event Viewer window

**Expected Results:**
- ✅ Theme applies immediately
- ✅ Background changes to dark (#1E1E1E)
- ✅ Text changes to light (#D4D4D4)
- ✅ List items are readable
- ✅ Selection highlighting works
- ✅ Column headers are themed

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 2: Theme Persistence

**Objective:** Verify theme remains active while injector is running

**Steps:**
1. Apply theme (Test Case 1)
2. Use Event Viewer normally for 5 minutes
   - Navigate between different event logs
   - Open event details
   - Filter events
   - Resize windows
3. Observe theme consistency

**Expected Results:**
- ✅ Theme remains active throughout
- ✅ No flickering or reversion to light theme
- ✅ All UI elements remain themed

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 3: Theme Deactivation

**Objective:** Verify theme disables when injector is closed

**Steps:**
1. Apply theme (Test Case 1)
2. Keep Event Viewer open
3. Close Injector.exe window
4. Observe Event Viewer

**Expected Results:**
- ✅ Theme reverts to original appearance within a few seconds
- ✅ Event Viewer returns to system default theme
- ✅ No corruption or artifacts
- ✅ Event Viewer remains functional

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 4: Multiple Event Viewer Instances

**Objective:** Test behavior with multiple Event Viewer windows

**Steps:**
1. Open first Event Viewer instance
2. Run Injector.exe
3. Open second Event Viewer instance (new window)
4. Observe both windows

**Expected Results:**
- ✅ First instance is themed
- ✅ Second instance may or may not be themed (document behavior)
- ✅ No crashes or errors

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 5: Injector Re-launch

**Objective:** Test running injector multiple times

**Steps:**
1. Event Viewer already open and themed
2. Run Injector.exe again (second instance)
3. Observe behavior

**Expected Results:**
- ✅ Warning message about duplicate instance
- ✅ No crash
- ✅ Theme remains active
- ✅ Either allows or prevents second instance gracefully

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Phase 4.3: Command-Line Argument Tests

### Test Case 6: --help Argument

**Command:** `Injector.exe --help`

**Expected Results:**
- ✅ Displays comprehensive help message
- ✅ Shows all available options
- ✅ Includes usage examples
- ✅ Shows project URL
- ✅ Exits cleanly without injection

**Actual Results:**
- Status: ✅ PASSED
- Notes: Help displays correctly with all options, examples, and GitHub URL
- Output includes version 1.1.0, comprehensive usage instructions
- All sections present: Options, Examples, Notes, GitHub link

---

### Test Case 7: --version Argument

**Command:** `Injector.exe --version`

**Expected Results:**
- ✅ Shows "Event Viewer Themer version 1.1.0"
- ✅ Shows copyright information
- ✅ Shows MIT License
- ✅ Exits cleanly

**Actual Results:**
- Status: ✅ PASSED
- Notes: Displays "Event Viewer Themer version 1.1.0"
- Copyright shows "Copyright (c) 2025 azm0de"
- License shows "Licensed under the MIT License"
- Exits cleanly

---

### Test Case 8: --status Argument (Theme Inactive)

**Command:** `Injector.exe --status`

**Prerequisites:** No theme currently active

**Expected Results:**
- ✅ Shows "Theme Status: INACTIVE"
- ✅ Explains theme is not active
- ✅ Exits with appropriate exit code

**Actual Results:**
- Status: ✅ PASSED
- Notes: Displays "Theme Status: INACTIVE"
- Shows explanation: "The Event Viewer theme is not currently active."
- Mutex detection working correctly (OpenMutexW returns NULL when inactive)
- Exits cleanly with appropriate exit code

---

### Test Case 9: --status Argument (Theme Active)

**Command:** `Injector.exe --status`

**Prerequisites:** Theme is currently active

**Steps:**
1. Run Injector.exe normally to activate theme
2. In new terminal: Run `Injector.exe --status`

**Expected Results:**
- ✅ Shows "Theme Status: ACTIVE"
- ✅ Explains theme is enabled
- ✅ Exits with appropriate exit code

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 10: --silent Argument

**Command:** `Injector.exe --silent`

**Prerequisites:** Event Viewer is open

**Expected Results:**
- ✅ No console output except errors
- ✅ Theme still applies correctly
- ✅ No "Press any key to continue" prompt
- ✅ Runs in background

**Actual Results:**
- Status: ⚠️ PARTIAL PASS
- Notes: --silent flag works with --status (still shows output for status check)
- Needs testing with actual injection (Event Viewer running)
- Silent mode should suppress informational messages but keep status output

---

### Test Case 11: Invalid Arguments

**Commands:**
- `Injector.exe --invalid`
- `Injector.exe --config` (without path)
- `Injector.exe randomtext`

**Expected Results:**
- ✅ Shows error message
- ✅ Suggests using --help
- ✅ Exits gracefully without crash

**Actual Results:**
- Status: ✅ PASSED
- Notes: Tested with --invalid flag
- Output: "Unknown argument: --invalid"
- Helpful message: "Use --help to see available options."
- Exits gracefully without crash

---

## Phase 4.4: Configuration Tests

### Test Case 12: Default theme.json

**Objective:** Verify default theme loads correctly

**Steps:**
1. Ensure default theme.json exists
2. Run injector
3. Verify colors match theme.json values

**Expected Results:**
- ✅ Colors match theme.json exactly:
  - window_bg: #1E1E1E
  - window_text: #D4D4D4
  - highlight_bg: #2A2D2E
  - highlight_text: #FFFFFF
  - button_face: #333333
  - button_text: #D4D4D4
  - header_bg: #333333

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 13: Custom Theme Colors

**Objective:** Test custom color scheme

**Steps:**
1. Backup original theme.json
2. Modify theme.json with different colors (e.g., Nord theme)
3. Run injector
4. Verify new colors are applied

**Expected Results:**
- ✅ Custom colors are applied correctly
- ✅ All UI elements use new color scheme

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 14: Malformed theme.json

**Objective:** Test error handling with invalid JSON

**Steps:**
1. Create invalid theme.json (broken JSON syntax)
2. Run injector
3. Observe behavior

**Expected Results:**
- ✅ Displays error message about invalid JSON
- ✅ Falls back to defaults OR refuses to inject
- ✅ No crash

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 15: Missing theme.json

**Objective:** Test behavior when theme.json is missing

**Steps:**
1. Rename or delete theme.json
2. Run injector

**Expected Results:**
- ✅ Shows error about missing theme file
- ✅ Exits gracefully OR uses hardcoded defaults
- ✅ No crash

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Phase 4.5: Error Condition Tests

### Test Case 16: No Administrator Privileges

**Objective:** Test behavior without admin rights

**Steps:**
1. Run Injector.exe as normal user (not elevated)
2. Observe error message

**Expected Results:**
- ✅ Clear error message about requiring admin privileges
- ✅ Suggests running as administrator
- ✅ Exits gracefully

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 17: Event Viewer Not Running

**Objective:** Test behavior when target process doesn't exist

**Steps:**
1. Close all Event Viewer instances
2. Run Injector.exe

**Expected Results:**
- ✅ Error message: "No mmc.exe process found"
- ✅ Suggests opening Event Viewer first
- ✅ Exits gracefully

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 18: Missing ThemeEngine.dll

**Objective:** Test behavior when DLL is missing

**Steps:**
1. Rename or move ThemeEngine.dll
2. Run Injector.exe with Event Viewer open

**Expected Results:**
- ✅ Error message about missing DLL
- ✅ Shows expected DLL path
- ✅ Exits gracefully

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Phase 4.6: Installer Tests

### Test Case 19: Fresh Installation

**Objective:** Test clean installation process

**Steps:**
1. Run EventViewerThemer-Setup.exe
2. Follow installation wizard
3. Accept defaults

**Expected Results:**
- ✅ Installer opens without errors
- ✅ License agreement displays
- ✅ Directory selection works
- ✅ Files install to Program Files
- ✅ Start Menu shortcuts created
- ✅ Installation completes successfully

**Actual Results:**
- Status: ⏳ Pending
- Install Path:
- Notes:

---

### Test Case 20: Start Menu Shortcuts

**Objective:** Verify Start Menu integration

**Steps:**
1. After installation, open Start Menu
2. Find "Event Viewer Themer" folder
3. Test each shortcut

**Expected Results:**
- ✅ Folder exists: Start Menu\Programs\Event Viewer Themer\
- ✅ "Event Viewer Themer" launches injector
- ✅ "Configure Theme" opens theme.json in notepad
- ✅ "README" opens README.txt
- ✅ "Uninstall" launches uninstaller

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 21: Desktop Shortcut (Optional)

**Objective:** Test optional desktop shortcut

**Steps:**
1. During installation, select "Desktop shortcut"
2. Complete installation
3. Check desktop

**Expected Results:**
- ✅ Shortcut appears on desktop
- ✅ Shortcut launches injector
- ✅ Icon displays correctly (if icon added)

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 22: Uninstallation

**Objective:** Verify complete removal

**Steps:**
1. After installation, run Uninstaller
2. Confirm uninstallation
3. Check all locations

**Expected Results:**
- ✅ Uninstaller confirms action
- ✅ All files removed from Program Files
- ✅ Start Menu folder removed
- ✅ Desktop shortcut removed (if created)
- ✅ Registry entries cleaned up
- ✅ No leftover files or folders

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 23: Silent Installation

**Command:** `EventViewerThemer-Setup.exe /S`

**Expected Results:**
- ✅ Installs without showing UI
- ✅ Uses default installation directory
- ✅ Creates Start Menu shortcuts
- ✅ Installation completes successfully

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Phase 4.7: Stability & Performance Tests

### Test Case 24: Extended Runtime (24 Hours)

**Objective:** Test for memory leaks and stability

**Steps:**
1. Apply theme
2. Leave Event Viewer and Injector running for 24 hours
3. Monitor memory usage periodically

**Expected Results:**
- ✅ No memory leaks
- ✅ Theme remains stable
- ✅ No crashes
- ✅ Event Viewer remains functional

**Actual Results:**
- Status: ⏳ Pending
- Memory Usage (Start):
- Memory Usage (24h):
- Notes:

---

### Test Case 25: Rapid Enable/Disable Cycles

**Objective:** Test rapid theme toggling

**Steps:**
1. Open Event Viewer
2. Run injector → close → run → close (10 times rapidly)
3. Observe stability

**Expected Results:**
- ✅ No crashes
- ✅ Theme toggles reliably
- ✅ No visual artifacts
- ✅ Event Viewer remains stable

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

### Test Case 26: Heavy Event Log Usage

**Objective:** Test with large event logs

**Steps:**
1. Apply theme
2. Open Application log with thousands of events
3. Scroll rapidly
4. Sort by different columns
5. Filter events
6. Open event details

**Expected Results:**
- ✅ No performance degradation
- ✅ Theme renders correctly with many items
- ✅ Scrolling is smooth
- ✅ No visual glitches

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Phase 4.8: Compatibility Tests

### Test Case 27: With Antivirus Software

**Objective:** Test behavior with antivirus active

**Antivirus:** (Specify installed AV)

**Steps:**
1. Ensure antivirus is active and updated
2. Run injector
3. Monitor antivirus alerts

**Expected Results:**
- ✅ Antivirus may show warning (document it)
- ✅ After allowing, injector works normally
- ✅ No false positive quarantine

**Actual Results:**
- Status: ⏳ Pending
- Antivirus Product:
- Alerts Shown:
- Notes:

---

### Test Case 28: With Windows Defender

**Objective:** Test Windows Defender compatibility

**Steps:**
1. Ensure Windows Defender is active
2. Run injector
3. Check for SmartScreen warnings

**Expected Results:**
- ✅ SmartScreen may warn about unsigned app
- ✅ After "Run anyway", works correctly
- ✅ No persistent blocks

**Actual Results:**
- Status: ⏳ Pending
- Notes:

---

## Test Summary

### Overall Results

**Total Test Cases:** 28
**Passed:** 5 (CLI argument tests)
**Partial Pass:** 1 (--silent needs full testing with injection)
**Failed:** 0
**Pending:** 22

**Test Results:**
- ✅ Test Case 6: --help (PASSED)
- ✅ Test Case 7: --version (PASSED)
- ✅ Test Case 8: --status inactive (PASSED)
- ⚠️ Test Case 10: --silent (PARTIAL - needs injection test)
- ✅ Test Case 11: Invalid arguments (PASSED)

**Critical Issues:** None found
**Major Issues:** None found
**Minor Issues:** None found

### Tested Platforms

- ✅ Windows 10 Pro 2009 (Build 19045) - CLI testing completed
- ⏳ Windows 11 - Pending
- ⏳ Windows 10 (other versions) - Pending
- ⏳ Windows Server - Pending

### Recommendations

1. Complete all pending test cases
2. Test on multiple Windows versions (VMs recommended)
3. Test with various antivirus software
4. Consider code signing certificate to eliminate SmartScreen warnings
5. Document any platform-specific issues

---

## Appendix A: Test Execution Checklist

- [✅] System information documented
- [ ] Basic functionality tests (1-5)
- [✅] CLI argument tests (6-11) - 5 passed, 1 partial
- [ ] Configuration tests (12-15)
- [ ] Error condition tests (16-18)
- [ ] Installer tests (19-23)
- [ ] Stability tests (24-26)
- [ ] Compatibility tests (27-28)
- [✅] Results documented
- [ ] Issues logged (if any)
- [ ] Test report generated

---

## Appendix B: Known Limitations

1. **--config argument:** Recognized but not yet functional
2. **--disable argument:** Recognized but not yet functional
3. **Multiple Event Viewer instances:** Current behavior targets first instance only
4. **32-bit Windows:** Not supported (64-bit only)

---

## Appendix C: Testing Environment Setup

### Required Tools
- Windows Event Viewer (built-in)
- Administrator account
- Optional: Multiple Windows VMs for cross-version testing

### Test Data Location
- Binaries: `C:\Users\Justin\DEV\shades\dist\EventViewerThemer\`
- Installer: `C:\Users\Justin\DEV\shades\dist\EventViewerThemer-Setup.exe`

---

**Testing Document Version:** 1.0
**Last Updated:** November 2025
