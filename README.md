# Helm2025

![Helm2025 Screenshot](images/ScreenShot.png)

---

## Table of Contents / Sommaire

### English
- [About This Project](#about-this-project)
- [Key Features](#key-features)
- [Major Improvements – October 2025](#major-improvements--october-2025)
- [Build Instructions](#build-instructions)
- [License & Credits](#license--credits)

### Français
- [À propos du projet](#à-propos-du-projet)
- [Fonctionnalités principales](#fonctionnalités-principales)
- [Améliorations majeures – Octobre 2025](#améliorations-majeures--octobre-2025)
- [Instructions de compilation](#instructions-de-compilation)
- [Licence & crédits](#licence--crédits)

---

## English

### About This Project

Helm2025 is a significantly modernized and independent fork of Matt Tytel's original [Helm synthesizer](https://github.com/mtytel/helm), started from the [bepzi/helm fork](https://github.com/bepzi/helm). It features major performance, stability, and usability improvements, as well as new synthesis and UI features.

### Key Features

- Up to **32-voice polyphony**
- **Step sequencer up to 64 steps** (editing, modulation, audio engine, and patch save/load)
- Interactive, real-time visual interface
- Powerful modulation system with live feedback
- Dual oscillators with cross-modulation and up to 15 unison voices each
- Sub oscillator with shuffle waveshaping
- Oscillator feedback and saturation
- 21 waveforms, including advanced and hybrid shapes
- Blendable 12/24dB low/band/high pass and shelf filters
- 2 monophonic and 1 polyphonic LFO
- Many modulation sources, including polyphonic aftertouch
- Simple arpeggiator
- Effects: formant filter, stutter, delay, distortion, reverb

---


### Major Improvements – October 2025

- **Full C++20 migration:** The entire codebase now requires and uses C++20, with strict enforcement in CMake for all platforms (including MSVC/Visual Studio). All modules and plugins are built with `/Zc:__cplusplus` for correct standard detection under MSVC.
- **Project-wide adoption of `std::span` and `std::array`:** All buffer and array accesses in the DSP, UI, and plugin code use `std::span` or `std::array` for type safety, bounds checking, and modern C++ clarity. Raw pointers for audio/data buffers have been eliminated in favor of spans.
- **Robust CMake & MSVC support:** The build system is fully modernized : CMake enforces C++20, handles MSVC quirks, and ensures all targets are built with the correct standard and options. The project builds out-of-the-box on Windows, macOS, and Linux with all major compilers.
- **Suppression of MSVC enum/float warnings:** All code comparing enums and floats has been refactored to use explicit casts, eliminating C5055 warnings and ensuring robust, portable code.
- **User preferences persistence:** All UI and audio settings (window size, UI zoom, audio/MIDI device, sample rate, buffer size, audio ports, etc.) are now saved and restored robustly. Preferences are never overwritten by defaults at startup; only the actually applied values are saved. Immediate save on any change. Works for all device types, including ASIO on Windows. Preferences are stored in a JSON file in the user’s application data directory.
- **Audio configuration restoration & warning:** At startup, the app restores the audio device, type, sample rate, buffer size, and ports as saved in the JSON preferences. If the applied audio configuration differs from the saved preferences (e.g., device unavailable, ASIO fallback), a warning is shown to the user (in English) before any overwrite.
- **Uninstall option: remove preferences:** The Windows installer (Inno Setup) now asks, during uninstall, if you want to delete your configuration/preferences (JSON file and folder). The message is shown in English or French, matching the installer language. Preferences are only deleted if the user confirms.
- **Multilingual user experience:** All new dialogs and uninstall options are available in both English and French. The installer and all user-facing messages adapt to the selected language.
- **Poly LFO S&H/S&G static visualization:** The Poly LFO S&H and S&G modes now always display a static, representative random curve in the UI, never blank or flat, and never crash.

---

## 🚀 Modern C++20: std::span migration (Octobre 2025)

### English
- All internal buffer and array accesses have been modernized to use `std::span` or `std::array` for safety, clarity, and performance.
- Raw pointers for audio/data buffers are replaced by `std::span` in all major modules (mopo::Memory, OpenGLEnvelope, HelmLfo, ComputeCache, AlignedAudioBuffer, etc.).
- This ensures bounds-checked, type-safe, and more maintainable code throughout the DSP and UI layers.
- All new code should use `std::span` or `std::array` for buffer access and manipulation.

### Français
- Tous les accès aux buffers et tableaux internes ont été modernisés avec `std::span` ou `std::array` pour plus de sécurité, de clarté et de performance.
- Les pointeurs bruts pour les buffers audio/données sont remplacés par `std::span` dans tous les modules principaux (mopo::Memory, OpenGLEnvelope, HelmLfo, ComputeCache, AlignedAudioBuffer, etc.).
- Cela garantit un code vérifié, sûr et maintenable dans toute la chaîne DSP et interface.
- Toute nouvelle contribution doit utiliser `std::span` ou `std::array` pour la gestion des buffers.

---

### Build Instructions

#### Prerequisites
- **Git** (for cloning the repository and submodules)
- **CMake** (version 3.15 or higher)
- **C++20 compiler** (e.g., Visual Studio 2022, GCC 10+, Clang 12+)
- **PowerShell** (for Windows users running the build script)
- **(Optional) ASIO SDK** (for professional low-latency audio on Windows)

#### Quick Start
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

---

### License & Credits

Helm2025 is free software, released under the GNU GPLv3. It is based on the original Helm synthesizer by Matt Tytel, with major modifications and new features by Hans45 and contributors. See the source code and [COPYING](COPYING) for details.

For more information, screenshots, and updates, visit the project repository.

---

## Français

### À propos du projet

Helm2025 est un fork moderne et indépendant du synthétiseur Helm original de Matt Tytel, basé sur le fork [bepzi/helm](https://github.com/bepzi/helm). Il apporte de nombreuses améliorations de performance, de stabilité, d’ergonomie et de nouvelles fonctionnalités de synthèse et d’interface.

### Fonctionnalités principales

- Jusqu’à **32 voix de polyphonie**
- **Séquenceur jusqu’à 64 pas** (édition, modulation, moteur audio, sauvegarde de patch)
- Interface visuelle interactive en temps réel
- Système de modulation puissant avec retour visuel
- Deux oscillateurs avec cross-modulation et jusqu’à 15 voix d’unisson chacun
- Oscillateur sub avec waveshaping shuffle
- Feedback et saturation d’oscillateur
- 21 formes d’onde, y compris des formes avancées et hybrides
- Filtres passe-bas, passe-haut, passe-bande et shelf 12/24dB
- 2 LFO monophoniques et 1 polyphonique
- Nombreuses sources de modulation, aftertouch polyphonique inclus
- Arpégiateur simple
- Effets : filtre formant, stutter, delay, distorsion, réverbe

---


### Améliorations majeures – Octobre 2025

- **Migration complète vers C++20 :** Toute la base de code requiert et utilise C++20, avec enforcement strict dans CMake pour toutes les plateformes (y compris MSVC/Visual Studio). Tous les modules et plugins sont compilés avec `/Zc:__cplusplus` pour une détection correcte du standard sous MSVC.
- **Adoption généralisée de `std::span` et `std::array` :** Tous les accès aux buffers et tableaux dans le DSP, l’UI et les plugins utilisent `std::span` ou `std::array` pour la sécurité de type, la vérification des bornes et la clarté moderne. Les pointeurs bruts pour les buffers audio/données ont été éliminés au profit des spans.
- **CMake & MSVC robustes :** Le système de build est entièrement modernisé : CMake force C++20, gère les spécificités MSVC, et garantit que toutes les cibles sont compilées avec le bon standard et les bonnes options. Le projet se compile sans modification sur Windows, macOS et Linux avec tous les compilateurs majeurs.
- **Suppression des warnings enum/float MSVC :** Tout le code comparant des enums et des flottants a été refactoré pour utiliser des casts explicites, supprimant les warnings C5055 et assurant un code robuste et portable.
- **Persistance robuste des préférences utilisateur :** Toutes les préférences d’interface et audio (taille de fenêtre, zoom, périphérique audio/MIDI, fréquence d’échantillonnage, buffer, ports, etc.) sont désormais sauvegardées et restaurées de façon robuste. Les préférences ne sont jamais écrasées par défaut au démarrage ; seules les valeurs effectivement appliquées sont enregistrées. Sauvegarde immédiate à chaque changement. Fonctionne pour tous les types de périphériques, y compris ASIO sous Windows. Stockage dans un fichier JSON dans le dossier utilisateur.
- **Restauration de la configuration audio & avertissement :** Au démarrage, l’application restaure le périphérique audio, le type, la fréquence, le buffer et les ports selon les préférences JSON. Si la configuration appliquée diffère (périphérique indisponible, fallback ASIO, etc.), un avertissement s’affiche (en anglais) avant tout écrasement.
- **Option de désinstallation : suppression des préférences :** L’installateur Windows (Inno Setup) propose, lors de la désinstallation, de supprimer la configuration/préférences (fichier et dossier JSON). Le message s’affiche en anglais ou en français selon la langue de l’installateur. Suppression uniquement si l’utilisateur confirme.
- **Expérience utilisateur multilingue :** Tous les nouveaux dialogues et options de désinstallation sont disponibles en anglais et en français. L’installateur et tous les messages utilisateur s’adaptent à la langue choisie.
- **Visualisation statique Poly LFO S&H/S&G :** Les modes Poly LFO S&H et S&G affichent désormais toujours une courbe aléatoire représentative statique dans l’interface, jamais vide ou plate, et sans crash.

---

## 🚀 Modern C++20: std::span migration (Octobre 2025)

### English
- All internal buffer and array accesses have been modernized to use `std::span` or `std::array` for safety, clarity, and performance.
- Raw pointers for audio/data buffers are replaced by `std::span` in all major modules (mopo::Memory, OpenGLEnvelope, HelmLfo, ComputeCache, AlignedAudioBuffer, etc.).
- This ensures bounds-checked, type-safe, and more maintainable code throughout the DSP and UI layers.
- All new code should use `std::span` or `std::array` for buffer access and manipulation.

### Français
- Tous les accès aux buffers et tableaux internes ont été modernisés avec `std::span` ou `std::array` pour plus de sécurité, de clarté et de performance.
- Les pointeurs bruts pour les buffers audio/données sont remplacés par `std::span` dans tous les modules principaux (mopo::Memory, OpenGLEnvelope, HelmLfo, ComputeCache, AlignedAudioBuffer, etc.).
- Cela garantit un code vérifié, sûr et maintenable dans toute la chaîne DSP et interface.
- Toute nouvelle contribution doit utiliser `std::span` ou `std::array` pour la gestion des buffers.

---

### Instructions de compilation

#### Prérequis
- **Git** (pour cloner le dépôt et les sous-modules)
- **CMake** (version 3.15 ou supérieure)
- **Compilateur C++20** (Visual Studio 2022, GCC 10+, Clang 12+)
- **PowerShell** (pour le script Windows)
- **(Optionnel) SDK ASIO** (pour l’audio pro sous Windows)

#### Démarrage rapide
1. Ouvrez PowerShell dans le dossier du projet.
2. Lancez :
   ```powershell
   .\build.ps1
   ```
   Ce script initialise les dépendances, lance CMake, compile tout et crée l’installateur.

#### macOS & Linux (ou Windows manuel)
1. Initialisez les dépendances :
   ```bash
   git submodule update --init --recursive
   ```
2. Configurez avec CMake :
   ```bash
   cmake -S . -B build
   ```
3. Compilez :
   ```bash
   cmake --build build
   ```

---

### Licence & crédits

Helm2025 est un logiciel libre sous licence GNU GPLv3, basé sur Helm de Matt Tytel, avec de nombreuses modifications et nouveautés par Hans45 et contributeurs. Voir le code source et [COPYING](COPYING) pour plus de détails.

Pour plus d’informations, de captures d’écran et de mises à jour, consultez le dépôt du projet.
