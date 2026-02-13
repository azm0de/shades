@echo off
REM Build ThemeConfig.exe (Configurator)

REM Find Visual Studio
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo ERROR: Visual Studio not found
    exit /b 1
)

cd /d "%~dp0Configurator"
cl.exe /O2 /EHsc /MD /I"..\libs\json" Configurator.cpp user32.lib gdi32.lib comdlg32.lib /Fe:ThemeConfig.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    copy ThemeConfig.exe ..\dist\EventViewerThemer\ThemeConfig.exe /Y
    echo ThemeConfig.exe copied to dist folder
) else (
    echo Build failed with error code %ERRORLEVEL%
)
