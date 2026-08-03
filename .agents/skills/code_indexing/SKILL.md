---
name: code-indexing
description: Auto-generates and maintains CODE_INDEX.md line number mapping for token-efficient file navigation. Use whenever adding new nodes, modifying symbol structures, or navigating large C++ files.
---

# Code Indexing & Token-Efficient Navigation Skill

This skill ensures AI agents can navigate large codebase files (such as 3,000-line C++ files) with minimal token usage by maintaining an automated line-number location map in `CODE_INDEX.md`.

## Workflow Overview

1. **Before Viewing Code**:
   - Check [`CODE_INDEX.md`](file:///Users/kijjaz/Desktop/Antigravity/2026/20260801%20Time%20Dilation%20DAW/CODE_INDEX.md) to locate the exact file and starting line number for the class or method you need to inspect.
   - Use `view_file` with explicit `StartLine` and `EndLine` parameters covering only the targeted function (e.g. 50-100 lines instead of the full 3000 lines).

2. **Updating the Index**:
   - Run the automated generator script whenever file structures change:
     ```bash
     python3 scripts/generate_code_index.py
     ```
   - This automatically regenerates `CODE_INDEX.md` and `.agents/CODE_INDEX.md` with accurate clickable line numbers.
