# ParaLog Build Script for Windows (PowerShell)
# Usage: .\build.ps1 [clean|release|debug|run|test]

param(
    [string]$Action = "debug" # Default to debug for better diagnostics
)

$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

function Build-Project {
    param([string]$BuildType = "Release") # Default to Release
    
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
        cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=$BuildType
        
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
    finally {
        Pop-Location
    }
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
        Build-Project -BuildType "Release"
    }
    "debug" {
        Build-Project -BuildType "Debug"
    }
    "run" {
        # Determine build type to run, default to Release
        $BuildTypeForRun = "Release"
        $ExePath = Join-Path $BuildDir "bin/paralog.exe"

        if (-not (Test-Path $ExePath)) {
            Write-Host "Executable not found. Building release version first..." -ForegroundColor Yellow
            Build-Project -BuildType $BuildTypeForRun
        }
        
        # Forward remaining arguments to the executable
        $RemainingArgs = $args | Where-Object { $_ -ne $Action }
        
        Write-Host "`nRunning ParaLog..." -ForegroundColor Cyan
        & $ExePath $RemainingArgs
    }
    "test" {
        Write-Host "Test runner not yet implemented." -ForegroundColor Yellow
    }
    default {
        Write-Host "Unknown action: $Action" -ForegroundColor Red
        Write-Host "Usage: .\build.ps1 [clean|release|debug|run|test]"
        exit 1
    }
}
