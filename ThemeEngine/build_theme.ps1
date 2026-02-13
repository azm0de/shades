# PowerShell build script for ThemeEngine.dll
Write-Host "Building ThemeEngine.dll with ListView background fix..." -ForegroundColor Cyan
Write-Host ""

# Change to ThemeEngine directory
Set-Location "C:\Users\Justin\DEV\shades\ThemeEngine"

# Import Visual Studio environment
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cmd /c "`"$vsPath`" && cl.exe /LD /O2 /EHsc /MD /I`"..\libs\nlohmann`" /I`"..\libs\detours\include`" ThemeEngine.cpp user32.lib gdi32.lib shell32.lib comctl32.lib ..\libs\detours\lib.X64\detours.lib /link /DEF:ThemeEngine.def /Fe:ThemeEngine.dll 2>&1"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Copying to dist folder..." -ForegroundColor Yellow
    Copy-Item "ThemeEngine.dll" "..\dist\EventViewerThemer\ThemeEngine.dll" -Force
    Write-Host "Done! ThemeEngine.dll is ready to test." -ForegroundColor Green
    Write-Host ""
    Write-Host "File location: dist\EventViewerThemer\ThemeEngine.dll" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "Build failed with error code $LASTEXITCODE" -ForegroundColor Red
}

Write-Host ""
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
