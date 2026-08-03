# Project Guidelines & Architectural Rules for AI Agents
> **Workspace**: Time Dilation DAW (Version 4.0)  
> **Target Platform**: macOS (JUCE 7 / C++20 Standalone & Plugin)  

---

## 1. Naming & Brand Conventions
- **Strict Rule on "Pd"**: Do NOT use the word `"Pd"` or `"PureData"` in code symbols, comments, or class names unless directly derived from authentic Pure Data source code files. Use `RelativisticNode`, `ExprAudioNode`, or `ControlNode` instead.
- **Node Type Naming**: Signal objects end with `~` (e.g. `osc~`, `filter~`, `delay~`, `expr~`, `gain~`, `out~`, `env~`, `tap~`). Control objects omit `~` (e.g. `expr`, `tap`). Relativistic time engines start with `time.` (e.g. `time.warp~`, `time.retro~`).

---

## 2. Audio Engine Power Default & Safety
- **Power Default**: The application MUST default to `AUDIO: OFF (SAFE)` on launch. The user must manually click the power toggle button to start the audio processing graph, preventing unexpected loud audio bursts.

---

## 3. Dual Perspective Guarantee (Top-Down & Bottom-Up)
Every new node object added to the engine MUST implement both perspectives:
1. **Top-Down (Visual)**: Implement `getParameterDefs()` in `RelativisticNodeObjects.cpp` returning explicit parameter keys, names, initial values, authentic min/max ranges, and modulation inlet indices (`modInletIdx`).
2. **Bottom-Up (Code Math)**: Implement `getDefaultFormulaScript()` returning authentic C++ DSP math expressions detailing inputs (`$v1`, `$v2`), parameters, dynamic coordinate time (`$t`), and output assignments.

---

## 4. Wireless Global Signal Tapping Pattern (`tap()`)
- Any expression evaluation logic MUST pass `parentGraph->tapSignal(target)` to support wireless tapping (`tap("node.param")` or `tap(id)`).
- When a parameter slider in the TOP-DOWN inspector is clicked (`[TAP]`), copy `tap('nodeLabel.paramKey')` to the clipboard and insert it into active expression text fields.

---

## 5. Audio Feedback Loop Stability Pattern
- All audio connections between ports MUST check for feedback loops.
- When an audio connection forms a feedback loop, the destination port MUST read from `previousBlockBuffer` (1-block history delay buffer) rather than live `audioData`. This guarantees absolute real-time DSP stability without buffer starvation or audio thread lockup.

---

## 6. UI Aesthetics & Zero-Emoji Design Hygiene
- **Zero Emojis**: Do NOT use unicode emojis in native UI controls, popup menus, buttons, or inspector header labels.
- **Design System**: Carbon & Gold + Slate Sci-Fi aesthetic (Cubase-inspired).
  - Canvas background: `#070a12` with micro-dot grid matrix (`#1a94a3b8`).
  - Dark Slate cards: `#0d1322` / `#111827`.
  - Color accents: Royal Violet (`#8b5cf6`), Cyber Cyan (`#06b6d4`), Relativistic Gold (`#f59e0b`).
  - Selection Halo: Glowing Cyan outline (`#38bdf8`).
- **Icons**: Draw vector antenna icons, LED meters, and port rings directly using JUCE native C++ graphics vector calls.

---

## 7. Token Optimization & Code Indexing Rule (`CODE_INDEX.md`)
- **Always Check `CODE_INDEX.md` First**: Before reading or modifying large source files (such as `RelativisticCanvasComponent.cpp` or `RelativisticNodeObjects.cpp`), inspect [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) to find the exact line numbers for target classes and methods.
- **Narrow Range Inspection**: Always specify exact `StartLine` and `EndLine` in `view_file` calls to save context tokens.
- **Auto-Update Index**: When adding new classes or modifying symbol line numbers significantly, run `python3 scripts/generate_code_index.py` to keep `CODE_INDEX.md` up to date.
