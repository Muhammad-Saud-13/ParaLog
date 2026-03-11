# ParaLog Build Script for Windows (PowerShell)
# Usage: .\build.ps1 [clean|release|debug|run|test]

param(
    [string]$Action = "release"
)

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

function Build-Project {
    param([string]$BuildType = "Release")
    
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  Building ParaLog ($BuildType)" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    # Create build directory if it doesn't exist
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }
    
    # Navigate to build directory
    Push-Location $BuildDir
    
    try {
        # Run CMake configuration
        Write-Host "`nConfiguring CMake..." -ForegroundColor Yellow
        cmake .. -DCMAKE_BUILD_TYPE=$BuildType
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        # Build the project
        Write-Host "`nBuilding project..." -ForegroundColor Yellow
        cmake --build . --config $BuildType
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "`n[SUCCESS] Build completed successfully!" -ForegroundColor Green
    }
    catch {
        Write-Host "`n[ERROR] Build failed: $_" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Pop-Location
}

function Clean-Project {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "[SUCCESS] Build directory cleaned" -ForegroundColor Green
    }
    else {
        Write-Host "Build directory does not exist" -ForegroundColor Gray
    }
}

function Run-Project {
    param([string]$LogFile = "")
    
    $ExePath = Join-Path $BuildDir "bin\paralog.exe"
    
    if (-not (Test-Path $ExePath)) {
        Write-Host "[ERROR] Executable not found. Build the project first." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "`nRunning ParaLog..." -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan
    
    if ($LogFile -eq "") {
        & $ExePath
    }
    else {
        & $ExePath $LogFile
    }
}

# Main script logic
switch ($Action.ToLower()) {
    "clean" {
        Clean-Project
    }
    "release" {
        Build-Project "Release"
    }
    "debug" {
        Build-Project "Debug"
    }
    "run" {
        Run-Project
    }
    "test" {
        $SampleLog = Join-Path $ProjectRoot "data\sample.log"
        Run-Project $SampleLog
    }
    default {
        Write-Host "Usage: .\build.ps1 [clean|release|debug|run|test]" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Commands:"
        Write-Host "  clean   - Remove build directory"
        Write-Host "  release - Build in Release mode (default)"
        Write-Host "  debug   - Build in Debug mode"
        Write-Host "  run     - Run the application"
        Write-Host "  test    - Run with sample log file"
    }
}
