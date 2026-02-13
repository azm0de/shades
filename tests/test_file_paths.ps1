# SHADES Test Module: File & Path Integrity
# Validates dist folder structure, required files, and build script correctness

Write-TestSection "File & Path Integrity"

# --- Test 1: dist/EventViewerThemer/ directory exists ---
Assert-DirectoryExists $script:DistDir "dist/EventViewerThemer/ directory exists"

# --- Test 2: All 6 required dist files present ---
$requiredDistFiles = @("SHADES.exe", "ThemeEngine.dll", "ThemeConfig.exe", "theme.json", "README.txt", "LICENSE.txt")
foreach ($file in $requiredDistFiles) {
    $filePath = Join-Path $script:DistDir $file
    Assert-FileExists $filePath "dist/ contains $file"
}

# --- Test 3: themes/ directory exists ---
Assert-DirectoryExists $script:ThemesDir "themes/ directory exists"

# --- Test 4: All 4 preset theme files exist ---
$presetThemes = @("Dracula.json", "Nord.json", "Monokai.json", "Solarized_Dark.json")
foreach ($theme in $presetThemes) {
    $themePath = Join-Path $script:ThemesDir $theme
    Assert-FileExists $themePath "themes/ contains $theme"
}

# --- Test 5: Injector/SHADES.manifest exists ---
Assert-FileExists (Join-Path $script:InjectorDir "SHADES.manifest") "Injector/SHADES.manifest exists"

# --- Test 6: Injector/SHADES.rc exists ---
Assert-FileExists (Join-Path $script:InjectorDir "SHADES.rc") "Injector/SHADES.rc exists"

# --- Test 7: build_injector.bat contains rc.exe ---
$buildBatPath = Join-Path $script:ProjectRoot "build_injector.bat"
if (Test-Path $buildBatPath) {
    $content = Get-Content $buildBatPath -Raw
    Assert-StringContains $content "rc.exe" "build_injector.bat contains 'rc.exe' resource compiler"
} else {
    Skip "build_injector.bat contains rc.exe" "File not found"
}

# --- Test 8: build_injector.bat contains /OUT:SHADES.exe ---
if (Test-Path $buildBatPath) {
    $content = Get-Content $buildBatPath -Raw
    Assert-StringContains $content "/OUT:SHADES.exe" "build_injector.bat contains '/OUT:SHADES.exe'"
} else {
    Skip "build_injector.bat contains /OUT:SHADES.exe" "File not found"
}

# --- Test 9: Injector/build.ps1 contains rc.exe ---
$buildPs1Path = Join-Path $script:InjectorDir "build.ps1"
if (Test-Path $buildPs1Path) {
    $content = Get-Content $buildPs1Path -Raw
    Assert-StringContains $content "rc.exe" "Injector/build.ps1 contains 'rc.exe' resource compiler"
} else {
    Skip "Injector/build.ps1 contains rc.exe" "File not found"
}

# --- Test 10: No Injector.exe in dist/ (rebrand complete) ---
$oldExe = Join-Path $script:DistDir "Injector.exe"
Assert-FileNotExists $oldExe "No legacy 'Injector.exe' in dist/ (rebrand to SHADES.exe complete)"
