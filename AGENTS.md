# Time Dilation DAW Workstation — AI Agent Guidelines & Project Index

> **Workspace**: Time Dilation DAW (Version 0.0.1)  
> **Producer & Lead Architect**: Kijjaz  
> **Target Platform**: macOS (JUCE 7 / C++20 Standalone Workstation & Audio Plugin VST3/AU)  
> **Aesthetic System**: Carbon & Gold + Slate Sci-Fi Workstation (Cubase-Inspired)  

---

## 1. Executive Project Overview

**Time Dilation DAW** is a state-of-the-art Relativistic Modular Audio Workstation built with native C++20 and JUCE 7. It unifies top-down visual block diagram patching with bottom-up authentic C++ / DSP math expression coding.

Every node in the workspace operates on its own **local coordinate time clock** ($\gamma = d\tau / dt$), enabling dynamic continuous time warping (`time.warp~`), temporal reversal (`time.retro~`), gravitational stasis (`time.stasis~`), relativistic grid quantization (`time.quantize~`), and Lorentz velocity addition composition (`time.math~`).

---

## 2. Information Map (Where to Look for Everything)

When inspecting or working on this project, always refer to the following authoritative documents and directories:

| Document / Directory | Primary Purpose & Contents |
| :--- | :--- |
| [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) | **MANDATORY FOR TOKEN OPTIMIZATION**. Line-by-line mapping of all C++ classes, methods, and symbols in `Source/dsp/` and `Source/gui/`. Always inspect this file first to find exact target lines before viewing large C++ source files. |
| [`ABOUT.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/ABOUT.md) | Executive summary, core architecture, dual-perspective inspector, relativistic engine concept, audio metering, design system tokens, marquee selection, undo/redo state serialization. |
| [`HELP.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/HELP.md) | Comprehensive workstation user manual. Details the full symbol command table (`[osc~]`, `[table]`, `[sampler~]`, `[time.transport]`, etc.), message routing objects, relativistic time node suite, port compatibility rules, and multi-track arrangement timeline. |
| [`math/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math) | 28 formal mathematical papers and LaTeX sources detailing DSP algorithms, relativistic time physics (`01_relativistic_time_dilation.tex`), 1-block feedback stability (`05_feedback_loop_stability.tex`), Lorentz time composition (`13_lorentz_time_signal_composition.tex`), SVF filters, PolyBLEP oscillators, FDN reverbs, and lookahead causality (`future_lookahead_causality.tex`). |
| [`Source/dsp/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp) | Core C++ DSP processing engine: <br>• [`RelativisticNodeObjects.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/RelativisticNodeObjects.cpp): Node definitions, parameters (`getParameterDefs`), and bottom-up DSP scripts (`getDefaultFormulaScript`). <br>• [`RelativisticNodeGraph.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/RelativisticNodeGraph.cpp): Dynamic audio/control processing loop, feedback delay history buffers, audio power safety state. <br>• [`TimeDilationEngine.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/TimeDilationEngine.cpp): Coordinate time clock transformation, time warping, and Lorentz composition engine. <br>• [`RelativisticExpressionParser.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/RelativisticExpressionParser.cpp): ExprTk math parser for `$v1`, `$v2`, `$t`, and `tap()` expressions. |
| [`Source/gui/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/gui) | GUI Components & Design System: <br>• [`RelativisticCanvasComponent.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/gui/RelativisticCanvasComponent.cpp): Node canvas layout, cable catenary rendering, port hover rings, selection halos, inspector sidebar. <br>• [`CarbonGoldLookAndFeel.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/gui/CarbonGoldLookAndFeel.cpp): Slate sci-fi UI widgets, vector icons, custom knobs, buttons, and custom modals. <br>• [`FullStudioWorkstation.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/gui/FullStudioWorkstation.cpp): Main workstation container window. |
| [`web/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/web) | Web UI prototypes, JavaScript engines (`tidal_beat_engine.js`), and logo preview tools (`logo_preview.html`). |
| [`scripts/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/scripts) | Workspace utility scripts (e.g. `scripts/generate_code_index.py` for auto-updating `CODE_INDEX.md`). |

---

## 3. Active Development Focus (What Part We Are Working On)

The workspace is currently in **Active Production (Version 0.0.1)** with focus on:
1. **Node DSP Polish & Precision**: Implementation of global `SineLookupTable` (-130dBFS precision) and multi-waveform anti-aliased oscillators (`osc~`) with bottom-up code math.
2. **Relativistic Oscilloscope & Scope Nodes**: Enhancing `time.scope` into a CRT oscilloscope with time axis grid markings and dynamic sweep speed parameters.
3. **Canvas UX & Custom Modal Overlay**: Node drag-and-drop smoothness, replacing native default popups with custom `Carbon & Gold HelpModalOverlay`, and fixing UTF-8 encoding in HUD overlays.
4. **Expression Parsing & Wireless Tapping**: Extending `ExprTk` parser support for wireless `tap("nodeLabel.paramKey")` parameter queries and clipboard `[TAP]` integration.

---

## 4. External Open-Source Libraries Used & Sources

