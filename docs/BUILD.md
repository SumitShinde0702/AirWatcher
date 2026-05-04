# Build and Run Guide

## Requirements
- `g++` compiler in PATH (MinGW-w64/MSYS2)
- PowerShell (Windows)

Quick check:
```powershell
g++ --version
```

## One-command build
```powershell
./build.ps1
```

## Run CLI
```powershell
./airwatcher.exe
```

Default login after first run when no accounts file exists:
- Username: `admin`
- Password: `admin`

## Run tests
```powershell
./build.ps1 -Target tests
./airwatcher_tests.exe
```

## Manual compile (without script)
```powershell
g++ -std=c++17 src/main.cpp src/cli/CliApp.cpp src/repository/CsvRepository.cpp src/service/AqiService.cpp src/service/AirAnalysisService.cpp src/service/AuthService.cpp src/utils/CsvUtils.cpp src/utils/Geo.cpp src/utils/Time.cpp -Isrc -o airwatcher.exe
g++ -std=c++17 tests/test_main.cpp src/cli/CliApp.cpp src/repository/CsvRepository.cpp src/service/AqiService.cpp src/service/AirAnalysisService.cpp src/service/AuthService.cpp src/utils/CsvUtils.cpp src/utils/Geo.cpp src/utils/Time.cpp -Isrc -o airwatcher_tests.exe
```

## Notes
- Data is read from `csv_files/`.
- New accounts are persisted in `csv_files/accounts.csv`.
- Execution times (ms) are printed for major algorithms in the CLI.
