; Event Viewer Themer - NSIS Installer Script
; Version 1.2.0
; Created for professional distribution

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "LogicLib.nsh"

;--------------------------------
; General Configuration

; Application information
!define PRODUCT_NAME "Event Viewer Themer"
\!define PRODUCT_VERSION "1.2.0"
!define PRODUCT_PUBLISHER "azm0de"
!define PRODUCT_WEB_SITE "https://github.com/azm0de/shades"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Installer name and output file
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "EventViewerThemer-Setup.exe"

; Default installation directory
InstallDir "$PROGRAMFILES64\EventViewerThemer"

; Get installation folder from registry if available (for upgrades)
InstallDirRegKey HKLM "Software\EventViewerThemer" "InstallPath"

; Request application privileges for Windows Vista and higher
RequestExecutionLevel admin

; Compression
SetCompressor /SOLID lzma

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

;--------------------------------
; Version Information

VIProductVersion "1.2.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "MIT License - Copyright 2025"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Setup"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"

;--------------------------------
; Pages

; Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "dist\EventViewerThemer\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES

; Finish page configuration
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View README"
!define MUI_FINISHPAGE_LINK "Visit project on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Installer Sections

Section "Core Files (Required)" SecCore
  SectionIn RO  ; Read-only (always installed)

  SetOutPath "$INSTDIR"

  ; Install main application files
  File "dist\EventViewerThemer\SHADES.exe"
  File "dist\EventViewerThemer\ThemeEngine.dll"
  File "dist\EventViewerThemer\ThemeConfig.exe"
  File "dist\EventViewerThemer\theme.json"
  File "dist\EventViewerThemer\README.txt"
  File "dist\EventViewerThemer\LICENSE.txt"

  ; Store installation folder
  WriteRegStr HKLM "Software\EventViewerThemer" "InstallPath" $INSTDIR

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Write uninstaller registry keys
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\SHADES.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1

  ; Estimate installation size (in KB) - approximately 2MB
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" 2048

SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
  ; Create Start Menu folder
  CreateDirectory "$SMPROGRAMS\Event Viewer Themer"

  ; Create shortcuts (Note: User will need to right-click and "Run as administrator")
  CreateShortcut "$SMPROGRAMS\Event Viewer Themer\Event Viewer Themer.lnk" "$INSTDIR\SHADES.exe" "" "$INSTDIR\SHADES.exe" 0
  CreateShortcut "$SMPROGRAMS\Event Viewer Themer\Theme Configurator.lnk" "$INSTDIR\ThemeConfig.exe"
  CreateShortcut "$SMPROGRAMS\Event Viewer Themer\README.lnk" "$INSTDIR\README.txt"
  CreateShortcut "$SMPROGRAMS\Event Viewer Themer\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Desktop Shortcut" SecDesktop
  ; Create desktop shortcut
  CreateShortcut "$DESKTOP\Event Viewer Themer.lnk" "$INSTDIR\SHADES.exe" "" "$INSTDIR\SHADES.exe" 0
SectionEnd

;--------------------------------
; Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core application files (required)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Add shortcuts to Start Menu for easy access"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Add a shortcut to your Desktop"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Remove Start Menu shortcuts
  Delete "$SMPROGRAMS\Event Viewer Themer\Event Viewer Themer.lnk"
  Delete "$SMPROGRAMS\Event Viewer Themer\Theme Configurator.lnk"
  Delete "$SMPROGRAMS\Event Viewer Themer\README.lnk"
  Delete "$SMPROGRAMS\Event Viewer Themer\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Event Viewer Themer"

  ; Remove Desktop shortcut
  Delete "$DESKTOP\Event Viewer Themer.lnk"

  ; Remove application files
  Delete "$INSTDIR\SHADES.exe"
  Delete "$INSTDIR\ThemeEngine.dll"
  Delete "$INSTDIR\ThemeConfig.exe"
  Delete "$INSTDIR\theme.json"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove installation directory
  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "Software\EventViewerThemer"

SectionEnd

;--------------------------------
; Installer Functions

Function .onInit
  ; Display welcome note
  ; Installer will proceed without additional checks
FunctionEnd

Function un.onInit
  ; Confirm uninstallation
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove ${PRODUCT_NAME} and all of its components?" IDYES +2
  Abort
FunctionEnd
