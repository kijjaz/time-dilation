# Time Dilation DAW (v4.0)

<p align="center">
  <img src="web/logo_banner.png" alt="Time Dilation DAW Logo" width="650"/>
</p>

<p align="center">
  <b>A Relativistic Modular Audio Workstation & Spacetime Synthesis Engine</b><br>
  Built with C++20, JUCE 7, and General Relativity DSP Math for macOS Standalone & Audio Plugins.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-8b5cf6.svg" alt="C++20"/>
  <img src="https://img.shields.io/badge/JUCE-7.0.12-06b6d4.svg" alt="JUCE 7"/>
  <img src="https://img.shields.io/badge/Platform-macOS-f59e0b.svg" alt="macOS"/>
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License MIT"/>
</p>

---

## 🌌 Overview

**Time Dilation DAW** is a modular, dataflow-driven Digital Audio Workstation and experimental synthesis workstation that treats **time as a continuous, warps-enabled signal**. Instead of relying on a rigid linear timeline, every audio oscillator, filter, sampler, and sequencer is driven by a dynamic coordinate time clock $\tau(t)$. 

By modulating the local flow of time using relativistic gravitational equations, Doppler shift calculations, and retrograde time reversal, the workstation unlocks unique acoustic phenomena—including continuous pitch bending, gravitational time stasis, micro-step quantum stuttering, and future causality lookahead.

---

## ✨ Key Features

### ⏱️ Relativistic Time Engine
- **`time.warp~`**: Continuous time dilation engine powered by low-frequency gamma factor modulation ($\gamma \in [0.01, 10.0]$).
- **`time.retro~`**: Instantaneous retrograde time reversal engine (negates time flow rate $\dot{\tau} = -1.0$).
- **`time.stasis~`**: Gravitational time freeze mechanism holding coordinate time fixed while retaining audio buffer state.
- **`time.singularity~`**: Event horizon gravitational redshift simulation with non-linear warping.
- **`time.quantize~`**: Metric time grid quantizer creating micro-step rhythmic stutter and granular slicing.
- **`time.future~`**: Causality horizon lead-time buffer permitting up to 10 seconds of lookahead into future time.

### 🎛️ Pure Data-Style Visual Canvas & Fast Object Creator
- **Fast Object Creator (`Cmd-1` / `N` / Double-Click)**: Type object names directly onto empty canvas space with real-time autocompletion, category filtering, and parameter parsing.
- **Live Autocomplete Hub**: Floating selection card displaying class descriptions, inputs/outputs, and category tags.
- **Non-blocking Cancellation & Validation**: Pressing `ESC` or clicking away on empty text cleans up draft boxes without littering canvas space. Invalid inputs render a neon-red warning outline (`! NO SUCH OBJECT`).
- **Carbon & Gold Aesthetic**: Dark Slate cards (`#0d1322`), micro-dot background matrix (`#1a94a3b8`), Cyber Cyan wire routing (`#06b6d4`), and glowing Relativistic Gold selection halos (`#f59e0b`).

### 📡 Wireless Signal Tapping (`tap()`)
- Wireless cross-node parameter tapping: Write `tap("nodeLabel.paramKey")` or `tap(id)` directly inside any C++ DSP expression script or control field.
- TOP-DOWN Inspector integration: Click **`[TAP]`** on any slider to instantly copy `tap('nodeLabel.paramKey')` to the system clipboard for immediate insertion into active text fields.

### 🔄 Absolute Feedback Loop Stability
- Real-time graph topological sorting automatically detects feedback loops between nodes.
- When an audio feedback loop is formed, destination ports automatically sample from a 1-block delay buffer (`previousBlockBuffer`), ensuring 100% real-time DSP stability without audio thread lockups or buffer starvation.

### 🎙️ Audio Debug Recording to `/tmp`
- Built-in WAV recording engine on `[out~]` nodes: Click **`[EXEC] Start WAV Record`** in the TOP-DOWN Inspector to stream 16-bit 44.1 kHz stereo audio directly to `/tmp/out_node_<id>_<timestamp>.wav`.
- Includes a visual CRT scope indicator with glowing red `REC (/tmp)` status during active recording.

---

## ⌨️ Keyboard Shortcuts & Canvas Controls

| Shortcut | Action |
| :--- | :--- |
| **`Cmd-1`** / **`N`** / **Double-Click** | Spawn Fast Object Creator Box on Canvas |
| **`Up`** / **`Down`** Arrow | Cycle through object autocompletion suggestions |
| **`Tab`** / **`Enter`** | Confirm selected autocomplete object / commit text |
| **`ESC`** | Cancel inline object creation without adding node |
| **`Cmd-Z`** | Undo previous graph edit / connection |
| **`Cmd-Shift-Z`** / **`Cmd-Y`** | Redo previously undone action |
| **`Cmd-C`** | Copy selected node(s) and connections to clipboard |
| **`Cmd-V`** | Paste node(s) with spatial offset |
| **`Cmd-D`** | Duplicate selected node(s) into fresh independent instances |
| **`Delete`** / **`Backspace`** | Delete selected node(s) or wire connection |
| **`Spacebar`** | Toggle Master Audio Processing Power ON/OFF |

