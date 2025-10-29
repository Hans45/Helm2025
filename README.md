
# Helm2025

Helm2025 is a modern, cross-platform, polyphonic synthesizer for GNU/Linux, macOS, and Windows. It is available as a standalone application and as LV2, VST3, and AU plugins.

![Helm2025 Screenshot](images/ScreenShot.png)

---

## About This Project / À propos du projet

Helm2025 is a significantly modernized and independent fork of Matt Tytel's original [Helm synthesizer](https://github.com/mtytel/helm), started from the [bepzi/helm fork](https://github.com/bepzi/helm). It features major performance, stability, and usability improvements, as well as new synthesis and UI features.

Helm2025 est un fork moderne et indépendant du synthétiseur Helm original de Matt Tytel, basé sur le fork [bepzi/helm](https://github.com/bepzi/helm). Il apporte de nombreuses améliorations de performance, de stabilité, d’ergonomie et de nouvelles fonctionnalités de synthèse et d’interface.

---

## Key Features / Fonctionnalités principales

- Up to **32-voice polyphony** / Jusqu’à **32 voix de polyphonie**
- **Step sequencer up to 64 steps** (editing, modulation, audio engine, and patch save/load) / **Séquenceur jusqu’à 64 pas** (édition, modulation, moteur audio, sauvegarde de patch)
- Interactive, real-time visual interface / Interface visuelle interactive en temps réel
- Powerful modulation system with live feedback / Système de modulation puissant avec retour visuel
- Dual oscillators with cross-modulation and up to 15 unison voices each / Deux oscillateurs avec cross-modulation et jusqu’à 15 voix d’unisson chacun
- Sub oscillator with shuffle waveshaping / Oscillateur sub avec waveshaping shuffle
- Oscillator feedback and saturation / Feedback et saturation d’oscillateur
- 21 waveforms, including advanced and hybrid shapes / 21 formes d’onde, y compris des formes avancées et hybrides
- Blendable 12/24dB low/band/high pass and shelf filters / Filtres passe-bas, passe-haut, passe-bande et shelf 12/24dB
- 2 monophonic and 1 polyphonic LFO / 2 LFO monophoniques et 1 polyphonique
- Many modulation sources, including polyphonic aftertouch / Nombreuses sources de modulation, aftertouch polyphonique inclus
- Simple arpeggiator / Arpégiateur simple
- Effects: formant filter, stutter, delay, distortion, reverb / Effets : filtre formant, stutter, delay, distorsion, réverbe

---

## Major Improvements – October 2025 / Améliorations majeures – Octobre 2025

### Robust User Preferences Persistence / Persistance robuste des préférences utilisateur

- All UI and audio settings (window size, UI zoom, audio/MIDI device, sample rate, buffer size, audio ports, etc.) are now saved and restored independently and robustly.
- Preferences are never overwritten by defaults at startup; only the actually applied values are saved.
- Immediate save on any change (window resize, device/port/sample rate/buffer change, etc.).
- The system works for all device types, including ASIO on Windows.
- Preferences are stored in a JSON file in the user’s application data directory.

---

### Audio Configuration Restoration & Warning / Restauration de la configuration audio & avertissement

- At startup, the app restores the audio device, type, sample rate, buffer size, and ports as saved in the JSON preferences.
- If the applied audio configuration differs from the saved preferences (e.g., device unavailable, ASIO fallback), a warning is shown to the user (in English) before any overwrite.
- This ensures the user is always aware if their configuration could not be fully restored.

---

### Uninstall Option: Remove Preferences / Option de désinstallation : suppression des préférences

- The Windows installer (Inno Setup) now asks, during uninstall, if you want to delete your configuration/preferences (JSON file and folder).
- The message is shown in English or French, matching the installer language.
- Preferences are only deleted if the user confirms.

---

### Multilingual User Experience / Expérience utilisateur multilingue

- All new dialogs and uninstall options are available in both English and French.
- The installer and all user-facing messages adapt to the selected language.

---

## Build Instructions / Instructions de compilation

### Prerequisites / Prérequis
- **Git** (for cloning the repository and submodules) / pour cloner le dépôt et les sous-modules
- **CMake** (version 3.15 or higher) / version 3.15 ou supérieure
- **C++17 compiler** (e.g., Visual Studio 2019/2022, GCC 7+, Clang 6+) / compilateur C++17
- **PowerShell** (for Windows users running the build script) / pour le script Windows
- **(Optional) ASIO SDK** (for professional low-latency audio on Windows) / optionnel pour l’audio pro sous Windows

### Quick Start (EN)
1. Open PowerShell in the project directory.
2. Run:
   ```powershell
   .\build.ps1
   ```
   This script will initialize dependencies, run CMake, build all targets, and create an installer.

### Démarrage rapide (FR)
1. Ouvrez PowerShell dans le dossier du projet.
2. Lancez :
   ```powershell
   .\build.ps1
   ```
   Ce script initialise les dépendances, lance CMake, compile tout et crée l’installateur.

---

## License & Credits / Licence & crédits

Helm2025 is free software, released under the GNU GPLv3. It is based on the original Helm synthesizer by Matt Tytel, with major modifications and new features by Hans45 and contributors. See the source code and [COPYING](COPYING) for details.

Helm2025 est un logiciel libre sous licence GNU GPLv3, basé sur Helm de Matt Tytel, avec de nombreuses modifications et nouveautés par Hans45 et contributeurs. Voir le code source et [COPYING](COPYING) pour plus de détails.

---

For more information, screenshots, and updates, visit the project repository.

Pour plus d’informations, de captures d’écran et de mises à jour, consultez le dépôt du projet.

## Building Helm2025

### Prerequisites
- **Git** (for cloning the repository and submodules)
- **CMake** (version 3.15 or higher)
- **C++17 compiler** (e.g., Visual Studio 2019/2022, GCC 7+, Clang 6+)
- **PowerShell** (for Windows users running the build script)
- **(Optional) ASIO SDK** (for professional low-latency audio on Windows; see [Steinberg website](https://www.steinberg.net/developers/))

### Quick Start (Recommended)

#### Windows
1. Open PowerShell in the project directory.
2. Run:
   ```powershell
   .\build.ps1
   ```
   This script will initialize dependencies, run CMake, build all targets, and create an installer.

#### macOS & Linux (or manual Windows)
1. Initialize dependencies:
   ```bash
   git submodule update --init --recursive
   ```
2. Configure with CMake:
   ```bash
   cmake -S . -B build
   ```
3. Build:
   ```bash
   cmake --build build
   ```

### Build System Highlights
- **Automated dependency management**: JUCE and Steinberg VST3 SDK are included as Git submodules and downloaded automatically.
- **Robust build script**: The Windows build script automates the entire process, with clear, color-coded output.
- **ASIO support**: The GPLv3 Steinberg ASIO SDK is included for Windows builds.

---

## License & Credits

Helm2025 is free software, released under the GNU GPLv3. It is based on the original Helm synthesizer by Matt Tytel, with major modifications and new features by Hans45 and contributors. See the source code and [COPYING](COPYING) for details.

---

For more information, screenshots, and updates, visit the project repository.
