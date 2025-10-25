# Attribution

This project is a significantly modified and independent version of the Helm synthesizer.

It originates from the work of Matt Tytel, creator of the original [Helm repository](https://github.com/mtytel/helm).

This particular codebase was started from the [bepzi/helm fork](https://github.com/bepzi/helm).

---

# Helm

Helm is a free, cross-platform, polyphonic synthesizer that runs on
GNU/Linux, Mac, and Windows as a standalone program and as a
LV2/VST3/AU plugin.

This is a **modernized fork** of Matt Tytel's original Helm synthesizer, featuring significant performance improvements and code modernization.

![alt tag](http://tytel.org/static/images/helm_screenshot.png)

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

- **Conditional ASIO Support**: The build system now automatically detects if you have the ASIO SDK. 
    - If you do, it enables ASIO support for the standalone application.
    - If you don't, it skips ASIO support and continues the build without errors.
    - To use it, set the `HELM_ASIO_SDK_PATH` variable during CMake configuration: `cmake -S . -B build -DHELM_ASIO_SDK_PATH="C:/path/to/ASIOSDK"`

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
- Step sequencer
- Lots of modulation sources including polyphonic aftertouch
- Simple arpeggiator
- Effects: Formant filter, stutter, delay, distortion, reverb
