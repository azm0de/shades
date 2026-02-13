# SHADES Test Module: Build Artifact Verification
# Validates SHADES.exe and ThemeEngine.dll are correctly built

Write-TestSection "Build Artifact Verification"

$exePath = Join-Path $script:DistDir "SHADES.exe"
$dllPath = Join-Path $script:DistDir "ThemeEngine.dll"

# --- Test 1: SHADES.exe exists ---
Assert-FileExists $exePath "SHADES.exe exists in dist/"

# --- Test 2: ThemeEngine.dll exists ---
Assert-FileExists $dllPath "ThemeEngine.dll exists in dist/"

# --- Test 3: SHADES.exe file size > 30KB ---
if (Test-Path $exePath) {
    $exeSize = (Get-Item $exePath).Length
    Assert-GreaterThan $exeSize 30720 "SHADES.exe file size > 30KB (actual: $([math]::Round($exeSize/1024))KB)"
} else {
    Skip "SHADES.exe file size > 30KB" "File not found"
}

# --- Test 4: ThemeEngine.dll file size > 30KB ---
if (Test-Path $dllPath) {
    $dllSize = (Get-Item $dllPath).Length
    Assert-GreaterThan $dllSize 30720 "ThemeEngine.dll file size > 30KB (actual: $([math]::Round($dllSize/1024))KB)"
} else {
    Skip "ThemeEngine.dll file size > 30KB" "File not found"
}

# --- Test 5: SHADES.exe is PE format (MZ header) ---
if (Test-Path $exePath) {
    $bytes = [System.IO.File]::ReadAllBytes($exePath)
    $mzHeader = [char]$bytes[0] + [char]$bytes[1]
    Assert-Equal "MZ" $mzHeader "SHADES.exe has MZ (PE) header"
} else {
    Skip "SHADES.exe has MZ header" "File not found"
}

# --- Test 6: ThemeEngine.dll is PE format (MZ header) ---
if (Test-Path $dllPath) {
    $bytes = [System.IO.File]::ReadAllBytes($dllPath)
    $mzHeader = [char]$bytes[0] + [char]$bytes[1]
    Assert-Equal "MZ" $mzHeader "ThemeEngine.dll has MZ (PE) header"
} else {
    Skip "ThemeEngine.dll has MZ header" "File not found"
}

# --- Test 7: SHADES.exe PE subsystem = WINDOWS (2) ---
if (Test-Path $exePath) {
    $bytes = [System.IO.File]::ReadAllBytes($exePath)
    # PE header offset is at 0x3C (4 bytes, little-endian)
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    # PE signature should be "PE\0\0"
    $peSig = [char]$bytes[$peOffset] + [char]$bytes[$peOffset+1]
    if ($peSig -eq "PE") {
        # Subsystem is at PE offset + 0x5C (for PE32+, it's the same relative position in Optional Header)
        # COFF header is 20 bytes after PE sig (4 bytes), Optional Header starts at PE+24
        # Subsystem field offset in Optional Header: 68 for PE32, 68 for PE32+
        $optionalHeaderOffset = $peOffset + 24
        $magic = [BitConverter]::ToUInt16($bytes, $optionalHeaderOffset)
        if ($magic -eq 0x20b) {
            # PE32+ (64-bit)
            $subsystem = [BitConverter]::ToUInt16($bytes, $optionalHeaderOffset + 68)
        } else {
            # PE32 (32-bit)
            $subsystem = [BitConverter]::ToUInt16($bytes, $optionalHeaderOffset + 68)
        }
        Assert-Equal 2 $subsystem "SHADES.exe PE subsystem = WINDOWS (2), not CONSOLE"
    } else {
        Fail "SHADES.exe PE subsystem check" "Invalid PE signature at offset $peOffset"
    }
} else {
    Skip "SHADES.exe PE subsystem = WINDOWS" "File not found"
}

# --- Test 8: ThemeEngine.dll has DLL flag in PE characteristics ---
if (Test-Path $dllPath) {
    $bytes = [System.IO.File]::ReadAllBytes($dllPath)
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    $peSig = [char]$bytes[$peOffset] + [char]$bytes[$peOffset+1]
    if ($peSig -eq "PE") {
        # Characteristics is at PE offset + 4 (COFF header) + 18
        $characteristics = [BitConverter]::ToUInt16($bytes, $peOffset + 4 + 18)
        $isDll = ($characteristics -band 0x2000) -ne 0  # IMAGE_FILE_DLL = 0x2000
        Assert-True $isDll "ThemeEngine.dll has DLL flag in PE characteristics"
    } else {
        Fail "ThemeEngine.dll DLL flag check" "Invalid PE signature"
    }
} else {
    Skip "ThemeEngine.dll has DLL flag" "File not found"
}

# --- Test 9: Version string "1.2.0" present in SHADES.exe ---
if (Test-Path $exePath) {
    $content = [System.IO.File]::ReadAllText($exePath, [System.Text.Encoding]::ASCII)
    $hasVersion = $content.Contains("1.2.0")
    Assert-True $hasVersion "SHADES.exe contains version string '1.2.0'"
} else {
    Skip "SHADES.exe contains version string" "File not found"
}
