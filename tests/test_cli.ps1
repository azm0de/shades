# SHADES Test Module: CLI Flag Tests
# Tests command-line argument handling by running SHADES.exe with various flags
# Note: SHADES.exe requires admin elevation (manifest). Tests skip if not elevated.

Write-TestSection "CLI Flag Tests"

$exePath = Join-Path $script:DistDir "SHADES.exe"

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

$cliTestNames = @(
    "--help exits with code 0",
    "--help output contains 'Usage:'",
    "--help output contains 'SHADES.exe'",
    "--version exits with code 0",
    "--version output contains '1.2.0'",
    "-h alias exits with code 0 (same as --help)",
    "-v alias exits with code 0 (same as --version)",
    "--status exits with code 0",
    "--status output contains 'ACTIVE' or 'INACTIVE'",
    "Unknown flag --bogus exits with code 0"
)

if (-not (Test-Path $exePath)) {
    foreach ($name in $cliTestNames) {
        Skip $name "SHADES.exe not found"
    }
    return
}

if (-not $isAdmin) {
    foreach ($name in $cliTestNames) {
        Skip $name "Requires admin elevation (SHADES.exe has requireAdministrator manifest)"
    }

    # Instead, validate CLI behavior through static analysis of source code
    Write-TestSection "CLI Static Analysis (no elevation needed)"

    $srcPath = Join-Path $script:InjectorDir "Injector.cpp"
    if (Test-Path $srcPath) {
        $src = Get-Content $srcPath -Raw

        # Verify --help flag is handled
        Assert-StringContains $src '"--help"' "Source handles --help flag"

        # Verify --version flag is handled
        Assert-StringContains $src '"--version"' "Source handles --version flag"

        # Verify --status flag is handled
        Assert-StringContains $src '"--status"' "Source handles --status flag"

        # Verify -h short alias
        Assert-StringContains $src '"-h"' "Source handles -h short alias"

        # Verify -v short alias
        Assert-StringContains $src '"-v"' "Source handles -v short alias"

        # Verify --silent flag is handled
        Assert-StringContains $src '"--silent"' "Source handles --silent flag"

        # Verify --config flag is handled
        Assert-StringContains $src '"--config"' "Source handles --config flag"

        # Verify --disable flag is handled
        Assert-StringContains $src '"--disable"' "Source handles --disable flag"

        # Verify unknown args are caught
        Assert-StringContains $src "Unknown argument" "Source catches unknown arguments"

        # Verify version string matches
        Assert-StringContains $src '"1.2.0"' "Source contains version 1.2.0"
    } else {
        Skip "CLI static analysis" "Injector.cpp not found"
    }

    return
}

# --- Running as admin: execute actual CLI tests ---

function Run-ShadesWithArgs($args_string) {
    $stdout = Join-Path $env:TEMP "shades_test_stdout.txt"
    $stderr = Join-Path $env:TEMP "shades_test_stderr.txt"

    # Clean up any previous output
    if (Test-Path $stdout) { Remove-Item $stdout -Force }
    if (Test-Path $stderr) { Remove-Item $stderr -Force }

    $proc = Start-Process -FilePath $exePath -ArgumentList $args_string `
        -RedirectStandardOutput $stdout -RedirectStandardError $stderr `
        -NoNewWindow -Wait -PassThru

    $output = ""
    $errorOutput = ""
    if (Test-Path $stdout) { $output = Get-Content $stdout -Raw -ErrorAction SilentlyContinue }
    if (Test-Path $stderr) { $errorOutput = Get-Content $stderr -Raw -ErrorAction SilentlyContinue }

    # Clean up temp files
    if (Test-Path $stdout) { Remove-Item $stdout -Force }
    if (Test-Path $stderr) { Remove-Item $stderr -Force }

    return @{
        ExitCode = $proc.ExitCode
        StdOut = if ($output) { $output } else { "" }
        StdErr = if ($errorOutput) { $errorOutput } else { "" }
        Combined = (if ($output) { $output } else { "" }) + (if ($errorOutput) { $errorOutput } else { "" })
    }
}

# --- Test 1: --help exits with code 0 ---
$result = Run-ShadesWithArgs "--help"
Assert-Equal 0 $result.ExitCode "--help exits with code 0"

# --- Test 2: --help output contains "Usage:" ---
Assert-StringContains $result.Combined "Usage:" "--help output contains 'Usage:'"

# --- Test 3: --help output contains "SHADES.exe" ---
Assert-StringContains $result.Combined "SHADES.exe" "--help output contains 'SHADES.exe'"

# --- Test 4: --version exits with code 0 ---
$result = Run-ShadesWithArgs "--version"
Assert-Equal 0 $result.ExitCode "--version exits with code 0"

# --- Test 5: --version output contains "1.2.0" ---
Assert-StringContains $result.Combined "1.2.0" "--version output contains '1.2.0'"

# --- Test 6: -h works same as --help ---
$result = Run-ShadesWithArgs "-h"
Assert-Equal 0 $result.ExitCode "-h alias exits with code 0 (same as --help)"

# --- Test 7: -v works same as --version ---
$result = Run-ShadesWithArgs "-v"
Assert-Equal 0 $result.ExitCode "-v alias exits with code 0 (same as --version)"

# --- Test 8: --status exits with code 0 ---
$result = Run-ShadesWithArgs "--status"
Assert-Equal 0 $result.ExitCode "--status exits with code 0"

# --- Test 9: --status output contains ACTIVE or INACTIVE ---
$hasStatus = $result.Combined.Contains("ACTIVE") -or $result.Combined.Contains("INACTIVE")
Assert-True $hasStatus "--status output contains 'ACTIVE' or 'INACTIVE'"

# --- Test 10: Unknown flag --bogus exits with code 0 and shows error ---
$result = Run-ShadesWithArgs "--bogus"
Assert-Equal 0 $result.ExitCode "Unknown flag --bogus exits with code 0"