---

## 📑 Complete Object Reference

### ⏱️ Relativistic Time Engines (`TIME`)
- `time.warp~` — Dilated Coordinate Time Generator (LFO Dilation)
- `time.retro~` — Retrograde Time Reverser (-1.0x Time Flow)
- `time.stasis~` — Gravitational Time Stasis Freeze Engine
- `time.singularity~` — Event Horizon Gravitational Redshift Warp
- `time.quantize~` — Metric Grid Time Quantizer (Micro-Step Stutter)
- `time.transport` — Master Transport Clock Hub & BPM Master
- `time.metro~` — Dilated Metronome Pulse Spiker
- `time.scope` — Relativistic Time & Telemetry Visualizer Monitor
- `time.future~` — Future Lookahead Causality Offset Engine

### 🔊 Audio & DSP Generators (`DSP`)
- `osc~` — Polyphonic Sine/Saw/Square Varispeed Oscillator
- `phasor~` — Linear Ramp Audio Phase Generator
- `sampler~` — Varispeed Audio Sampler & Loop Player
- `filter~` — State-Variable Filter (LP/HP/BP/Notch)
- `svfilter~` — Vadim Zavalishin TPT State-Variable Filter
- `delay~` — Feedback Delay Line (Hermite 4-Point Resampling)
- `drive~` — Non-Linear Harmonic Tube Overdrive Distortion
- `reverb~` — Stereo Algorithmic Reverb Unit
- `crush~` — Quantum Bitcrusher & Sample Reducer
- `adsr~` — Envelope Generator (Attack, Decay, Sustain, Release)
- `env~` — Audio Envelope Follower & Peak Detector
- `gain~` — Audio Signal Scaler & Varispeed Time Warper
- `out~` — Master Output Fader & Live Oscilloscope CRT
- `dac~` — Audio Master Hardware Output DAC
- `fbdrum~` — Polyphonic Future Bass Drum Synthesizer
- `tabosc4~` — 4-Point Hermite Interpolated Wavetable Oscillator

### 🎼 Sequencers & Generative Engines (`SEQ`)
- `seq` — Multi-Step Pattern Sequencer
- `drumseq` — Multi-Track 16-Step Future Bass Drum Sequencer
- `euclid` — Euclidean Rhythm Pattern Generator
- `markov` — Stochastic Markov Chain Melodic Generator
- `tidal` / `tidal~` — Tidal Live-Coding Mini-Notation Sequencer
- `timeline` — Multi-Track Timeline Clip Sequencer

### 🎛️ Control Interactors & Triggers (`CTRL`)
- `number` / `num` — Control Number Box (Click & Drag Value)
- `msg` / `message` / `v` — Message Box & Value Storage Node
- `bang` / `b` — Control Trigger Pulse Spiker
- `bang~` / `b~` — Audio-Rate Impulse Spike Spiker
- `counter` — Smart Value Counter (Low, High, Step, Carry)
- `note` — MIDI Note Pitch Generator
- `tap` / `tap~` — Wireless Control & Audio Signal Tap Listener

### 🧮 Math & Expressions (`MATH`)
- `expr` — Control Expression Evaluator ($v1, $v2, tap('id'))
- `expr~` — Audio Expression Evaluator ($v1, $v2, $t)
- `fexpr~` — Filter Recurrent Expression ($y1[-1])
- `+` / `*` — Audio & Control Signal Adder / Multiplier
- `mtof` / `ftom` — MIDI Pitch $\leftrightarrow$ Frequency Hz Converter

---

## 📐 Mathematical Formulation

Coordinate time $\tau(t)$ under relativistic dilation is governed by:

$$\tau(t) = \int_0^t \gamma(t') \, dt'$$

where $\gamma(t')$ represents the instantaneous Lorentz factor:

$$\gamma(t') = \frac{1}{\sqrt{1 - \frac{v(t')^2}{c^2}}}$$

Hermite 4-point cubic polynomial interpolation is applied to ensure artifact-free varispeed audio playback during rapid time acceleration:

$$y(x) = a_0 + a_1 x + a_2 x^2 + a_3 x^3$$

---

## 🛠️ Building from Source

### Prerequisites
- **macOS**: 12.0 or newer
- **Compiler**: Clang / AppleClang with C++20 support
- **Build System**: CMake 3.22+ & Ninja or Make

### Quick Build Instructions

```bash
# 1. Clone the repository
git clone https://github.com/kijjaz/time-dilation.git
cd time-dilation

# 2. Configure build environment with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Compile Standalone Workstation & Plugins
cmake --build build --config Release -j8

# 4. Launch Application
open "build/TimeDilationDAW_App_artefacts/Time Dilation DAW.app"
```

---

## 📄 License
Released under the **MIT License**. Created for research in relativistic spacetime audio synthesis.
