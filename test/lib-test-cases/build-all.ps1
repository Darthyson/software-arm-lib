#!/usr/bin/env pwsh
# Build all CMake presets for sblib

param(
    [switch]$Clean,
    [switch]$Configure
)

$ErrorActionPreference = "Stop"

# Define all build presets
$buildPresets = @(
    "debug-x86",
    "release-x86",
    "debug-x64",
    "release-x64",
    "debug-x86-logging",
    "debug-x64-logging"
)

# Define corresponding configure presets
$configurePresets = @{
    "debug-x86" = "x86"
    "release-x86" = "x86"
    "debug-x64" = "x64"
    "release-x64" = "x64"
    "debug-x86-logging" = "x86_logging"
    "debug-x64-logging" = "x64_logging"
}

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Building all lib-test-cases presets" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$successful = @()
$failed = @()

foreach ($preset in $buildPresets) {
    Write-Host "----------------------------------------" -ForegroundColor Yellow
    Write-Host "Building preset: $preset" -ForegroundColor Yellow
    Write-Host "----------------------------------------" -ForegroundColor Yellow
    
    try {
        # Configure if requested or if build directory doesn't exist
        $configPreset = $configurePresets[$preset]
        $buildDir = "cmake-build/$configPreset"
        
        if ($Configure -or $Clean -or -not (Test-Path $buildDir)) {
            Write-Host "Configuring preset: $configPreset" -ForegroundColor Cyan
            
            if ($Clean -and (Test-Path $buildDir)) {
                Write-Host "Cleaning build directory: $buildDir" -ForegroundColor Magenta
                Remove-Item -Recurse -Force $buildDir
            }
            
            cmake --preset $configPreset
            if ($LASTEXITCODE -ne 0) {
                throw "Configuration failed for preset $configPreset"
            }
        }
        
        # Build
        Write-Host "Building preset: $preset" -ForegroundColor Cyan
        cmake --build --preset $preset
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Successfully built: $preset" -ForegroundColor Green
            $successful += $preset
        } else {
            throw "Build failed"
        }
    }
    catch {
        Write-Host "✗ Failed to build: $preset" -ForegroundColor Red
        Write-Host "Error: $_" -ForegroundColor Red
        $failed += $preset
    }
    
    Write-Host ""
}

# Summary
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Build Summary lib-test-cases" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Successful: $($successful.Count)/$($buildPresets.Count)" -ForegroundColor Green
foreach ($preset in $successful) {
    Write-Host "  ✓ $preset" -ForegroundColor Green
}

if ($failed.Count -gt 0) {
    Write-Host "Failed: $($failed.Count)/$($buildPresets.Count)" -ForegroundColor Red
    foreach ($preset in $failed) {
        Write-Host "  ✗ $preset" -ForegroundColor Red
    }
    exit 1
}

Write-Host ""
Write-Host "All builds completed successfully!" -ForegroundColor Green
