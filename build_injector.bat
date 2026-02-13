@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd /d "C:\Users\Justin\DEV\shades\Injector"
cl.exe /O2 /EHsc /MD /I"..\libs\json" Injector.cpp user32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /Fe:SHADES.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    copy SHADES.exe ..\dist\EventViewerThemer\SHADES.exe /Y
    echo SHADES.exe copied to dist folder
) else (
    echo Build failed with error code %ERRORLEVEL%
)
