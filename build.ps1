# Build Script for Helm Synthesizer
# Automates cleaning, compilation, and installer creation
# Version: 1.2
# Date: 25 October 2025

param(
    [switch]$CleanOnly,
    [switch]$SkipClean,
    [switch]$SkipInstaller,
    [string]$Configuration = "Release",
    [string]$InnoSetupPath = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)

# Configuration
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build"
$InstallerScript = Join-Path $ProjectRoot "helm2025-installer.iss"
$InstallerOutput = Join-Path $ProjectRoot "installer"

# Colors for output
function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    switch ($Color) {
        "Green" { Write-Host $Message -ForegroundColor Green }
        "Yellow" { Write-Host $Message -ForegroundColor Yellow }
        "Red" { Write-Host $Message -ForegroundColor Red }
        "Cyan" { Write-Host $Message -ForegroundColor Cyan }
        "Magenta" { Write-Host $Message -ForegroundColor Magenta }
        default { Write-Host $Message }
    }
}

# Function to check for prerequisites
function Test-Prerequisites {
    Write-ColorOutput "==> Verifying prerequisites..." "Cyan"

    # Update Git submodules
    Write-ColorOutput " *  Updating Git submodules (JUCE)..." "Cyan"
    git submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput "!! Error while updating submodules." "Red"
        exit 1
    }
    Write-ColorOutput " *  Submodules updated successfully." "Green"

    # Check for CMake
    try {
        $cmakeVersion = cmake --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput " *  CMake found: $($cmakeVersion[0])" "Green"
        }
    } catch {
        Write-ColorOutput "!! CMake not found. Please install it." "Red"
        exit 1
    }

    # Check for Visual Studio Build Tools
    $msbuildPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    )

    $msbuildFound = $false
    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            Write-ColorOutput " *  MSBuild found: $path" "Green"
            $msbuildFound = $true
            break
        }
    }

    if (-not $msbuildFound) {
        Write-ColorOutput " !  MSBuild not found in standard locations, using cmake --build" "Yellow"
    }

    # Check for Inno Setup
    if (-not $SkipInstaller) {
        if (Test-Path $InnoSetupPath) {
            Write-ColorOutput " *  Inno Setup found: $InnoSetupPath" "Green"
        } else {
            Write-ColorOutput " !  Inno Setup not found at: $InnoSetupPath" "Yellow"
            Write-ColorOutput "   The installer will not be created." "Yellow"
            $script:SkipInstaller = $true
        }
    }

    # Check for Inno Setup script
    if (-not $SkipInstaller -and -not (Test-Path $InstallerScript)) {
        Write-ColorOutput "!! Inno Setup script not found: $InstallerScript" "Red"
        $script:SkipInstaller = $true
    }
}

# Clean function
function Invoke-CleanBuild {
    Write-ColorOutput "==> Cleaning project..." "Yellow"

    if (Test-Path $BuildDir) {
        Write-ColorOutput "   Removing existing build directory..." "White"
        Remove-Item $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 1
    }

    if (Test-Path $InstallerOutput) {
        Write-ColorOutput "   Removing existing installer directory..." "White"
        Remove-Item $InstallerOutput -Recurse -Force -ErrorAction SilentlyContinue
    }

    # Clean temporary files
    $tempFiles = @("*.tmp", "*.log", "CMakeCache.txt")
    foreach ($pattern in $tempFiles) {
        Get-ChildItem -Path $ProjectRoot -Filter $pattern -Recurse -ErrorAction SilentlyContinue |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }

    Write-ColorOutput " *  Cleaning finished" "Green"
}

# CMake configure function
function Invoke-CMakeConfigure {
    Write-ColorOutput "==> Configuring CMake..." "Cyan"

    Set-Location $ProjectRoot

    $cmakeArgs = @(
        "-S", ".",
        "-B", "build",
        "-DCMAKE_BUILD_TYPE=$Configuration"
    )

    Write-ColorOutput "   Command: cmake $($cmakeArgs -join ' ')" "White"

    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput "!! Error during CMake configuration" "Red"
        exit 1
    }

    Write-ColorOutput " *  CMake configuration successful" "Green"
}

# Build function
function Invoke-BuildTargets {
    Write-ColorOutput "==> Compiling targets..." "Cyan"

    $targets = @(
        @{Name = "Helm2025Standalone"; Description = "Standalone Application"},
        @{Name = "Helm2025Plugin_VST3"; Description = "VST3 Plugin"},
        @{Name = "Helm2025Plugin_LV2"; Description = "LV2 Plugin"}
    )

    foreach ($target in $targets) {
        Write-ColorOutput " *  Compiling $($target.Description)..." "Yellow"

        $buildArgs = @(
            "--build", "build",
            "--config", $Configuration,
            "--target", $target.Name,
            "-j", [Environment]::ProcessorCount
        )

        & cmake @buildArgs
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput "!! Error while compiling $($target.Name)" "Red"
            exit 1
        }

        Write-ColorOutput " *  $($target.Description) compiled successfully" "Green"
    }
}

