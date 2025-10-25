# Attribution

This project is a significantly modified and independent version of the Helm synthesizer.

It originates from the work of Matt Tytel, creator of the original [Helm repository](https://github.com/mtytel/helm).

This particular codebase was started from the [bepzi/helm fork](https://github.com/bepzi/helm).

---

# Helm

Helm is a free, cross-platform, polyphonic synthesizer that runs on
GNU/Linux, Mac, and Windows as a standalone program and as a
LV2/VST3/AU plugin.

This is a **modernized fork** of Matt Tytel's original Helm synthesizer, featuring significant performance improvements and code modernization. Our enhancements include:

- Modern C++17 code base with enhanced type safety
- SIMD optimizations for improved audio processing performance
- Optimized parameter interpolation for smoother sound transitions
- Updated build system with modern CMake
- Up-to-date VST3 SDK integration
- Improved cross-platform compatibility
- Enhanced MIDI handling
- Modern development workflow with GitHub Actions

![alt tag](http://tytel.org/static/images/helm_screenshot.png)

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

## Building

The build system has been modernized to use CMake 3.15+ with improved configuration options.

Prerequisites:

- CMake 3.15 or higher
- Modern C++17 compatible compiler
- VST3 SDK (automatically downloaded)
- JUCE framework (automatically downloaded)
- ASIO SDK (optional, for professional audio driver support on Windows)

To build everything:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)  # Linux/macOS
cmake --build build             # Windows
```

To build specific plugin formats:

```bash
cmake --build build --target HelmPlugin_<PLUGIN_FORMAT>
```

Where `<PLUGIN_FORMAT>` is one of:

- `VST3` - Modern plugin format with improved features
- `LV2` - Open source plugin format
- `AU` - Audio Units (macOS only)

To build the standalone application:

```bash
cmake --build build --target HelmStandalone
```

Build artifacts will be placed in:

- VST3: `build/HelmPlugin_artefacts/Release/VST3/`
- LV2: `build/HelmPlugin_artefacts/Release/LV2/`
- Standalone: `build/HelmStandalone_artefacts/Release/`

### ASIO Support (Windows)

For professional low-latency audio on Windows, ASIO driver support can be enabled:

1. Download the ASIO SDK from Steinberg
2. Extract to a directory (e.g., `G:\ASIOSDK`)
3. The build system will automatically detect and use ASIO if available
4. ASIO drivers will appear in the standalone application's audio device settings

ASIO support provides:

- Ultra-low latency audio processing
- Professional audio interface compatibility
- Direct hardware access for optimal performance
- Support for high sample rates and multiple I/O channels

### Factory Presets Installation

For the standalone application to access factory presets, patches must be installed to system-specific locations:

**Windows Installation:**

Factory presets must be copied to the public documents directory:

```powershell
# Create the directory
$publicDocs = [Environment]::GetFolderPath("CommonDocuments")
New-Item -Path "$publicDocs\Helm\Patches" -ItemType Directory -Force

# Copy factory presets
Copy-Item "patches\Factory Presets" "$publicDocs\Helm\Patches\" -Recurse -Force
```

This typically installs to: `C:\Users\Public\Documents\Helm\Patches\Factory Presets\`

**User Presets Directory:**

Personal patches are stored in your user documents:

```powershell
# Create user patches directory
$userDocs = [Environment]::GetFolderPath("MyDocuments")
New-Item -Path "$userDocs\Helm\Patches" -ItemType Directory -Force
```

**Linux Installation:**

Factory presets should be installed to: `/usr/share/helm/patches`

**macOS Installation:**

Factory presets should be installed to: `/Library/Audio/Presets/Helm`

**Note:** Without proper installation, the standalone application will not display any presets in the patch browser, though the synthesizer functionality remains fully operational.

### Visual Studio Development

For Windows developers using Visual Studio, a solution file is available:

**Solution Files:**

- `Helm.sln` (root directory) - Main solution file for easy access
- `build/HELM.sln` (build directory) - CMake-generated solution file

**Project Structure:**

- **HelmStandalone** - Standalone application with ASIO support
- **HelmPlugin_VST3** - VST3 plugin format
- **HelmPlugin_LV2** - LV2 plugin format
- **HelmData** - Shared data and resources
- **mopo** - Core synthesis engine

**Available Configurations:**

- **Debug|x64** - Debug build with symbols
- **Release|x64** - Optimized release build (recommended)
- **MinSizeRel|x64** - Size-optimized build
- **RelWithDebInfo|x64** - Release with debug info

**Quick Start:**

1. Install factory presets (see [Factory Presets Installation](#factory-presets-installation))
2. Open `Helm.sln` in Visual Studio
3. Select **Release|x64** configuration
4. Set **HelmStandalone** as startup project
5. Press F5 to build and run

## Technical Improvements

### Performance Optimizations

- Implemented SIMD vectorization for core DSP operations
- Optimized parameter interpolation for smoother audio transitions
- Enhanced audio buffer processing
- Improved realtime performance for live usage
- Efficient waveform lookup tables with pre-calculated anti-aliasing for 21 waveform types
- Optimized memory layout for new waveform generators reducing CPU overhead

### Code Modernization

- Updated codebase to C++17 standards
- Enhanced type safety throughout the project
- Modern smart pointer usage
- Improved error handling and safety checks
- Better const-correctness
- Expanded synthesis engine with 8 new mathematically-designed waveforms
- Modern waveform generation algorithms with optimized harmonic calculations

### Build System Improvements

- Modern CMake with proper target management
- Automated dependency handling
- Improved cross-platform compatibility
- Proper version management
- Enhanced testing capabilities

## Developer Notes

### Windows Development

- Windows headers define macros named `min`/`max` which can break C++ code. Some source files in this project define `NOMINMAX` before including `windows.h` to avoid those macro collisions. If you add new Windows-specific includes, prefer adding `#ifndef NOMINMAX\n#define NOMINMAX\n#endif` before the include and leave a short comment explaining why.
- VST3 plugins should be installed to `%ProgramFiles%\Common Files\VST3`
- LV2 plugins should be installed to `%COMMONPROGRAMFILES%\LV2`

### Modern JUCE Integration

- This repository builds JUCE with modal loops disabled (see `JUCE_MODAL_LOOPS_PERMITTED`). Because of that, JUCE's blocking `FileChooser` APIs (browseForFileToSave / showDialog) are unavailable; use `FileChooser::launchAsync` and run UI work on the message thread (for example with `MessageManager::callAsync`) instead.
- Uses modern JUCE plugin formats and features
- Improved MIDI handling and timing

### Testing

- Tested with modern DAWs and plugin hosts
- Verified compatibility with current operating systems
- Performance testing with various audio buffer sizes

## Recent Enhancements (October 2025)

### Import/Export Bank Functionality

- **Fixed non-working import/export bank features** that previously showed no response when clicked
- Implemented proper asynchronous FileChooser pattern using `std::shared_ptr<FileChooser>`
- Added comprehensive error handling for user cancellation scenarios
- Integrated debug messaging system with AlertWindow for operation tracking
- Automatic patch list refresh after successful import operations
- Full compatibility with JUCE's modern async file handling requirements

### Dynamic User Interface Improvements

- **Browse Button Enhancement**: Implemented intelligent label switching functionality
- Button automatically changes from "browse" to "edit" when patch browser is visible
- Returns to "browse" when browser is closed, providing clear visual feedback
- Integrated VisibilityListener pattern for real-time UI state tracking
- Enhanced user experience with contextual button labeling

### Professional Audio Driver Support

- **ASIO Driver Integration**: Added full support for professional ASIO audio drivers
- Configured ASIO SDK integration with proper include paths and definitions
- Enabled low-latency professional audio interfaces in standalone application
- ASIO drivers now appear in audio device selection alongside WASAPI and DirectSound
- Critical for professional audio production workflows requiring minimal latency

### Patch Browser Navigation Enhancements

- **Category Grouping**: Implemented intelligent category consolidation in the patch browser
- When no specific bank is selected, duplicate categories (Bass, Lead, Pad, etc.) are automatically grouped
- Visual indication "(All)" added to grouped categories for clarity
- Selecting a grouped category displays patches from all banks containing that category
- Eliminates duplicate category listings when browsing across multiple preset banks
- Maintains normal behavior when specific banks are selected
- Improves navigation efficiency and reduces visual clutter in the patch browser

### Enhanced Waveform Collection

- **Expanded Waveform Library**: Significantly enriched the oscillator and LFO waveform collection from 13 to 21 different shapes
- **Advanced Pulse Waves**: Added variable pulse width waveforms
  - **Pulse 25%** - Narrow pulse wave with 25% duty cycle for punchy, percussive sounds
  - **Pulse 10%** - Ultra-narrow pulse wave with 10% duty cycle for sharp, cutting tones
- **Hybrid Waveforms**: Innovative combinations of classic waveforms
  - **Saw+Square** - 60% sawtooth + 40% square wave blend for rich harmonic content
  - **Tri+Square** - 70% triangle + 30% square wave mix for warm yet edgy timbres
- **Textured Oscillations**: Organic and expressive waveform variations
  - **Skewed Sine** - Asymmetric sine wave with different rise/fall characteristics
  - **Folded Sine** - Sine wave with wavefold distortion for complex harmonics
  - **Super Saw** - Superposition of 7 slightly detuned sawtooth waves for ultra-rich textures
  - **Chirp** - Frequency sweep waveform transitioning from 1x to 5x base frequency
- **Professional Anti-Aliasing**: All new waveforms include full band-limited implementations
  - Prevents digital artifacts and maintains clean sound at all frequencies
  - Optimized lookup tables with pre-calculated harmonic content
  - Seamless integration with existing oscillator and LFO systems
- **Creative Sound Design**: Dramatically expanded sonic palette
  - Percussive basses with narrow pulse waves
  - Rich leads with hybrid waveforms and super saw textures
  - Cinematic effects with chirp sweeps and folded distortions
  - Organic modulations with skewed and textured LFO shapes

### Mopo Engine C++17 Modernization

- **Complete Engine Modernization**: Updated the core mopo synthesis engine from C++11 to C++17
- **constexpr Optimizations**: Implemented conditional constexpr macros for compile-time constants
  - MOPO_CONSTEXPR for performance-critical mathematical constants
  - Maintains backward compatibility with older compilers via conditional compilation
- **Enhanced Macro System**: Introduced MOPO_DECLARE_CLONE macro for streamlined object cloning
  - Eliminates code duplication in processor clone methods
  - Improves maintainability across all mopo engine components
- **Modern Type Management**: Added using declarations for cleaner type definitions
  - using mopo_float = double for consistent floating-point precision
  - Improved code readability and type safety throughout the engine
- **Performance Improvements**: Compile-time constant evaluation for mathematical operations
  - Reduced runtime overhead for frequently-used constants
  - Enhanced optimization opportunities for modern compilers

### Resource and Asset Optimization

- **Icon System Consolidation**: Streamlined UI resource management
  - Removed unused PNG icon variants to reduce binary size
  - Maintained existing functionality with optimized asset loading
  - Cleaned up CMakeLists.txt BinaryData dependencies
  - Resolved compilation warnings from missing resource files
- **Build Warning Elimination**: Removed deprecated JUCE splash screen configuration
  - Eliminated MSVC warnings from obsolete JUCE_DISPLAY_SPLASH_SCREEN flag
  - Cleaner compilation output without functional impact
  - Modern JUCE framework compatibility maintained

### Code Quality Improvements

- Implemented modern C++ async patterns for file operations
- Enhanced memory management with RAII principles
- Proper object lifetime management in callback scenarios
- Silent handling of user cancellation without error propagation
- Comprehensive debug logging for troubleshooting and validation
- C++17 standard compliance throughout the mopo engine
- Improved code maintainability with modern macro patterns

### Build System Updates

- **CMake Configuration**: Updated for ASIO SDK integration and C++17 requirements
- Proper compiler definitions for audio driver support
- Enhanced include directory management for external SDKs
- Cross-platform compatibility maintained while adding Windows-specific features
- Validated build process with Visual Studio 2022 and modern toolchains
- Optimized binary data compilation with cleaned resource dependencies
- Warning-free compilation with modern compiler standards

### Development Environment Enhancements

- **C++17 Standard**: Full migration to modern C++ standards for improved performance
- **Compile-Time Optimizations**: Enhanced build speed and runtime performance
- **Professional Audio Workflow**: ASIO integration for serious music production
- **Improved Developer Experience**: Cleaner code, better documentation, reduced warnings

### Comprehensive Preset Collection - "New Factory Presets"

A professionally crafted collection of **25 patches** demonstrating the expanded waveform capabilities and advanced synthesis techniques:

#### ??? **Advanced Waves** (8 patches)

Showcase patches featuring the new waveforms:

- **NW Pulse Master** - Variable pulse width combinations (Pulse 25% + 10%)
- **NW Hybrid Morph** - Saw+Square hybrid waveform with dynamic filtering
- **NW Triangle Square** - Tri+Square blend with envelope modulation
- **NW Skewed Dream** - Asymmetric sine wave with textural modulation
- **NW Folded Reality** - Wavefold distortion effects with harmonic richness
- **NW Super Stack** - Ultra-rich supersaw with unison detuning
- **NW Chirping Bird** - Frequency sweep effects for cinematic sounds
- **NW Folded Chirp** - Complex combination of folded sine and chirp patterns

#### ?? **Lead** (4 patches)

Cutting leads with new waveform combinations:

- **NW Crystal Lead** - Skewed sine with super saw layering for crystalline textures
- **NW Acid Pluck** - Saw+Square hybrid with narrow pulse accents
- **NW Sync Lead** - Triangle+Square morph with cross-modulation
- **NW Harmonic Lead** - Folded sine harmonics with LFO modulation

#### ?? **Bass** (5 patches)

Powerful bass sounds utilizing new pulse and hybrid waves:

- **NW Deep Pulse** - 25% pulse wave with sub oscillator reinforcement
- **NW Growling Beast** - Saw+Square with aggressive distortion and filtering
- **NW Modern Sub** - Triangle+Square hybrid with enhanced low-end
- **NW Morphing Bass** - Sample & hold modulation with cross-mod textures
- **NW Vintage Formant** - Triangle+Square through formant filtering

#### ?? **Pad** (3 patches)

Atmospheric textures with textured waveforms:

- **NW Ethereal Pad** - Folded sine with super saw for ethereal atmospheres
- **NW Warm Strings** - Skewed sine warmth with triangle wave blending
- **NW Shimmer Pad** - Folded sine and chirp combination for shimmering textures

#### ?? **Arp** (3 patches)

Rhythmic patterns showcasing new waveforms:

- **NW Pulse Cascade** - 25% pulse wave arpeggiator patterns
- **NW Morphing Sequence** - Cross-modulation with step sequencer programming
- **NW Chord Cascade** - Chirp sweeps with saw+square chord progressions

#### ?? **SFX** (2 patches)

Experimental sound design patches:

- **NW Transformer** - Extreme modulation with all new waveforms
- **NW Digital Chaos** - Sample & hold with stutter effects and digital textures

**Key Features:**

- All 21 waveforms demonstrated across musical contexts
- Advanced modulation techniques including cross-modulation, complex LFO routing, and step sequencing
- Professional effect chains with formant filtering, stutter, delay, and reverb
- Educational value for learning synthesis techniques
- Ready-to-use patches for music production across genres

These comprehensive enhancements significantly improve the user experience, add professional audio capabilities, modernize the synthesis engine with C++17 features, expand the sonic palette with 8 new waveforms for creative sound design, provide a complete preset collection demonstrating the expanded capabilities, and enhance the development workflow while maintaining backward compatibility and cross-platform support.