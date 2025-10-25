# Build Script pour Helm Synthesizer
# Automatise le nettoyage, la compilation et la création de l'installateur
# Version: 1.0
# Date: 24 octobre 2025

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
$InstallerScript = Join-Path $ProjectRoot "helm-installer.iss"
$InstallerOutput = Join-Path $ProjectRoot "installer"

# Couleurs pour l'affichage
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

# Fonction pour vérifier les prérequis
function Test-Prerequisites {
    Write-ColorOutput "?? Vérification des prérequis..." "Cyan"

    # Vérifier CMake
    try {
        $cmakeVersion = cmake --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput "? CMake trouvé: $($cmakeVersion[0])" "Green"
        }
    } catch {
        Write-ColorOutput "? CMake non trouvé. Veuillez l'installer." "Red"
        exit 1
    }

    # Vérifier Visual Studio Build Tools
    $msbuildPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    )

    $msbuildFound = $false
    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            Write-ColorOutput "? MSBuild trouvé: $path" "Green"
            $msbuildFound = $true
            break
        }
    }

    if (-not $msbuildFound) {
        Write-ColorOutput "??  MSBuild non trouvé aux emplacements standard, utilisation de cmake --build" "Yellow"
    }

    # Vérifier Inno Setup
    if (-not $SkipInstaller) {
        if (Test-Path $InnoSetupPath) {
            Write-ColorOutput "? Inno Setup trouvé: $InnoSetupPath" "Green"
        } else {
            Write-ColorOutput "??  Inno Setup non trouvé à: $InnoSetupPath" "Yellow"
            Write-ColorOutput "   L'installateur ne sera pas créé." "Yellow"
            $script:SkipInstaller = $true
        }
    }

    # Vérifier le script Inno Setup
    if (-not $SkipInstaller -and -not (Test-Path $InstallerScript)) {
        Write-ColorOutput "? Script Inno Setup non trouvé: $InstallerScript" "Red"
        $script:SkipInstaller = $true
    }
}

# Fonction de nettoyage
function Invoke-CleanBuild {
    Write-ColorOutput "?? Nettoyage du projet..." "Yellow"

    if (Test-Path $BuildDir) {
        Write-ColorOutput "   Suppression du répertoire build existant..." "White"
        Remove-Item $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 1
    }

    if (Test-Path $InstallerOutput) {
        Write-ColorOutput "   Suppression du répertoire installer existant..." "White"
        Remove-Item $InstallerOutput -Recurse -Force -ErrorAction SilentlyContinue
    }

    # Nettoyage des fichiers temporaires
    $tempFiles = @("*.tmp", "*.log", "CMakeCache.txt")
    foreach ($pattern in $tempFiles) {
        Get-ChildItem -Path $ProjectRoot -Filter $pattern -Recurse -ErrorAction SilentlyContinue |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }

    Write-ColorOutput "? Nettoyage terminé" "Green"
}

# Fonction de configuration CMake
function Invoke-CMakeConfigure {
    Write-ColorOutput "??  Configuration CMake..." "Cyan"

    Set-Location $ProjectRoot

    $cmakeArgs = @(
        "-S", ".",
        "-B", "build",
        "-DCMAKE_BUILD_TYPE=$Configuration"
    )

    Write-ColorOutput "   Commande: cmake $($cmakeArgs -join ' ')" "White"

    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput "? Erreur lors de la configuration CMake" "Red"
        exit 1
    }

    Write-ColorOutput "? Configuration CMake réussie" "Green"
}

# Fonction de compilation
function Invoke-BuildTargets {
    Write-ColorOutput "?? Compilation des targets..." "Cyan"

    $targets = @(
        @{Name = "HelmStandalone"; Description = "Application Standalone"},
        @{Name = "HelmPlugin_VST3"; Description = "Plugin VST3"},
        @{Name = "HelmPlugin_LV2"; Description = "Plugin LV2"}
    )

    foreach ($target in $targets) {
        Write-ColorOutput "   ?? Compilation de $($target.Description)..." "Yellow"

        $buildArgs = @(
            "--build", "build",
            "--config", $Configuration,
            "--target", $target.Name,
            "-j", [Environment]::ProcessorCount
        )

        & cmake @buildArgs
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput "? Erreur lors de la compilation de $($target.Name)" "Red"
            exit 1
        }

        Write-ColorOutput "? $($target.Description) compilé avec succès" "Green"
    }
}