# Artifact verification function
function Test-BuildArtifacts {
    Write-ColorOutput "==> Verifying build artifacts..." "Cyan"

    $artifacts = @(
        @{Path = "build\Helm2025Standalone_artefacts\$Configuration\helm2025.exe"; Name = "Helm2025 Standalone"},
        @{Path = "build\Helm2025Plugin_artefacts\$Configuration\VST3\Helm2025.vst3"; Name = "VST3 Plugin"},
        @{Path = "build\Helm2025Plugin_artefacts\$Configuration\LV2\Helm2025.lv2"; Name = "LV2 Plugin"}
    )

    $allFound = $true
    foreach ($artifact in $artifacts) {
        $fullPath = Join-Path $ProjectRoot $artifact.Path
        if (Test-Path $fullPath) {
            $item = Get-Item $fullPath -ErrorAction SilentlyContinue
            if ($item) {
                if ($item.PSIsContainer) {
                    # It's a directory (VST3/LV2 plugin)
                    $files = Get-ChildItem $fullPath -Recurse -File -ErrorAction SilentlyContinue
                    if ($files.Count -gt 0) {
                        $totalSize = ($files | Measure-Object -Property Length -Sum).Sum
                        $sizeStr = "{0:N1} MB" -f ($totalSize / 1MB)
                        $fileCount = $files.Count
                        Write-ColorOutput " *  $($artifact.Name): $fileCount file(s), $sizeStr" "Green"
                    } else {
                        Write-ColorOutput " !  $($artifact.Name): empty directory" "Yellow"
                        $allFound = $false
                    }
                } else {
                    # It's a file (helm.exe)
                    if ($item.Length -gt 0) {
                        $sizeStr = "{0:N1} MB" -f ($item.Length / 1MB)
                        Write-ColorOutput " *  $($artifact.Name): $sizeStr" "Green"
                    } else {
                        Write-ColorOutput " !  $($artifact.Name): empty file" "Yellow"
                        $allFound = $false
                    }
                }
            } else {
                Write-ColorOutput "!! $($artifact.Name): inaccessible" "Red"
                $allFound = $false
            }
        } else {
            Write-ColorOutput "!! $($artifact.Name): not found" "Red"
            $allFound = $false
        }
    }

    if (-not $allFound) {
        Write-ColorOutput " !  Some artifacts are missing, but continuing..." "Yellow"
    }
}

# Installer creation function
function Invoke-CreateInstaller {
    if ($SkipInstaller) {
        Write-ColorOutput "==> Installer creation skipped" "Yellow"
        return
    }

    Write-ColorOutput "==> Creating installer..." "Magenta"

    # Create installer directory if it doesn't exist
    if (-not (Test-Path $InstallerOutput)) {
        New-Item -ItemType Directory -Path $InstallerOutput -Force | Out-Null
    }

    Set-Location $ProjectRoot

    Write-ColorOutput "   Script: $InstallerScript" "White"
    Write-ColorOutput "   Output: $InstallerOutput" "White"

    & $InnoSetupPath $InstallerScript
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput "!! Error while creating the installer" "Red"
        exit 1
    }

    # Verify that the installer was created
    $installerFiles = Get-ChildItem -Path $InstallerOutput -Filter "helm2025-setup-*.exe"
    if ($installerFiles.Count -gt 0) {
        $installerFile = $installerFiles[0]
        $size = "{0:N1} MB" -f ($installerFile.Length / 1MB)
        Write-ColorOutput " *  Installer created: $($installerFile.Name) ($size)" "Green"
        Write-ColorOutput "   Location: $($installerFile.FullName)" "White"
    } else {
        Write-ColorOutput "!! Installer not found in $InstallerOutput" "Red"
    }
}

# Main function
function Main {
    $startTime = Get-Date

    Write-ColorOutput "==> Helm2025 Synthesizer Build Script" "Magenta"
    Write-ColorOutput "   Configuration: $Configuration" "White"
    Write-ColorOutput "   Start time: $($startTime.ToString('HH:mm:ss'))" "White"
    Write-ColorOutput ""

    # Verify prerequisites
    Test-Prerequisites

    if ($CleanOnly) {
        Invoke-CleanBuild
        Write-ColorOutput " *  Clean only finished" "Green"
        return
    }

    try {
        # Clean
        if (-not $SkipClean) {
            Invoke-CleanBuild
        }

        # Configure CMake
        Invoke-CMakeConfigure

        # Compile
        Invoke-BuildTargets

        # Verify artifacts
        Test-BuildArtifacts

        # Create installer
        Invoke-CreateInstaller

        $endTime = Get-Date
        $duration = $endTime - $startTime

        Write-ColorOutput ""
        Write-ColorOutput "==> Build finished successfully!" "Green"
        Write-ColorOutput "   Total duration: $($duration.ToString('mm\:ss'))" "White"
        Write-ColorOutput "   End time: $($endTime.ToString('HH:mm:ss'))" "White"

    } catch {
        Write-ColorOutput "!! Error during build: $($_.Exception.Message)" "Red"
        exit 1
    }
}

# Help parameter handling
if ($args -contains "-?" -or $args -contains "-h" -or $args -contains "--help") {
    Write-ColorOutput "Build Script for Helm2025 Synthesizer" "Magenta"
    Write-ColorOutput ""
    Write-ColorOutput "USAGE:" "White"
    Write-ColorOutput "  .\build.ps1 [OPTIONS]" "White"
    Write-ColorOutput ""
    Write-ColorOutput "OPTIONS:" "White"
    Write-ColorOutput "  -CleanOnly           Only cleans the project" "White"
    Write-ColorOutput "  -SkipClean           Skips the cleaning step" "White"
    Write-ColorOutput "  -SkipInstaller       Skips installer creation" "White"
    Write-ColorOutput "  -Configuration       Build configuration (Debug/Release, default: Release)" "White"
    Write-ColorOutput "  -InnoSetupPath       Path to ISCC.exe" "White"
    Write-ColorOutput ""
    Write-ColorOutput "EXAMPLES:" "White"
    Write-ColorOutput "  .\build.ps1                    # Full build in Release" "White"
    Write-ColorOutput "  .\build.ps1 -CleanOnly         # Clean only" "White"
    Write-ColorOutput "  .\build.ps1 -SkipInstaller     # Build without installer" "White"
    Write-ColorOutput "  .\build.ps1 -Configuration Debug  # Build in Debug" "White"
    exit 0
}

# Execute main script
Main