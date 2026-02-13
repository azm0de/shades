@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist "C:\Users\Justin\DEV\shades\Configurator" mkdir "C:\Users\Justin\DEV\shades\Configurator"
cd /d "C:\Users\Justin\DEV\shades\Configurator"
cl.exe /O2 /EHsc /MD /I"..\libs\json" Configurator.cpp user32.lib gdi32.lib comdlg32.lib /Fe:ThemeConfig.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    copy ThemeConfig.exe ..\dist\EventViewerThemer\ThemeConfig.exe /Y
    echo ThemeConfig.exe copied to dist folder
) else (
    echo Build failed with error code %ERRORLEVEL%
)
