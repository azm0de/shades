# PowerShell build script for SHADES.exe
Write-Host "Building SHADES.exe with auto-launch and error dialogs..." -ForegroundColor Cyan
Write-Host ""

# Change to Injector directory
Set-Location "C:\Users\Justin\DEV\shades\Injector"

# Import Visual Studio environment
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cmd /c "`"$vsPath`" && cl.exe /O2 /EHsc /MD /I`"..\libs\json`" Injector.cpp user32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /Fe:SHADES.exe 2>&1"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Copying to dist folder..." -ForegroundColor Yellow
    Copy-Item "SHADES.exe" "..\dist\EventViewerThemer\SHADES.exe" -Force
    Write-Host "Done! SHADES.exe is ready to test." -ForegroundColor Green
    Write-Host ""
    Write-Host "File location: dist\EventViewerThemer\SHADES.exe" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "Build failed with error code $LASTEXITCODE" -ForegroundColor Red
}

Write-Host ""
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
