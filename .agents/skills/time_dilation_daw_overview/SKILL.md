---
name: time-dilation-daw-overview
description: Comprehensive index, architectural rules, information directory, active work focus, open-source library sources, and original innovations for Time Dilation DAW Workstation.
---

# Time Dilation DAW Workstation — Project Overview & Agent Skill

This skill provides AI agents with instant project orientation, information routing, active development targets, external library sources, original innovations, and mandatory coding rules.

## 1. Project Identification
- **Project**: Time Dilation DAW Workstation (v0.0.1)
- **Architect & Lead Producer**: Kijjaz
- **Platform Stack**: Native C++20 / JUCE 7 & Tracktion Engine (macOS Standalone & VST3/AU Plugin)
- **Aesthetic Identity**: Carbon & Gold + Slate Sci-Fi Workstation (Cubase-Inspired)

---

## 2. Information Map (Where to Look)
- **Token Optimization & Symbol Navigation**: Inspect [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) or [`.agents/CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/.agents/CODE_INDEX.md) for line number mappings of C++ classes and methods before viewing source code files.
- **Architecture & System Design**: Read [`ABOUT.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/ABOUT.md) for executive summary, dual-perspective inspector, audio VU metering, and undo state mechanics.
- **User & Object Reference Manual**: Read [`HELP.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/HELP.md) for symbol commands, object method tables (`[osc~]`, `[table]`, `[sampler~]`, `[time.transport]`, etc.), port compatibility, and timeline rules.
- **Mathematical Foundations**: Refer to [`math/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/math) for 28 LaTeX documents covering relativistic time physics (`01_relativistic_time_dilation.tex`), 1-block delay stability (`05_feedback_loop_stability.tex`), Lorentz velocity addition (`13_lorentz_time_signal_composition.tex`), and lookahead causality (`future_lookahead_causality.tex`).
- **Core DSP Source Code**: Inspect [`Source/dsp/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp) for graph execution (`RelativisticNodeGraph.cpp`), node objects (`RelativisticNodeObjects.cpp`), expression parsing (`RelativisticExpressionParser.cpp`), and coordinate time engines (`TimeDilationEngine.cpp`).
- **GUI & LookAndFeel**: Inspect [`Source/gui/`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/gui) for canvas rendering (`RelativisticCanvasComponent.cpp`), slate widgets (`CarbonGoldLookAndFeel.cpp`), and workstation container (`FullStudioWorkstation.cpp`).

---

## 3. Active Development Focus
1. **High-Precision Node Oscillators**: Adding `-130dBFS` precision `SineLookupTable` and anti-aliased waveforms to `osc~` with bottom-up code math.
2. **Relativistic Scope**: CRT oscilloscope (`time.scope`) with time-axis grid lines and sweep speed modulation.
3. **Canvas UX & Custom Modals**: Drag & drop smoothness, replacing default dialogs with `HelpModalOverlay`, fixing UTF-8 text encoding.
4. **Wireless Tapping & Expression Parsing**: Expanding `ExprTk` parser support for `tap("nodeLabel.paramKey")` and clipboard `[TAP]` integration.

---

## 4. Open-Source External Libraries & Attributions
1. **JUCE 7** ([juce.com](https://juce.com)): C++ cross-platform audio app framework (Audio I/O, Plugin API VST3/AU, GUI, LookAndFeel, Vector graphics).
2. **Tracktion Engine** ([tracktion.com](https://www.tracktion.com)): High-level open-source audio engine built on JUCE for multitrack arrangement, audio clip scheduling, and transport timeline management.
3. **ExprTk** ([partow.net](https://www.partow.net/programming/exprtk/)): Arash Partow's C++ mathematical expression parsing library, powering real-time evaluation of `$v1`, `$v2`, `$t`, and `tap()` expressions inside [`RelativisticExpressionParser.cpp`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/Source/dsp/RelativisticExpressionParser.cpp).
4. **Pure Data (Pd) DSP Principles** ([puredata.info](https://puredata.info)): Mathematical formulations and object semantics by Miller Puckette, adapted into clean, independent C++ DSP routines.

---

## 5. Original Ideas & Conceptual Innovations (Conceived by Kijjaz & Team)
1. **Relativistic Time Dilation ($\gamma$)**: Independent coordinate time clocks per node ($d\tau = \gamma \cdot dt$), purple time cables, continuous time warping (`time.warp~`), temporal reversal (`time.retro~`), gravitational stasis (`time.stasis~`), relativistic grid quantization (`time.quantize~`), and Lorentz time composition (`time.math~`).
2. **Dual Perspective Guarantee**: Simultaneous top-down visual control UI (sliders, mod inlets) AND editable bottom-up C++ DSP math expressions (`$v1`, `$v2`, `$t`) per object block.
3. **Wireless Global Signal Tapping (`tap()`)**: Cable-free signal/parameter sampling across the graph (`tap("nodeLabel.paramKey")` or `tap(id)`) with 1-click clipboard copy buttons (`[TAP]`).
4. **1-Block History Feedback Loop Stability (`previousBlockBuffer`)**: Topological feedback detection that routes feedback destination ports to read from a 1-block delay buffer, ensuring 100% DSP stability without buffer starvation or thread deadlocks.
5. **Bidirectional Time-Audio Signal Interchange**: Direct conversion between purple time signals ($\gamma$) and cyan audio streams (`[audio2time~]`, `[time2audio~]`).
6. **Carbon & Gold + Slate Sci-Fi Aesthetic**: High-density used-future dark UI design system (Carbon `#070a12`, Cyan `#06b6d4`, Gold `#f59e0b`, Violet `#8b5cf6`), zero-emoji policy, and pure JUCE vector graphic calls.

---

## 6. Mandatory Agent Coding Rules
- Do NOT use `"Pd"` or `"PureData"` in code symbols or class names.
- Application MUST default to `AUDIO: OFF (SAFE)` on launch.
- Always implement both Top-Down (`getParameterDefs`) and Bottom-Up (`getDefaultFormulaScript`) for every new node.
- Check [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) first to save token usage before viewing source files.
- Run `python3 scripts/generate_code_index.py` whenever updating symbol structures.
