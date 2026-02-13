# SHADES Test Helpers
# Shared assertion functions and counters for all test modules

$script:TestsPassed = 0
$script:TestsFailed = 0
$script:TestsSkipped = 0
$script:FailureDetails = @()

# Project root (parent of tests/)
$script:ProjectRoot = Split-Path -Parent $PSScriptRoot
$script:DistDir = Join-Path $script:ProjectRoot "dist\EventViewerThemer"
$script:ThemesDir = Join-Path $script:ProjectRoot "themes"
$script:InjectorDir = Join-Path $script:ProjectRoot "Injector"

function Pass($message) {
    $script:TestsPassed++
    Write-Host "  [PASS] $message" -ForegroundColor Green
}

function Fail($message, $detail) {
    $script:TestsFailed++
    Write-Host "  [FAIL] $message" -ForegroundColor Red
    if ($detail) {
        Write-Host "         $detail" -ForegroundColor DarkRed
        $script:FailureDetails += "$message : $detail"
    } else {
        $script:FailureDetails += $message
    }
}

function Skip($message, $reason) {
    $script:TestsSkipped++
    Write-Host "  [SKIP] $message" -ForegroundColor Yellow
    if ($reason) {
        Write-Host "         $reason" -ForegroundColor DarkYellow
    }
}

function Assert-True($condition, $message) {
    if ($condition) {
        Pass $message
    } else {
        Fail $message
    }
}

function Assert-False($condition, $message) {
    if (-not $condition) {
        Pass $message
    } else {
        Fail $message
    }
}

function Assert-Equal($expected, $actual, $message) {
    if ($expected -eq $actual) {
        Pass $message
    } else {
        Fail $message "Expected: '$expected', Got: '$actual'"
    }
}

function Assert-GreaterThan($value, $threshold, $message) {
    if ($value -gt $threshold) {
        Pass $message
    } else {
        Fail $message "Value $value is not greater than $threshold"
    }
}

function Assert-FileExists($path, $message) {
    if (-not $message) { $message = "File exists: $(Split-Path -Leaf $path)" }
    if (Test-Path $path) {
        Pass $message
    } else {
        Fail $message "File not found: $path"
    }
}

function Assert-FileNotExists($path, $message) {
    if (-not $message) { $message = "File does not exist: $(Split-Path -Leaf $path)" }
    if (-not (Test-Path $path)) {
        Pass $message
    } else {
        Fail $message "File unexpectedly found: $path"
    }
}

function Assert-DirectoryExists($path, $message) {
    if (-not $message) { $message = "Directory exists: $(Split-Path -Leaf $path)" }
    if (Test-Path $path -PathType Container) {
        Pass $message
    } else {
        Fail $message "Directory not found: $path"
    }
}

function Assert-StringContains($haystack, $needle, $message) {
    if ($haystack -and $haystack.Contains($needle)) {
        Pass $message
    } else {
        Fail $message "String does not contain '$needle'"
    }
}

function Assert-Match($string, $pattern, $message) {
    if ($string -match $pattern) {
        Pass $message
    } else {
        Fail $message "String '$string' does not match pattern '$pattern'"
    }
}

function Write-TestSection($name) {
    Write-Host ""
    Write-Host "--- $name ---" -ForegroundColor Cyan
}