| Library Name | License / Source | Role & Implementation in Time Dilation DAW |
| :--- | :--- | :--- |
| **JUCE 7** | GPLv3 / Commercial ([juce.com](https://juce.com)) | C++ cross-platform audio application framework. Provides audio device manager, plugin wrappers (VST3/AU), GUI components, native vector rendering, LookAndFeel, and low-latency buffer management. |
| **Tracktion Engine** | GPLv3 / Commercial ([tracktion.com](https://www.tracktion.com)) | Open-source high-level audio engine built on top of JUCE. Powers multitrack arrangement timelines ([timeline]), audio clip scheduling, and transport synchronization. |
| **ExprTk** | Arash Partow Open-Source License ([partow.net](https://www.partow.net/programming/exprtk/)) | High-performance C++ Mathematical Expression Parsing and Evaluation Library. Integrated inside [`RelativisticExpressionParser.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/RelativisticExpressionParser.cpp) to evaluate real-time DSP math expressions (`$v1`, `$v2`, `$t`, `tap()`) without compiling native binary code. |
| **Pure Data (Pd) DSP Principles** | Open-Source BSD / Miller Puckette ([puredata.info](https://puredata.info)) | Mathematical formulas and object semantics for core DSP objects (`osc~`, `filter~`, `delay~`, `env~`, `gain~`, `out~`, `bang`) adapted as inspiration into native C++. *Note: No Pure Data source code or trademarked names are used directly.* |

---

## 5. Original Ideas & Conceptual Innovations (Conceived by Kijjaz & Team)

1. **Relativistic Time Dilation Architecture ($\gamma$)**:
   - Each node operates on a distinct coordinate time clock ($d\tau = \gamma \cdot dt$).
   - Time context flows down purple time cables (`NodePortType::Time`).
   - Supports time speed acceleration (`time.warp~`), temporal reversal (`time.retro~`), gravitational freeze (`time.stasis~`), step quantization (`time.quantize~`), and Lorentz velocity addition composition (`time.math~`).
2. **Dual Perspective Guarantee (Top-Down Visual & Bottom-Up Code Math)**:
   - Every node simultaneously exposes a visual control interface (sliders, mod inlets) AND an authentic C++ DSP math expression script (`$v1`, `$v2`, `$t`), allowing instant live-coding of DSP math directly inside visual node blocks.
3. **Wireless Global Signal Tapping (`tap()`)**:
   - Signal and parameter expressions can tap any node's output or parameter anywhere in the patch graph without drawing physical cables (`tap("nodeLabel.paramKey")` or `tap(id)`).
   - 1-Click `[TAP]` button on parameter sliders automatically copies tap syntax to the system clipboard.
4. **1-Block History Feedback Loop Stability (`previousBlockBuffer`)**:
   - The audio engine automatically detects topological feedback loops in the patch graph. Destination ports in feedback paths sample from a 1-block delay buffer (`previousBlockBuffer`), ensuring 100% real-time stability without sample rate crashes or thread deadlocks.
5. **Bidirectional Time-Audio Signal Interchange**:
   - Seamless conversion between purple time signals ($\gamma$) and cyan audio streams (`[audio2time~]`, `[time2audio~]`), enabling audio signals to dynamically deform coordinate time.
6. **Carbon & Gold + Slate Sci-Fi Workstation Aesthetics**:
   - Proprietary design system with Carbon background (`#070a12`), Slate cards (`#0d1322`), Royal Violet time accents (`#8b5cf6`), Cyber Cyan audio accents (`#06b6d4`), and Relativistic Gold math accents (`#f59e0b`). Enforces strict zero-emoji hygiene and relies exclusively on native C++ JUCE vector graphics calls.

---

## 6. Mandatory AI Agent Rules & Conventions

1. **Strict Naming Rule on "Pd"**:
   - Do NOT use the word `"Pd"` or `"PureData"` in code symbols, comments, or class names. Use `RelativisticNode`, `ExprAudioNode`, or `ControlNode` instead.
   - Signal objects end with `~` (e.g. `osc~`, `filter~`, `delay~`, `expr~`, `gain~`, `out~`). Control objects omit `~`. Relativistic time nodes start with `time.` (e.g. `time.warp~`, `time.retro~`).
2. **Audio Engine Power Default**:
   - Application MUST unconditionally default to `AUDIO: OFF (SAFE)` on launch. The user must manually click the power toggle button to start audio execution.
3. **Dual Perspective Implementation Rule**:
   - When adding any new node object, implement BOTH `getParameterDefs()` in `RelativisticNodeObjects.cpp` (Top-Down) AND `getDefaultFormulaScript()` (Bottom-Up code math).
4. **Wireless Tapping Support**:
   - Pass `parentGraph->tapSignal(target)` in expression evaluations to support `tap("node.param")`.
5. **Token Optimization & Line Mapping**:
   - **ALWAYS inspect [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) first** before viewing large source files. Specify narrow line ranges in `view_file`.
   - Run `python3 scripts/generate_code_index.py` after modifying code structures to keep line mappings accurate.
6. **Zero-Emoji Policy**:
   - Do NOT use unicode emojis in UI controls, popups, buttons, or inspector header labels. Use JUCE vector drawing.
