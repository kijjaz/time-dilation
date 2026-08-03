# Time Dilation DAW
> **Producer**: Kijjaz  
> **Concept**: Cross-Platform Relativistic Digital Audio Workstation  
> **Platform Stack**: Native C++20 / JUCE 7 & Tracktion Engine (Desktop Application & VST3 / AU Audio Plugin)  

---

## Overview

**Time Dilation DAW** is a native cross-platform digital audio workstation and audio plugin engine designed for non-linear temporal music production. Inspired by general relativity, dynamic physics, and avant-garde sound design, Time Dilation breaks free from fixed-tempo global clocks by allowing individual tracks, audio clips, and effects to operate under independent **Time Dilation Fields ($\gamma$)**.

---

## Project Status & Archival Record

### 1. Archived v1 GUI Design (`archive/v1_studio_gui/`)
- The initial v1 UI design components (`ArrangementViewComponent`, `MixerConsoleComponent`, `DeviceBrowserComponent`, `RelativisticLabComponent`, `TimelineSequencerComponent`, `SpacetimeVisualizerComponent`) have been safely archived in `archive/v1_studio_gui/` to make way for the next fresh UI design direction.

### 2. Preserved Core DSP Architecture & Assets (`Source/`)
All underlying DSP engines, high-performance C++ algorithms, open-source fonts, and build pipelines remain active and ready:

- **C++ DSP Core (`Source/dsp/`)**:
  - `TimeDilationEngine`: Relativistic multi-track time clocks, accumulated Proper Time counters ($\tau$), nested track hierarchy trees, and transport engine.
  - `HermiteResampler`: 3rd-order Hermite cubic resampler for continuous varispeed pitch and rate bending.
  - `DopplerDelay`: Dynamic fractional delay line modeling sound sources moving through curved space.
  - `GammaScriptEngine`: High-performance recursive descent expression parser and evaluator for algorithmic time-dilation scripts.
  - `GammaTapMatrix`: Dynamic time stream tap matrix for cross-track modulation and amplitude sidechaining.
  - `PolySynthVoice`: Polyphonic FM/subtractive synthesis engine.
  - `GammaLFO`: Relativistic LFO modulators (*BlackHoleExp*, *TachyonPulse*, *LorenzChaos*, *Sine*, *Triangle*).

- **Open-Source Fonts (`Source/assets/fonts/`)**:
  - `SmoochSans.ttf` (SIL Open Font License): Stylized sci-fi display typography.
  - `NotoSans.ttf` (SIL Open Font License): Multi-language fallback font supporting international Unicode glyphs (Thai, CJK, Latin, Cyrillic, Greek).

- **Audio Infrastructure & Build System**:
  - `AudioSettingsComponent`: Audio Interface, Sample Rate (44.1 kHz - 192 kHz), and Buffer Size manager.
  - `FontManager`: Dynamic Typeface loader and font selector.
  - `CMakeLists.txt`: CMake build system for Standalone App, VST3 Plugin, and AudioUnit (AU) Plugin with Tracktion Engine.

---

## Technical Specifications & Build Targets

- **Language**: C++20
- **Framework**: JUCE 7 / JUCE 8 & Tracktion Engine 3 (CMake build system)
- **Targets**: Standalone DAW Desktop Application, VST3 Plugin, AudioUnit (AU) Plugin
- **Platforms**: macOS (Apple Silicon / Intel), Windows (MSVC / MinGW), Linux
