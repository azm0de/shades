# PowerShell build script for ThemeEngine.dll
Write-Host "Building ThemeEngine.dll..." -ForegroundColor Cyan
Write-Host ""

# Change to ThemeEngine directory
Set-Location "$PSScriptRoot"

# Find Visual Studio
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

$vsPath = $null
foreach ($p in $vsPaths) {
    if (Test-Path $p) {
        $vsPath = $p
        break
    }
}

if (-not $vsPath) {
    Write-Host "ERROR: Visual Studio not found" -ForegroundColor Red
    exit 1
}

cmd /c "`"$vsPath`" && cl.exe /LD /O2 /EHsc /MT /I`"..\libs\nlohmann`" /I`"..\libs\detours\include`" ThemeEngine.cpp ..\libs\detours\lib.X64\detours.lib user32.lib gdi32.lib uxtheme.lib comctl32.lib shell32.lib /link /DEF:ThemeEngine.def /Fe:ThemeEngine.dll 2>&1"

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
