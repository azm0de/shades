# SHADES Test Module: Theme JSON Validation
# Validates all theme JSON files have correct structure and valid color values

Write-TestSection "Theme JSON Validation"

$distThemePath = Join-Path $script:DistDir "theme.json"
$requiredColorKeys = @("window_bg", "window_text", "highlight_bg", "highlight_text", "button_face", "button_text", "header_bg")
$hexPattern = '^#[0-9A-Fa-f]{6}$'

$presetThemes = @("Dracula", "Nord", "Monokai", "Solarized_Dark")

# Helper: validate a theme JSON file
function Test-ThemeFile($path, $name, $requireName) {
    if (-not (Test-Path $path)) {
        Skip "$name is valid JSON" "File not found: $path"
        Skip "$name has all required color keys" "File not found"
        Skip "$name has valid hex color values" "File not found"
        return
    }

    # Test: valid JSON
    $json = $null
    $isValid = $true
    try {
        $json = Get-Content $path -Raw | ConvertFrom-Json
    } catch {
        $isValid = $false
    }
    Assert-True $isValid "$name is valid JSON"

    if (-not $json) { return }

    # Test: has colors object
    $hasColors = $null -ne $json.colors
    Assert-True $hasColors "$name has 'colors' object"

    if (-not $hasColors) { return }

    # Test: has all 7 required color keys
    $missingKeys = @()
    foreach ($key in $requiredColorKeys) {
        $val = $json.colors.PSObject.Properties[$key]
        if (-not $val) {
            $missingKeys += $key
        }
    }
    if ($missingKeys.Count -eq 0) {
        Pass "$name has all 7 required color keys"
    } else {
        Fail "$name has all 7 required color keys" "Missing: $($missingKeys -join ', ')"
    }

    # Test: all color values match #RRGGBB
    $badColors = @()
    foreach ($prop in $json.colors.PSObject.Properties) {
        if ($prop.Value -notmatch $hexPattern) {
            $badColors += "$($prop.Name)=$($prop.Value)"
        }
    }
    if ($badColors.Count -eq 0) {
        Pass "$name has valid #RRGGBB color values"
    } else {
        Fail "$name has valid #RRGGBB color values" "Invalid: $($badColors -join ', ')"
    }

    # Test: has name field (for preset themes)
    if ($requireName) {
        $hasName = $null -ne $json.name -and $json.name -ne ""
        Assert-True $hasName "$name has 'name' metadata field"
    }
}

# --- Tests 1-4: dist/theme.json ---
Test-ThemeFile $distThemePath "dist/theme.json" $false

# --- Tests 5-16: Each preset theme ---
foreach ($theme in $presetThemes) {
    $themePath = Join-Path $script:ThemesDir "$theme.json"
    Test-ThemeFile $themePath "themes/$theme.json" $true
}

# --- Test 17: No duplicate theme names across presets ---
$names = @()
$hasDuplicates = $false
foreach ($theme in $presetThemes) {
    $themePath = Join-Path $script:ThemesDir "$theme.json"
    if (Test-Path $themePath) {
        try {
            $json = Get-Content $themePath -Raw | ConvertFrom-Json
            if ($json.name) {
                if ($names -contains $json.name) {
                    $hasDuplicates = $true
                }
                $names += $json.name
            }
        } catch {}
    }
}
Assert-False $hasDuplicates "No duplicate theme names across presets"

# --- Fixture edge-case tests ---
Write-TestSection "Theme JSON Edge Cases (Fixtures)"

$fixturesDir = Join-Path $PSScriptRoot "fixtures"

# Test: empty.json fails to parse
$emptyPath = Join-Path $fixturesDir "empty.json"
if (Test-Path $emptyPath) {
    $parseFailed = $false
    try {
        $content = Get-Content $emptyPath -Raw
        if ([string]::IsNullOrWhiteSpace($content)) {
            $parseFailed = $true
        } else {
            $null = $content | ConvertFrom-Json
        }
    } catch {
        $parseFailed = $true
    }
    Assert-True $parseFailed "Fixture: empty.json fails to parse as valid theme"
} else {
    Skip "Fixture: empty.json fails to parse" "Fixture file not found"
}

# Test: invalid_json.json fails to parse
$invalidPath = Join-Path $fixturesDir "invalid_json.json"
if (Test-Path $invalidPath) {
    $parseFailed = $false
    try {
        $null = Get-Content $invalidPath -Raw | ConvertFrom-Json
    } catch {
        $parseFailed = $true
    }
    Assert-True $parseFailed "Fixture: invalid_json.json fails to parse"
} else {
    Skip "Fixture: invalid_json.json fails to parse" "Fixture file not found"
}

# Test: missing_colors.json has no 'colors' key
$missingColorsPath = Join-Path $fixturesDir "missing_colors.json"
if (Test-Path $missingColorsPath) {
    try {
        $json = Get-Content $missingColorsPath -Raw | ConvertFrom-Json
        $hasColors = $null -ne $json.colors
        Assert-False $hasColors "Fixture: missing_colors.json lacks 'colors' key"
    } catch {
        Pass "Fixture: missing_colors.json lacks 'colors' key (parse error)"
    }
} else {
    Skip "Fixture: missing_colors.json lacks 'colors' key" "Fixture file not found"
}

# Test: bad_hex.json has invalid color values
$badHexPath = Join-Path $fixturesDir "bad_hex.json"
if (Test-Path $badHexPath) {
    try {
        $json = Get-Content $badHexPath -Raw | ConvertFrom-Json
        $hasBadColors = $false
        if ($json.colors) {
            foreach ($prop in $json.colors.PSObject.Properties) {
                if ($prop.Value -notmatch $hexPattern) {
                    $hasBadColors = $true
                    break
                }
            }
        }
        Assert-True $hasBadColors "Fixture: bad_hex.json contains invalid color values"
    } catch {
        Pass "Fixture: bad_hex.json contains invalid color values (parse error)"
    }
} else {
    Skip "Fixture: bad_hex.json contains invalid colors" "Fixture file not found"
}

# Test: partial_colors.json is missing required keys
$partialPath = Join-Path $fixturesDir "partial_colors.json"
if (Test-Path $partialPath) {
    try {
        $json = Get-Content $partialPath -Raw | ConvertFrom-Json
        $missingCount = 0
        if ($json.colors) {
            foreach ($key in $requiredColorKeys) {
                if (-not $json.colors.PSObject.Properties[$key]) {
                    $missingCount++
                }
            }
        }
        Assert-GreaterThan $missingCount 0 "Fixture: partial_colors.json is missing required color keys"
    } catch {
        Pass "Fixture: partial_colors.json is missing required color keys (parse error)"
    }
} else {
    Skip "Fixture: partial_colors.json missing keys" "Fixture file not found"
}
