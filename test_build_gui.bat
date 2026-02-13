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

REM Compile ColorUtils
echo.
echo Compiling ColorUtils...
cl.exe /c /Fo:build_test\ColorUtils.obj ^
    /I"src\gui" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\ColorUtils.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ColorUtils compilation failed!
    exit /b 1
)

REM Compile ThemeManager
echo.
echo Compiling ThemeManager...
cl.exe /c /Fo:build_test\ThemeManager.obj ^
    /I"src\gui" ^
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
    /I"src\gui" ^
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
    /I"src\gui" ^
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
    /I"src\gui" ^
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
    /I"src\gui" ^
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
    /I"src\gui" ^
    /I"src\common" ^
    /I"src\theme_manager" ^
    /I"src\gui" ^
    /I"libs\nlohmann" ^
    /DUNICODE /D_UNICODE /EHsc /std:c++17 ^
    src\gui\main.cpp build_test\ThemeManager.obj build_test\ColorUtils.obj build_test\ColorEditor.obj build_test\ColorPicker.obj build_test\PreviewPanel.obj build_test\ButtonBar.obj ^
    user32.lib gdi32.lib gdiplus.lib comctl32.lib shell32.lib shlwapi.lib advapi32.lib comdlg32.lib ^
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
