# Time Dilation DAW Workstation (Version 0.0.1)
> **Producer**: Kijjaz  
> **Status**: Active Production (Version 0.0.1)  
> **Platform Stack**: Native C++20 / JUCE 7 & Tracktion Engine (Standalone Workstation & VST3 / AU Audio Plugin)  

---

## 1. Executive Summary

**Time Dilation DAW** is a state-of-the-art Relativistic Modular Workstation that unifies top-down visual patch composition with bottom-up authentic C++ / DSP math expression coding.

The system allows producers complete freedom to set and modulate every project parameter. It features a physics-based organic catenary cable engine, dynamic modulation inlets (`+ MOD INLET`), wireless global signal tapping (`tap("node.param")`), and relativistic time dilation inheritance ($\gamma$) where objects run on independent local coordinate time clocks.

---

## 2. Core Architecture & Features

### A. Dual Perspective Inspector (Top-Down & Bottom-Up)
- **Top-Down (Visual)**: Real-time linear sliders, custom parameter ranges, per-parameter math expression inputs, 1-click `[TAP]` buttons, and `+ MOD INLET` buttons for dynamically patching modulation inlets on any object canvas box.
- **Bottom-Up (Code Math)**: Instant loading of authentic C++ DSP formula scripts for every object (`[dac~]`, `[osc~]`, `[filter~]`, `[delay~]`, `[time.warp~]`, `[expr~]`, `[fexpr~]`, `[out~]`, `[env~]`).

### B. Relativistic Time Dilation Engine ($\gamma$)
- Objects pass time contexts along purple time cables.
- **`[time.warp~]`**: Modulates coordinate time dilation factor ($\gamma = 1.0 \to 10.0\times$).
- **`[time.retro~]`**: Temporal reversal engine for negative time progression ($\gamma = -1.0$).
- **`[time.quantize~]`**: Metric grid step quantizer.
- **`[time.metro~]`**: Relativistic metronome pulse generator.
- Objects automatically inherit parent time dilation ($\gamma$) down the signal hierarchy if unpatched.

### C. Wireless Global Signal Tapping Engine (`tap()`)
- Expressions in `[expr]`, `[expr~]`, `[fexpr~]`, or parameter math fields can wirelessly tap signals from ANY node in the patch graph without physical patch cables.
- **Syntax**:
  - `tap(node_id)`: Taps node by ID (e.g. `out = $v1 * tap(2);`).
  - `tap("node_label")`: Taps node by label (e.g. `out = $v1 * tap('osc~ 440 Hz');`).
  - `tap("node_label.param")`: Taps specific internal property (e.g. `tap('filter~.cutoff')`, `tap('osc~.frequency')`, `tap('time.warp~.gamma')`).
- **1-Click UX**:
  - `[TAP]` button on parameter sliders copies `tap("node.param")` to clipboard and inserts it into active expressions.
  - `TAP SIGNAL POINT` visual dropdown selector lists all nodes and parameters in the patch.

### D. Audio Signal Processing & VU Metering
- **`[gain~]`**: General audio signal scaler ($0.0 \to 2.0\times$ / $-\infty \text{ to } +6\text{ dB}$).
- **`[out~]`**: Master output fader with real-time dual RMS & Peak VU level meters rendered directly inside its canvas node box (Green $\to$ Yellow $\to$ Red Clip).
- **`[env~]`**: Envelope follower / peak detector with configurable attack/release (1 ms to 2000 ms).
- **`[tap]` / `[tap~]`**: Dedicated control and audio tap point objects.

### E. Audio Feedback Routing & Stability
- Feedback loops between audio objects read from a **1-Block History Delay Buffer** (`previousBlockBuffer`), guaranteeing absolute real-time DSP stability without buffer starvation, sample rate crashes, or feedback loops.

---

## 3. Design System & Aesthetics

- **Aesthetic**: Carbon & Gold + Slate Sci-Fi Workstation (Cubase-Inspired).
- **Color Palette**:
  - Dark Carbon Canvas Background: `#070a12` with micro dot-matrix grid (`#1a94a3b8`).
  - Dark Slate Container Panels: `#0d1322` / `#111827`.
  - Relativistic Time Accent: Deep Royal Violet (`#8b5cf6`).
  - Audio & DSP Accent: Cyber Cyan (`#06b6d4`).
  - Math & Control Accent: Relativistic Gold (`#f59e0b`).
  - Glowing Selection Halo: Cyan (`#38bdf8`).
- **Zero-Emoji Policy**: Clean technical tags (`[TAP]`, `[MOD]`, `[EXEC]`) and custom C++ JUCE vector graphics calls.

---

## 4. Workstation Selection & Undo State

- **Marquee Selection**: Shift-click-drag rubberband rectangle to select multiple node objects.
- **Hotkeys**: Command-A (Select All), Command-D (Duplicate), Command-C (Copy), Command-V (Paste), Command-Z (Undo), Command-Shift-Z (Redo), Delete / Backspace (Remove node/connection), N / Double-Click (Add Object Search Menu).
- **Undo State Persistence**: Full undo/redo undo history stored directly inside saved `.rel` project files.
