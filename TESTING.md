# Event Viewer Themer - Testing Plan & Results

## Testing Overview

**Version Under Test:** v1.2.0
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
| Windows 10 Pro 2009 | 19045 | âœ… Testing | Current system - CLI tests passed |
| Windows 11 23H2 | | â³ Pending | |
| Windows 11 22H2 | | â³ Pending | |
| Windows 10 22H2 | | â³ Pending | |
| Windows 10 21H2 | | â³ Pending | |
| Windows Server 2022 | | â³ Pending | |
| Windows Server 2019 | | â³ Pending | |

---

## Phase 4.2: Core Functionality Tests

### Test Case 1: Basic Theme Application

**Objective:** Verify theme applies correctly to Event Viewer

**Prerequisites:**
- Event Viewer is closed
- No previous instance of Injector running

**Steps:**
1. Open Event Viewer (Win+R â†’ eventvwr.msc)
2. Note original appearance (light theme)
3. Run Injector.exe as Administrator
4. Observe Event Viewer window

**Expected Results:**
- âœ… Theme applies immediately
- âœ… Background changes to dark (#1E1E1E)
- âœ… Text changes to light (#D4D4D4)
- âœ… List items are readable
- âœ… Selection highlighting works
- âœ… Column headers are themed

**Actual Results:**
- Status: â³ Pending
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
- âœ… Theme remains active throughout
- âœ… No flickering or reversion to light theme
- âœ… All UI elements remain themed

**Actual Results:**
- Status: â³ Pending
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
- âœ… Theme reverts to original appearance within a few seconds
- âœ… Event Viewer returns to system default theme
- âœ… No corruption or artifacts
- âœ… Event Viewer remains functional

**Actual Results:**
- Status: â³ Pending
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
- âœ… First instance is themed
- âœ… Second instance may or may not be themed (document behavior)
- âœ… No crashes or errors

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 5: Injector Re-launch

**Objective:** Test running injector multiple times

**Steps:**
1. Event Viewer already open and themed
2. Run Injector.exe again (second instance)
3. Observe behavior

**Expected Results:**
- âœ… Warning message about duplicate instance
- âœ… No crash
- âœ… Theme remains active
- âœ… Either allows or prevents second instance gracefully

**Actual Results:**
- Status: â³ Pending
- Notes:

---

## Phase 4.3: Command-Line Argument Tests

### Test Case 6: --help Argument

**Command:** `Injector.exe --help`

**Expected Results:**
- âœ… Displays comprehensive help message
- âœ… Shows all available options
- âœ… Includes usage examples
- âœ… Shows project URL
- âœ… Exits cleanly without injection

**Actual Results:**
- Status: âœ… PASSED
- Notes: Help displays correctly with all options, examples, and GitHub URL
- Output includes version 1.2.0, comprehensive usage instructions
- All sections present: Options, Examples, Notes, GitHub link

---

### Test Case 7: --version Argument

**Command:** `Injector.exe --version`

**Expected Results:**
- âœ… Shows "Event Viewer Themer version 1.2.0"
- âœ… Shows copyright information
- âœ… Shows MIT License
- âœ… Exits cleanly

**Actual Results:**
- Status: âœ… PASSED
- Notes: Displays "Event Viewer Themer version 1.2.0"
- Copyright shows "Copyright (c) 2025 azm0de"
- License shows "Licensed under the MIT License"
- Exits cleanly

---

### Test Case 8: --status Argument (Theme Inactive)

**Command:** `Injector.exe --status`

**Prerequisites:** No theme currently active

**Expected Results:**
- âœ… Shows "Theme Status: INACTIVE"
- âœ… Explains theme is not active
- âœ… Exits with appropriate exit code

**Actual Results:**
- Status: âœ… PASSED
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
- âœ… Shows "Theme Status: ACTIVE"
- âœ… Explains theme is enabled
- âœ… Exits with appropriate exit code

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 10: --silent Argument

**Command:** `Injector.exe --silent`

**Prerequisites:** Event Viewer is open

**Expected Results:**
- âœ… No console output except errors
- âœ… Theme still applies correctly
- âœ… No "Press any key to continue" prompt
- âœ… Runs in background

**Actual Results:**
- Status: âš ï¸ PARTIAL PASS
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
- âœ… Shows error message
- âœ… Suggests using --help
- âœ… Exits gracefully without crash

**Actual Results:**
- Status: âœ… PASSED
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
- âœ… Colors match theme.json exactly:
  - window_bg: #1E1E1E
  - window_text: #D4D4D4
  - highlight_bg: #2A2D2E
  - highlight_text: #FFFFFF
  - button_face: #333333
  - button_text: #D4D4D4
  - header_bg: #333333

**Actual Results:**
- Status: â³ Pending
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
- âœ… Custom colors are applied correctly
- âœ… All UI elements use new color scheme

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 14: Malformed theme.json

**Objective:** Test error handling with invalid JSON

**Steps:**
1. Create invalid theme.json (broken JSON syntax)
2. Run injector
3. Observe behavior

**Expected Results:**
- âœ… Displays error message about invalid JSON
- âœ… Falls back to defaults OR refuses to inject
- âœ… No crash

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 15: Missing theme.json

**Objective:** Test behavior when theme.json is missing

**Steps:**
1. Rename or delete theme.json
2. Run injector

**Expected Results:**
- âœ… Shows error about missing theme file
- âœ… Exits gracefully OR uses hardcoded defaults
- âœ… No crash

**Actual Results:**
- Status: â³ Pending
- Notes:

---

## Phase 4.5: Error Condition Tests

### Test Case 16: No Administrator Privileges

**Objective:** Test behavior without admin rights

**Steps:**
1. Run Injector.exe as normal user (not elevated)
2. Observe error message

**Expected Results:**
- âœ… Clear error message about requiring admin privileges
- âœ… Suggests running as administrator
- âœ… Exits gracefully

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 17: Event Viewer Not Running

**Objective:** Test behavior when target process doesn't exist

**Steps:**
1. Close all Event Viewer instances
2. Run Injector.exe

**Expected Results:**
- âœ… Error message: "No mmc.exe process found"
- âœ… Suggests opening Event Viewer first
- âœ… Exits gracefully

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 18: Missing ThemeEngine.dll

**Objective:** Test behavior when DLL is missing

**Steps:**
1. Rename or move ThemeEngine.dll
2. Run Injector.exe with Event Viewer open

**Expected Results:**
- âœ… Error message about missing DLL
- âœ… Shows expected DLL path
- âœ… Exits gracefully

**Actual Results:**
- Status: â³ Pending
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
- âœ… Installer opens without errors
- âœ… License agreement displays
- âœ… Directory selection works
- âœ… Files install to Program Files
- âœ… Start Menu shortcuts created
- âœ… Installation completes successfully

**Actual Results:**
- Status: â³ Pending
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
- âœ… Folder exists: Start Menu\Programs\Event Viewer Themer\
- âœ… "Event Viewer Themer" launches injector
- âœ… "Configure Theme" opens theme.json in notepad
- âœ… "README" opens README.txt
- âœ… "Uninstall" launches uninstaller

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 21: Desktop Shortcut (Optional)

**Objective:** Test optional desktop shortcut

**Steps:**
1. During installation, select "Desktop shortcut"
2. Complete installation
3. Check desktop

**Expected Results:**
- âœ… Shortcut appears on desktop
- âœ… Shortcut launches injector
- âœ… Icon displays correctly (if icon added)

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 22: Uninstallation

**Objective:** Verify complete removal

**Steps:**
1. After installation, run Uninstaller
2. Confirm uninstallation
3. Check all locations

**Expected Results:**
- âœ… Uninstaller confirms action
- âœ… All files removed from Program Files
- âœ… Start Menu folder removed
- âœ… Desktop shortcut removed (if created)
- âœ… Registry entries cleaned up
- âœ… No leftover files or folders

**Actual Results:**
- Status: â³ Pending
- Notes:

---

### Test Case 23: Silent Installation

**Command:** `EventViewerThemer-Setup.exe /S`

**Expected Results:**
- âœ… Installs without showing UI
- âœ… Uses default installation directory
- âœ… Creates Start Menu shortcuts
- âœ… Installation completes successfully

**Actual Results:**
- Status: â³ Pending
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
- âœ… No memory leaks
- âœ… Theme remains stable
- âœ… No crashes
- âœ… Event Viewer remains functional

**Actual Results:**
- Status: â³ Pending
- Memory Usage (Start):
- Memory Usage (24h):
- Notes:

---

### Test Case 25: Rapid Enable/Disable Cycles

**Objective:** Test rapid theme toggling

**Steps:**
1. Open Event Viewer
2. Run injector â†’ close â†’ run â†’ close (10 times rapidly)
3. Observe stability

**Expected Results:**
- âœ… No crashes
- âœ… Theme toggles reliably
- âœ… No visual artifacts
- âœ… Event Viewer remains stable

**Actual Results:**
- Status: â³ Pending
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
- âœ… No performance degradation
- âœ… Theme renders correctly with many items
- âœ… Scrolling is smooth
- âœ… No visual glitches

**Actual Results:**
- Status: â³ Pending
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
- âœ… Antivirus may show warning (document it)
- âœ… After allowing, injector works normally
- âœ… No false positive quarantine

**Actual Results:**
- Status: â³ Pending
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
- âœ… SmartScreen may warn about unsigned app
- âœ… After "Run anyway", works correctly
- âœ… No persistent blocks

**Actual Results:**
- Status: â³ Pending
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
- âœ… Test Case 6: --help (PASSED)
- âœ… Test Case 7: --version (PASSED)
- âœ… Test Case 8: --status inactive (PASSED)
- âš ï¸ Test Case 10: --silent (PARTIAL - needs injection test)
- âœ… Test Case 11: Invalid arguments (PASSED)

**Critical Issues:** None found
**Major Issues:** None found
**Minor Issues:** None found

### Tested Platforms

- âœ… Windows 10 Pro 2009 (Build 19045) - CLI testing completed
- â³ Windows 11 - Pending
- â³ Windows 10 (other versions) - Pending
- â³ Windows Server - Pending

### Recommendations

1. Complete all pending test cases
2. Test on multiple Windows versions (VMs recommended)
3. Test with various antivirus software
4. Consider code signing certificate to eliminate SmartScreen warnings
5. Document any platform-specific issues

---

## Appendix A: Test Execution Checklist

- [âœ…] System information documented
- [ ] Basic functionality tests (1-5)
- [âœ…] CLI argument tests (6-11) - 5 passed, 1 partial
- [ ] Configuration tests (12-15)
- [ ] Error condition tests (16-18)
- [ ] Installer tests (19-23)
- [ ] Stability tests (24-26)
- [ ] Compatibility tests (27-28)
- [âœ…] Results documented
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
