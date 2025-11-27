@echo off
REM Test build script for GUI (uses cl.exe directly)
REM This is for testing only - production builds use CMake

echo.
echo ====================================
echo SHADES v2.0 - Test GUI Build
echo ====================================
echo.

REM Find Visual Studio
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo ERROR: Visual Studio not found
    exit /b 1
)

REM Create build directory for test
if not exist build_test mkdir build_test
if not exist build_test\include mkdir build_test\include

REM Generate version.h from template
echo Generating version.h...
(
    echo #ifndef SHADES_VERSION_H
    echo #define SHADES_VERSION_H
    echo #define SHADES_VERSION_MAJOR 2
    echo #define SHADES_VERSION_MINOR 0
    echo #define SHADES_VERSION_PATCH 0
    echo #define SHADES_VERSION_STRING "2.0.0-alpha"
    echo #define SHADES_VERSION_FULL "SHADES v2.0.0-alpha"
    echo #define SHADES_COPYRIGHT "Copyright (c) 2024-2026 azm0de"
    echo #define SHADES_DESCRIPTION "Windows Event Viewer Themer with GUI"
    echo #define SHADES_COMPANY "azm0de"
    echo #define SHADES_PRODUCT_NAME "SHADES"
    echo #define SHADES_BUILD_DATE __DATE__
    echo #define SHADES_BUILD_TIME __TIME__
    echo #define SHADES_VERSION_NUMBER 20000
    echo #define SHADES_THEME_FORMAT_VERSION "1.0"
    echo #define SHADES_API_VERSION 0x00020000
    echo #endif
) > build_test\include\version.h

REM Compile ThemeManager
echo.
echo Compiling ThemeManager...
cl.exe /c /Fo:build_test\ThemeManager.obj ^
    /I"build_test\include" ^
    /I"libs\nlohmann" ^
    /I"src\theme_manager" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\theme_manager\ThemeManager.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ThemeManager compilation failed!
    exit /b 1
)

REM Compile ColorEditor
echo.
echo Compiling ColorEditor...
cl.exe /c /Fo:build_test\ColorEditor.obj ^
    /I"build_test\include" ^
    /I"libs\nlohmann" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\ColorEditor.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ColorEditor compilation failed!
    exit /b 1
)

REM Compile ColorPicker
echo.
echo Compiling ColorPicker...
cl.exe /c /Fo:build_test\ColorPicker.obj ^
    /I"build_test\include" ^
    /I"libs\nlohmann" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\ColorPicker.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ColorPicker compilation failed!
    exit /b 1
)

REM Compile PreviewPanel
echo.
echo Compiling PreviewPanel...
cl.exe /c /Fo:build_test\PreviewPanel.obj ^
    /I"build_test\include" ^
    /I"libs\nlohmann" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\PreviewPanel.cpp

if %ERRORLEVEL% NEQ 0 (
    echo PreviewPanel compilation failed!
    exit /b 1
)

REM Compile ButtonBar
echo.
echo Compiling ButtonBar...
cl.exe /c /Fo:build_test\ButtonBar.obj ^
    /I"build_test\include" ^
    /I"libs\nlohmann" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\ButtonBar.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ButtonBar compilation failed!
    exit /b 1
)

REM Compile GUI
echo.
echo Compiling SHADES GUI...
cl.exe /Fe:build_test\SHADES.exe ^
    /I"build_test\include" ^
    /I"src\common" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /I"libs\nlohmann" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\main.cpp build_test\ThemeManager.obj build_test\ColorEditor.obj build_test\ColorPicker.obj build_test\PreviewPanel.obj build_test\ButtonBar.obj ^
    user32.lib gdi32.lib gdiplus.lib comctl32.lib shell32.lib shlwapi.lib ^
    /link /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Build succeeded!
    echo ====================================
    echo.
    echo Output: build_test\SHADES.exe
    echo.
    echo You can run it with: build_test\SHADES.exe
) else (
    echo.
    echo Build failed with error code %ERRORLEVEL%
    exit /b 1
)
