# SHADES - Testing Plan

**Version:** v1.2.0
**Platform:** Windows 10/11 (64-bit)

---

## Test Matrix

| OS | Status |
| --- | --- |
| Windows 10 Pro (Build 19045) | CLI tests passed |
| Windows 11 | Pending |
| Windows Server 2019/2022 | Pending |

---

## Core Functionality

| # | Test | Status |
| --- | --- | --- |
| 1 | Theme applies to Event Viewer on launch | Pending |
| 2 | Theme persists during normal usage (navigate logs, open details, filter) | Pending |
| 3 | Theme deactivates when SHADES exits (tray -> Exit) | Pending |
| 4 | Hot-reload: edit theme.json while running, colors update within 500ms | Pending |
| 5 | Multiple Event Viewer instances (first is themed) | Pending |
| 6 | Re-launching SHADES warns about duplicate instance | Pending |

## CLI Arguments

| # | Test | Status |
| --- | --- | --- |
| 7 | `--help` displays usage and exits | Passed |
| 8 | `--version` displays v1.2.0 and exits | Passed |
| 9 | `--status` reports INACTIVE when no instance running | Passed |
| 10 | `--status` reports ACTIVE when instance running | Pending |
| 11 | `--silent` suppresses console output | Partial |
| 12 | `--config <path>` loads custom theme file | Pending |
| 13 | `--disable` shuts down running instance | Pending |
| 14 | Invalid arguments show error + suggest --help | Passed |

## Configuration

| # | Test | Status |
| --- | --- | --- |
| 15 | Default theme.json loads correctly | Pending |
| 16 | Custom color scheme applies | Pending |
| 17 | Malformed theme.json shows error, no crash | Pending |
| 18 | Missing theme.json shows error, no crash | Pending |

## Error Handling

| # | Test | Status |
| --- | --- | --- |
| 19 | Run without admin: clear error message | Pending |
| 20 | Event Viewer not running: prompts to launch | Pending |
| 21 | Missing ThemeEngine.dll: error with expected path | Pending |

## Installer

| # | Test | Status |
| --- | --- | --- |
| 22 | Fresh install to Program Files | Pending |
| 23 | Start Menu shortcuts work | Pending |
| 24 | Uninstall removes all files and registry | Pending |
| 25 | Silent install (`/S` flag) | Pending |

## Stability

| # | Test | Status |
| --- | --- | --- |
| 26 | 24-hour runtime: no memory leaks or crashes | Pending |
| 27 | Rapid enable/disable 10x cycles | Pending |
| 28 | Large event log (10,000+ entries) scrolling | Pending |

---

## Summary

**Total:** 28 | **Passed:** 4 | **Partial:** 1 | **Pending:** 23

**Known Limitations:**
- 64-bit Windows only
- Targets first Event Viewer instance found
