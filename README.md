

# Helm2025

![Helm2025 Screenshot](images/ScreenShot.png)

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

- **Robust user preferences persistence:** All UI and audio settings (window size, UI zoom, audio/MIDI device, sample rate, buffer size, audio ports, etc.) are now saved and restored independently and robustly. Preferences are never overwritten by defaults at startup; only the actually applied values are saved. Immediate save on any change (window resize, device/port/sample rate/buffer change, etc.). The system works for all device types, including ASIO on Windows. Preferences are stored in a JSON file in the user’s application data directory.
- **Audio configuration restoration & warning:** At startup, the app restores the audio device, type, sample rate, buffer size, and ports as saved in the JSON preferences. If the applied audio configuration differs from the saved preferences (e.g., device unavailable, ASIO fallback), a warning is shown to the user (in English) before any overwrite. This ensures the user is always aware if their configuration could not be fully restored.
- **Uninstall option: remove preferences:** The Windows installer (Inno Setup) now asks, during uninstall, if you want to delete your configuration/preferences (JSON file and folder). The message is shown in English or French, matching the installer language. Preferences are only deleted if the user confirms.
- **Multilingual user experience:** All new dialogs and uninstall options are available in both English and French. The installer and all user-facing messages adapt to the selected language.
- **Poly LFO S&H/S&G static visualization:** The Poly LFO S&H and S&G modes now always display a static, representative random curve in the UI, never blank or flat, and never crash.

---

### Build Instructions

#### Prerequisites
- **Git** (for cloning the repository and submodules)
- **CMake** (version 3.15 or higher)
- **C++17 compiler** (e.g., Visual Studio 2019/2022, GCC 7+, Clang 6+)
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

- **Persistance robuste des préférences utilisateur :** Toutes les préférences d’interface et audio (taille de fenêtre, zoom, périphérique audio/MIDI, fréquence d’échantillonnage, buffer, ports, etc.) sont désormais sauvegardées et restaurées de façon indépendante et robuste. Les préférences ne sont jamais écrasées par défaut au démarrage ; seules les valeurs effectivement appliquées sont enregistrées. Sauvegarde immédiate à chaque changement (redimensionnement, changement de périphérique/port/fréquence/buffer, etc.). Fonctionne pour tous les types de périphériques, y compris ASIO sous Windows. Stockage dans un fichier JSON dans le dossier de l’utilisateur.
- **Restauration de la configuration audio & avertissement :** Au démarrage, l’application restaure le périphérique audio, le type, la fréquence, le buffer et les ports selon les préférences JSON. Si la configuration appliquée diffère (périphérique indisponible, fallback ASIO, etc.), un avertissement s’affiche (en anglais) avant tout écrasement. L’utilisateur est ainsi toujours informé si sa configuration n’a pas pu être restaurée.
- **Option de désinstallation : suppression des préférences :** L’installateur Windows (Inno Setup) propose, lors de la désinstallation, de supprimer la configuration/préférences (fichier et dossier JSON). Le message s’affiche en anglais ou en français selon la langue de l’installateur. Suppression uniquement si l’utilisateur confirme.
- **Expérience utilisateur multilingue :** Tous les nouveaux dialogues et options de désinstallation sont disponibles en anglais et en français. L’installateur et tous les messages utilisateur s’adaptent à la langue choisie.
- **Visualisation statique Poly LFO S&H/S&G :** Les modes Poly LFO S&H et S&G affichent désormais toujours une courbe aléatoire représentative statique dans l’interface, jamais vide ou plate, et sans crash.

---

### Instructions de compilation

#### Prérequis
- **Git** (pour cloner le dépôt et les sous-modules)
- **CMake** (version 3.15 ou supérieure)
- **Compilateur C++17** (Visual Studio 2019/2022, GCC 7+, Clang 6+)
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
