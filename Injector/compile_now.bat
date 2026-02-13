@echo off
echo Compiling SHADES.exe with auto-launch and error dialogs...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cl.exe /O2 /EHsc /MD /I"..\libs\json" Injector.cpp user32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /Fe:SHADES.exe
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Copying to dist folder...
    copy SHADES.exe ..\dist\EventViewerThemer\SHADES.exe /Y
    echo.
    echo Done! SHADES.exe is ready to test.
) else (
    echo.
    echo Build failed with error code %ERRORLEVEL%
)
pause
