@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd /d "C:\Users\Justin\DEV\shades\ThemeEngine"
cl.exe /LD /O2 /EHsc /MT /I"..\libs\nlohmann" /I"..\libs\detours\include" ThemeEngine.cpp ..\libs\detours\lib.x64\detours.lib user32.lib gdi32.lib uxtheme.lib comctl32.lib shell32.lib /link /OUT:ThemeEngine.dll
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    copy ThemeEngine.dll ..\dist\EventViewerThemer\ThemeEngine.dll /Y
    echo DLL copied to dist folder
) else (
    echo Build failed with error code %ERRORLEVEL%
)