# Fonction de vérification des artefacts
function Test-BuildArtifacts {
    Write-ColorOutput "?? Vérification des artefacts de build..." "Cyan"

    $artifacts = @(
        @{Path = "build\HelmStandalone_artefacts\$Configuration\helm.exe"; Name = "Helm Standalone"},
        @{Path = "build\HelmPlugin_artefacts\$Configuration\VST3\Helm.vst3"; Name = "Plugin VST3"},
        @{Path = "build\HelmPlugin_artefacts\$Configuration\LV2\Helm.lv2"; Name = "Plugin LV2"}
    )

    $allFound = $true
    foreach ($artifact in $artifacts) {
        $fullPath = Join-Path $ProjectRoot $artifact.Path
        if (Test-Path $fullPath) {
            $item = Get-Item $fullPath -ErrorAction SilentlyContinue
            if ($item) {
                if ($item.PSIsContainer) {
                    # C'est un répertoire (plugin VST3/LV2)
                    $files = Get-ChildItem $fullPath -Recurse -File -ErrorAction SilentlyContinue
                    if ($files.Count -gt 0) {
                        $totalSize = ($files | Measure-Object -Property Length -Sum).Sum
                        $sizeStr = "{0:N1} MB" -f ($totalSize / 1MB)
                        $fileCount = $files.Count
                        Write-ColorOutput "? $($artifact.Name): $fileCount fichier(s), $sizeStr" "Green"
                    } else {
                        Write-ColorOutput "??  $($artifact.Name): répertoire vide" "Yellow"
                        $allFound = $false
                    }
                } else {
                    # C'est un fichier (helm.exe)
                    if ($item.Length -gt 0) {
                        $sizeStr = "{0:N1} MB" -f ($item.Length / 1MB)
                        Write-ColorOutput "? $($artifact.Name): $sizeStr" "Green"
                    } else {
                        Write-ColorOutput "??  $($artifact.Name): fichier vide" "Yellow"
                        $allFound = $false
                    }
                }
            } else {
                Write-ColorOutput "? $($artifact.Name): inaccessible" "Red"
                $allFound = $false
            }
        } else {
            Write-ColorOutput "? $($artifact.Name): non trouvé" "Red"
            $allFound = $false
        }
    }

    if (-not $allFound) {
        Write-ColorOutput "??  Certains artefacts sont manquants, mais on continue..." "Yellow"
    }
}

# Fonction de création de l'installateur
function Invoke-CreateInstaller {
    if ($SkipInstaller) {
        Write-ColorOutput "??  Création de l'installateur ignorée" "Yellow"
        return
    }

    Write-ColorOutput "?? Création de l'installateur..." "Magenta"

    # Créer le répertoire installer s'il n'existe pas
    if (-not (Test-Path $InstallerOutput)) {
        New-Item -ItemType Directory -Path $InstallerOutput -Force | Out-Null
    }

    Set-Location $ProjectRoot

    Write-ColorOutput "   Script: $InstallerScript" "White"
    Write-ColorOutput "   Output: $InstallerOutput" "White"

    & $InnoSetupPath $InstallerScript
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput "? Erreur lors de la création de l'installateur" "Red"
        exit 1
    }

    # Vérifier que l'installateur a été créé
    $installerFiles = Get-ChildItem -Path $InstallerOutput -Filter "helm-setup-*.exe"
    if ($installerFiles.Count -gt 0) {
        $installerFile = $installerFiles[0]
        $size = "{0:N1} MB" -f ($installerFile.Length / 1MB)
        Write-ColorOutput "? Installateur créé: $($installerFile.Name) ($size)" "Green"
        Write-ColorOutput "   Emplacement: $($installerFile.FullName)" "White"
    } else {
        Write-ColorOutput "? Installateur non trouvé dans $InstallerOutput" "Red"
    }
}

# Fonction principale
function Main {
    $startTime = Get-Date

    Write-ColorOutput "?? Build Script Helm Synthesizer" "Magenta"
    Write-ColorOutput "   Configuration: $Configuration" "White"
    Write-ColorOutput "   Heure de début: $($startTime.ToString('HH:mm:ss'))" "White"
    Write-ColorOutput ""

    # Vérifier les prérequis
    Test-Prerequisites

    if ($CleanOnly) {
        Invoke-CleanBuild
        Write-ColorOutput "? Nettoyage uniquement terminé" "Green"
        return
    }

    try {
        # Nettoyage
        if (-not $SkipClean) {
            Invoke-CleanBuild
        }

        # Configuration CMake
        Invoke-CMakeConfigure

        # Compilation
        Invoke-BuildTargets

        # Vérification des artefacts
        Test-BuildArtifacts

        # Création de l'installateur
        Invoke-CreateInstaller

        $endTime = Get-Date
        $duration = $endTime - $startTime

        Write-ColorOutput ""
        Write-ColorOutput "?? Build terminé avec succès!" "Green"
        Write-ColorOutput "   Durée totale: $($duration.ToString('mm\:ss'))" "White"
        Write-ColorOutput "   Heure de fin: $($endTime.ToString('HH:mm:ss'))" "White"

    } catch {
        Write-ColorOutput "? Erreur lors du build: $($_.Exception.Message)" "Red"
        exit 1
    }
}

# Gestion des paramètres d'aide
if ($args -contains "-?" -or $args -contains "-h" -or $args -contains "--help") {
    Write-ColorOutput "Build Script pour Helm Synthesizer" "Magenta"
    Write-ColorOutput ""
    Write-ColorOutput "USAGE:" "White"
    Write-ColorOutput "  .\build.ps1 [OPTIONS]" "White"
    Write-ColorOutput ""
    Write-ColorOutput "OPTIONS:" "White"
    Write-ColorOutput "  -CleanOnly           Nettoie seulement le projet" "White"
    Write-ColorOutput "  -SkipClean           Ignore le nettoyage" "White"
    Write-ColorOutput "  -SkipInstaller       Ignore la création de l'installateur" "White"
    Write-ColorOutput "  -Configuration       Configuration de build (Debug/Release, défaut: Release)" "White"
    Write-ColorOutput "  -InnoSetupPath       Chemin vers ISCC.exe" "White"
    Write-ColorOutput ""
    Write-ColorOutput "EXEMPLES:" "White"
    Write-ColorOutput "  .\build.ps1                    # Build complet en Release" "White"
    Write-ColorOutput "  .\build.ps1 -CleanOnly         # Nettoyage uniquement" "White"
    Write-ColorOutput "  .\build.ps1 -SkipInstaller     # Build sans installateur" "White"
    Write-ColorOutput "  .\build.ps1 -Configuration Debug  # Build en Debug" "White"
    exit 0
}

# Exécution du script principal
Main