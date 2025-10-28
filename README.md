# Attribution

This project is a significantly modified and independent version of the Helm synthesizer.

It originates from the work of Matt Tytel, creator of the original [Helm repository](https://github.com/mtytel/helm).

This particular codebase was started from the [bepzi/helm fork](https://github.com/bepzi/helm).

---

# Helm2025

Helm2025 is a free, cross-platform, polyphonic synthesizer that runs on
GNU/Linux, Mac, and Windows as a standalone program and as a
LV2/VST3/AU plugin.

This is a **modernized fork** of Matt Tytel's original Helm synthesizer, featuring significant performance improvements and code modernization.

![Helm2025 Screenshot](images/ScreenShot.png)

## Building The Project

The build system has been significantly modernized to be robust and user-friendly.

### Prerequisites

- **Git**: Required to clone the repository and its dependencies (submodules).
- **CMake**: Version 3.15 or higher.
- **C++17 Compiler**: A modern C++ compiler (e.g., Visual Studio 2019/2022, GCC 7+, Clang 6+).
- **PowerShell**: For Windows users running the build script.
- **(Optional) ASIO SDK**: For Windows users who need professional low-latency audio. Download from the [Steinberg website](https://www.steinberg.net/developers/) and see ASIO configuration below.

### Build Instructions

The recommended way to build is to use the provided PowerShell script on Windows, or the direct CMake commands on macOS and Linux.

**Windows (Recommended Method):**

Open PowerShell and run the build script:

```powershell
.\build.ps1
```

This script automates all steps: it initializes the required dependencies (JUCE and VST3 SDK), runs CMake, builds all targets, and creates an installer.

**macOS and Linux (or Manual Windows):**

1.  **Initialize Dependencies:** First, initialize the JUCE and VST3 SDK submodules. This only needs to be done once after cloning.

    ```bash
    git submodule update --init --recursive
    ```

2.  **Configure with CMake:**

    ```bash
    cmake -S . -B build
    ```

3.  **Compile:**

    ```bash
    cmake --build build
    ```

### Build System Improvements (October 2025)

- **Automated Dependency Management**: JUCE and the Steinberg VST3 SDK are now included as **Git Submodules**. The build script will automatically download them, so you no longer need to download them manually.

- **Robust Build Script**: A new `build.ps1` script for Windows automates the entire process from cleaning to installer creation. It is fully translated into English and provides clear, color-coded output.

- **ASIO Support**: The GPLv3 licensed version of Steinberg ASIO SDK is included in the projet. 

## Features

- 32 voice polyphony
- Interactive visual interface
- Powerful modulation system with live visual feedback
- Dual oscillators with cross modulation and up to 15 unison oscillators each
- Sub oscillator with shuffle waveshaping
- Oscillator feedback and saturation for waveshaping
- 21 different waveforms including advanced shapes (pulse variations, hybrid waves, textured oscillations)
- Blending between 12 or 24dB low/band/high pass filter
- Low/Band/High Shelf filters
- 2 monophonic and 1 polyphonic LFO
- Step sequencer jusqu'à 64 pas
- Lots of modulation sources including polyphonic aftertouch
- Simple arpeggiator
- Effects: Formant filter, stutter, delay, distortion, reverb

## Nouveautés Step Sequencer (octobre 2025)

- **Jusqu'à 64 pas** : Le step sequencer supporte désormais jusqu'à 64 pas, en édition, modulation et moteur audio, sans crash ni limitation cachée.
- **Stabilité** : Correction d'un crash lors de l'édition des pas >32. Toute la chaîne (paramètres, UI, DSP) gère 64 pas réels.

## LFO Synchronization & Random Improvements (October 2025)

- **Perfect LFO/Visualization Sync**: The modulation and the OpenGL waveform viewer are now perfectly synchronized for all LFOs, including S&H, S&G, and WhiteNoise, both in mono and polyphonic modes.
- **Deterministic Randoms**: S&H, S&G, and WhiteNoise LFOs use a deterministic, cycle-synchronized random sequence. The random values are generated with a reproducible seed, ensuring that the modulation and the visual curve always match, even when the LFO frequency changes dynamically.
- **Musical Step Count**: For S&H and S&G, the number of random steps per LFO cycle is now fixed to a musical value (16), ensuring a musically useful cadence regardless of LFO rate.
- **Cycle-Accurate Renewal**: The random sequence is renewed at every LFO cycle, even in free-running mode (no external reset), guaranteeing a new random pattern for each cycle, both in sound and in the UI.
- **Robust Polyphonic Support**: All improvements apply to both monophonic and polyphonic LFOs, with robust handling of all edge cases (UI switching, frequency changes, etc).
- **Crash-Proof Visualization**: The OpenGL viewer is now robust against all edge cases (buffer underrun, null pointers, etc) and never crashes when switching LFO type or frequency.

These improvements guarantee that what you see in the LFO viewer is always exactly what you hear, with a musical and reliable random modulation for all S&H/S&G/WhiteNoise LFOs.

## Ergonomie : sélection de forme d’onde simplifiée (octobre 2025)

- **Navigation intuitive sur les sélecteurs d’onde** : Sur tous les composants de sélection de forme d’onde (LFO, oscillateurs, sub, etc.), un clic sur la moitié gauche de la zone sélectionne la forme précédente, un clic sur la moitié droite sélectionne la suivante. Cela fonctionne dans :
    - le visualiseur OpenGL (LFO)
    - le WaveSelector (oscillateurs, sub)
    - le WaveViewer (aperçu d’onde)
- Ce comportement accélère la navigation et rend le workflow plus fluide, sans menus contextuels ni clics multiples.

Le composant TextSlider conserve la sélection directe par position du clic (comportement d’origine).
