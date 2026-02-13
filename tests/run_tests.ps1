# SHADES Test Suite Runner
# Run all test modules and report results

param(
    [string]$Filter = ""  # Optional: run only tests matching this name
)

$ErrorActionPreference = "Continue"

# Source helpers
. "$PSScriptRoot\test_helpers.ps1"

Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "  SHADES Test Suite" -ForegroundColor White
Write-Host "========================================" -ForegroundColor White

# Collect all test modules
$testModules = @(
    "test_build_verify",
    "test_manifest",
    "test_cli",
    "test_json_themes",
    "test_file_paths",
    "test_installer"
)

# Source and run each module
foreach ($module in $testModules) {
    if ($Filter -and $module -notlike "*$Filter*") {
        continue
    }

    $modulePath = Join-Path $PSScriptRoot "$module.ps1"
    if (Test-Path $modulePath) {
        try {
            . $modulePath
        } catch {
            Write-Host ""
            Write-Host "  [ERROR] Module $module threw exception: $_" -ForegroundColor Red
            $script:TestsFailed++
            $script:FailureDetails += "Module $module crashed: $_"
        }
    } else {
        Write-Host ""
        Write-Host "  [WARN] Module not found: $modulePath" -ForegroundColor Yellow
    }
}

# Summary
$total = $script:TestsPassed + $script:TestsFailed + $script:TestsSkipped
Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "  Results: $total total" -ForegroundColor White
Write-Host "    Passed:  $($script:TestsPassed)" -ForegroundColor Green
Write-Host "    Failed:  $($script:TestsFailed)" -ForegroundColor $(if ($script:TestsFailed -gt 0) { "Red" } else { "Green" })
Write-Host "    Skipped: $($script:TestsSkipped)" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor White

if ($script:FailureDetails.Count -gt 0) {
    Write-Host ""
    Write-Host "  Failure Details:" -ForegroundColor Red
    foreach ($detail in $script:FailureDetails) {
        Write-Host "    - $detail" -ForegroundColor DarkRed
    }
}

Write-Host ""

# Exit with appropriate code
if ($script:TestsFailed -gt 0) {
    exit 1
} else {
    exit 0
}
