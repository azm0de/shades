# SHADES Test Module: Admin Manifest Verification
# Validates the admin manifest is correctly configured and embedded

Write-TestSection "Admin Manifest Verification"

$manifestPath = Join-Path $script:InjectorDir "SHADES.manifest"
$rcPath = Join-Path $script:InjectorDir "SHADES.rc"
$exePath = Join-Path $script:DistDir "SHADES.exe"

# --- Test 1: SHADES.manifest file exists ---
Assert-FileExists $manifestPath "SHADES.manifest source file exists"

# --- Test 2: SHADES.manifest contains requireAdministrator ---
if (Test-Path $manifestPath) {
    $manifestContent = Get-Content $manifestPath -Raw
    Assert-StringContains $manifestContent "requireAdministrator" "SHADES.manifest contains 'requireAdministrator'"
} else {
    Skip "SHADES.manifest contains requireAdministrator" "File not found"
}

# --- Test 3: SHADES.manifest is valid XML ---
if (Test-Path $manifestPath) {
    $isValidXml = $true
    try {
        [xml]$xml = Get-Content $manifestPath -Raw
    } catch {
        $isValidXml = $false
    }
    Assert-True $isValidXml "SHADES.manifest is valid XML"
} else {
    Skip "SHADES.manifest is valid XML" "File not found"
}

# --- Test 4: SHADES.exe binary contains requireAdministrator (manifest embedded) ---
if (Test-Path $exePath) {
    $bytes = [System.IO.File]::ReadAllBytes($exePath)
    $content = [System.Text.Encoding]::UTF8.GetString($bytes)
    $hasManifest = $content.Contains("requireAdministrator")
    Assert-True $hasManifest "SHADES.exe binary contains embedded 'requireAdministrator' manifest"
} else {
    Skip "SHADES.exe has embedded manifest" "File not found"
}

# --- Test 5: SHADES.rc references manifest with resource type 24 ---
if (Test-Path $rcPath) {
    $rcContent = Get-Content $rcPath -Raw
    $hasType24 = $rcContent -match '1\s+24\s+"SHADES\.manifest"'
    Assert-True $hasType24 "SHADES.rc references SHADES.manifest with resource type 24"
} else {
    Skip "SHADES.rc references manifest" "File not found"
}
