@echo off
echo Compiling SHADES.exe with auto-launch and error dialogs...

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

rc.exe /fo SHADES.res SHADES.rc
cl.exe /O2 /EHsc /MD /I"..\libs\json" Injector.cpp SHADES.res user32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /OUT:SHADES.exe
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
