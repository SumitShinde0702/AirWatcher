param(
    [ValidateSet("app", "tests", "all")]
    [string]$Target = "app"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    Write-Error "g++ not found in PATH. Install MinGW-w64/MSYS2 and add g++ to PATH, then rerun build.ps1."
}

$commonSources = @(
    "src/cli/CliApp.cpp",
    "src/repository/CsvRepository.cpp",
    "src/service/AqiService.cpp",
    "src/service/AirAnalysisService.cpp",
    "src/service/AuthService.cpp",
    "src/utils/CsvUtils.cpp",
    "src/utils/Geo.cpp",
    "src/utils/Time.cpp"
)

function Build-App {
    g++ -std=c++17 "src/main.cpp" $commonSources -Isrc -o "airwatcher.exe"
}

function Build-Tests {
    g++ -std=c++17 "tests/test_main.cpp" $commonSources -Isrc -o "airwatcher_tests.exe"
}

switch ($Target) {
    "app" { Build-App }
    "tests" { Build-Tests }
    "all" {
        Build-App
        Build-Tests
    }
}

Write-Host "Build complete for target: $Target"
