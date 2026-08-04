# Time Dilation DAW (v0.0.1)

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

## 📐 Mathematical Formulation & Theoretical Documentation

The theoretical physics and digital signal processing mathematics of **Time Dilation DAW (v0.0.1)** are formally documented in the [`math/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math) suite:

1. **[`01_relativistic_time_dilation.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/01_relativistic_time_dilation.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/01_relativistic_time_dilation.pdf)) — Lorentz Velocity Dilation, Schwarzschild Gravitational Redshift Event Horizons, Universal Node `timeIn` Propagation, & Coordinate Time Integration.
2. **[`02_dsp_interpolation_and_resampling.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/02_dsp_interpolation_and_resampling.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/02_dsp_interpolation_and_resampling.pdf)) — Hermite 4-Point 3rd-Order Cubic Spline Interpolation Matrix \& Varispeed Sub-Sample Resampling.
3. **[`03_pitch_and_tuning_conversions.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/03_pitch_and_tuning_conversions.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/03_pitch_and_tuning_conversions.pdf)) — Equal Temperament Logarithmic `[mtof]` / `[ftom]` Conversions, Microtonal Cent Ratios, \& Acoustic Doppler Shifts.
4. **[`04_state_variable_filter_dsp.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/04_state_variable_filter_dsp.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/04_state_variable_filter_dsp.pdf)) — Vadim Zavalishin Topology-Preserving Transform (TPT) Zero-Delay State Variable Filter \& Moog 24dB Cascade Math.
5. **[`05_feedback_loop_stability.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/05_feedback_loop_stability.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/05_feedback_loop_stability.pdf)) — Directed Graph DFS Cycle Detection, Topological Sorting, \& 1-Block History Memory Allocation Proof.
6. **[`06_nonlinear_saturation_dsp.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/06_nonlinear_saturation_dsp.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/06_nonlinear_saturation_dsp.pdf)) — Non-Linear Hyperbolic Tangent ($\tanh$) Tube Saturation, Asymmetric Grid Bias, \& Cubic Soft-Clipping.
7. **[`07_envelope_generators_adsr.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/07_envelope_generators_adsr.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/07_envelope_generators_adsr.pdf)) — Piecewise Exponential ADSR Envelope Generator State Transitions \& Peak/RMS Envelope Followers.
8. **[`08_polyblep_anti_aliasing.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/08_polyblep_anti_aliasing.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/08_polyblep_anti_aliasing.pdf)) — 2-Point PolyBLEP (Polynomial Band-Limited Step) Residual Anti-Aliasing Correction.
9. **[`09_granular_synthesis_math.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/09_granular_synthesis_math.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/09_granular_synthesis_math.pdf)) — Gabor Granular Synthesis, Hanning/Gaussian Window Envelopes, Poisson Grain Distributions, \& Overlap-Add (OLA).
10. **[`10_reverb_fdn_matrices.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/10_reverb_fdn_matrices.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/10_reverb_fdn_matrices.pdf)) — Multi-Channel Feedback Delay Network (FDN) Reverb Unitary Householder Matrices \& Prime Delay Selection.
11. **[`11_spectrometer_fft_log_analysis.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/11_spectrometer_fft_log_analysis.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/11_spectrometer_fft_log_analysis.pdf)) — 32-Band Logarithmic Frequency Spectrum Estimation \& 2D Phase Space Parametric State Trajectories.
12. **[`12_arc_peak_limiter_math.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/12_arc_peak_limiter_math.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/12_arc_peak_limiter_math.pdf)) — Auto Release Control (ARC) Dynamic Peak Limiting at -1.5 dBFS \& Adaptive Gain Smoothing.
13. **[`13_lorentz_time_signal_composition.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/13_lorentz_time_signal_composition.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/13_lorentz_time_signal_composition.pdf)) — Relativistic Lorentz Velocity Composition Law, Dynamic Telemetry Auto-Scaling, \& Dual-Time Phase Space Mapping.
14. **[`future_lookahead_causality.tex`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/future_lookahead_causality.tex)** ([PDF](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math/future_lookahead_causality.pdf)) — Relativistic Future Lookahead, Global Causality Horizon Offset Engine, \& Smoothed Time Dilation.

---

## 🛠️ Building from Source

For detailed, step-by-step compilation instructions across **macOS**, **Linux**, and **Windows**, please refer to the dedicated **[`INSTALL.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/INSTALL.md)** guide.

### Quick Build Instructions

```bash
# 1. Clone the repository
git clone https://github.com/kijjaz/time-dilation-daw.git
cd "20260801 Time Dilation DAW"

# 2. Configure build environment with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja

# 3. Compile Standalone Workstation & Plugins
cmake --build build --config Release -j$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)

# 4. Launch Application (macOS example)
open "build/TimeDilationDAW_App_artefacts/Release/Standalone/Time Dilation DAW.app"
```

---

## 🙏 Open-Source Credits & Attributions

**Time Dilation DAW** is built on top of and inspired by these incredible open-source projects:

| Library / Project | Creator / Maintainer | License | Primary Role & Link |
| :--- | :--- | :--- | :--- |
| **JUCE 7** | RAW Material Software / Pace Anti-Piracy | GPLv3 / Commercial | C++ cross-platform audio app framework, VST3/AU wrappers, audio device management, and vector graphics. ([juce.com](https://juce.com)) |
| **Tracktion Engine** | Tracktion Software | GPLv3 / Commercial | High-level DAW engine powering the multitrack arrangement timeline ([timeline]) and transport scheduling. ([tracktion.com](https://www.tracktion.com)) |
| **ExprTk** | Arash Partow | Arash Partow Open-Source License | High-performance C++ mathematical expression parsing engine evaluating real-time DSP math expressions (`$v1`, `$v2`, `$t`, `tap()`). ([partow.net](https://www.partow.net/programming/exprtk/)) |
| **Pure Data (Pd) Principles** | Miller Puckette | BSD License | Mathematical formulations and object semantics (`osc~`, `filter~`, `delay~`, `env~`, `bang`) adapted into native C++. ([puredata.info](https://puredata.info)) |

---

## 📄 License
Released under the **MIT License**. Created for research in relativistic spacetime audio synthesis.
