# SHADES Test Module: Installer Script Validation
# Validates the NSIS installer script references correct files and settings

Write-TestSection "Installer Script Validation"

$nsiPath = Join-Path $script:ProjectRoot "installer.nsi"

if (-not (Test-Path $nsiPath)) {
    Skip "installer.nsi exists" "File not found"
    Skip "Product name is SHADES" "installer.nsi not found"
    Skip "Version is 1.2.0" "installer.nsi not found"
    Skip "References all dist files" "installer.nsi not found"
    Skip "References all preset themes" "installer.nsi not found"
    Skip "Has MUI_FINISHPAGE_RUN" "installer.nsi not found"
    Skip "Uninstaller removes scheduled task" "installer.nsi not found"
    Skip ".onInit checks for running SHADES" "installer.nsi not found"
    return
}

$nsiContent = Get-Content $nsiPath -Raw

# --- Test 1: installer.nsi exists ---
Assert-FileExists $nsiPath "installer.nsi exists"

# --- Test 2: Product name is "SHADES" ---
Assert-StringContains $nsiContent 'PRODUCT_NAME "SHADES"' "installer.nsi product name is 'SHADES'"

# --- Test 3: Version is "1.2.0" ---
Assert-StringContains $nsiContent 'PRODUCT_VERSION "1.2.0"' "installer.nsi version is '1.2.0'"

# --- Test 4: References all dist files ---
$distFiles = @("SHADES.exe", "ThemeEngine.dll", "ThemeConfig.exe", "theme.json", "README.txt", "LICENSE.txt")
$allRefd = $true
$missingRefs = @()
foreach ($file in $distFiles) {
    if (-not $nsiContent.Contains($file)) {
        $allRefd = $false
        $missingRefs += $file
    }
}
if ($allRefd) {
    Pass "installer.nsi references all required dist files"
} else {
    Fail "installer.nsi references all required dist files" "Missing: $($missingRefs -join ', ')"
}

# --- Test 5: References all 4 preset themes ---
$themes = @("Dracula.json", "Nord.json", "Monokai.json", "Solarized_Dark.json")
$allThemes = $true
$missingThemes = @()
foreach ($theme in $themes) {
    if (-not $nsiContent.Contains($theme)) {
        $allThemes = $false
        $missingThemes += $theme
    }
}
if ($allThemes) {
    Pass "installer.nsi references all 4 preset themes"
} else {
    Fail "installer.nsi references all 4 preset themes" "Missing: $($missingThemes -join ', ')"
}

# --- Test 6: Has MUI_FINISHPAGE_RUN for auto-launch ---
Assert-StringContains $nsiContent "MUI_FINISHPAGE_RUN" "installer.nsi has MUI_FINISHPAGE_RUN for auto-launch"

# --- Test 7: Uninstaller removes scheduled task ---
Assert-StringContains $nsiContent 'schtasks /delete /tn "SHADES"' "installer.nsi uninstaller removes scheduled task"

# --- Test 8: .onInit checks for running SHADES ---
$hasOnInit = $nsiContent.Contains("Function .onInit") -or $nsiContent.Contains("function .onInit")
$checksTrayWnd = $nsiContent.Contains("ShadesTrayWnd")
Assert-True ($hasOnInit -and $checksTrayWnd) "installer.nsi .onInit checks for running SHADES instance"
