#!/usr/bin/env python3
import os
import re

SOURCE_DIR = "Source"
OUTPUT_FILE = "CODE_INDEX.md"
AGENTS_OUTPUT_FILE = ".agents/CODE_INDEX.md"

def analyze_file(filepath):
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    entries = []

    class_pattern = re.compile(r'^\s*(class|struct)\s+([A-Za-z0-9_]+)')
    method_pattern = re.compile(r'^(?:[A-Za-z0-9_<>:*\s]+?\s+)?([A-Za-z0-9_]+::[A-Za-z0-9_]+)\s*\([^;]*?\)\s*$(?!\s*;)')
    node_comment_pattern = re.compile(r'//\s*(\d+[a-z]?\.\s*\[[A-Za-z0-9_.~]+\][^\n]*)')

    for idx, line in enumerate(lines, 1):
        line_str = line.strip()

        # Check for Node Object section comment
        node_match = node_comment_pattern.search(line_str)
        if node_match:
            entries.append({
                "type": "section",
                "name": node_match.group(1),
                "line": idx
            })
            continue

        # Check for class / struct declaration
        class_match = class_pattern.match(line)
        if class_match:
            symbol_name = class_match.group(2)
            entries.append({
                "type": "class",
                "name": symbol_name,
                "line": idx
            })
            continue

        # Check for C++ method definition
        if "::" in line_str and "(" in line_str and not line_str.startswith("//") and not line_str.startswith("*"):
            method_match = method_pattern.search(line_str)
            if method_match:
                m_name = method_match.group(1)
                entries.append({
                    "type": "method",
                    "name": m_name,
                    "line": idx
                })

    return len(lines), entries

def main():
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root_dir)

    md_lines = []
    md_lines.append("# Code Structure & Line Number Index\n")
    md_lines.append("> **Auto-generated index for fast token-efficient code navigation.**\n")
    md_lines.append("Use this document to pinpoint exact line numbers before invoking `view_file` or `replace_file_content`.\n")

    cpp_files = []
    for dirpath, _, filenames in os.walk(SOURCE_DIR):
        for f in sorted(filenames):
            if f.endswith(".h") or f.endswith(".cpp"):
                cpp_files.append(os.path.join(dirpath, f))

    cpp_files.sort()

    md_lines.append("## Summary of Source Files\n")
    md_lines.append("| File | Total Lines | Category |")
    md_lines.append("| :--- | :--- | :--- |")

    file_details = []

    for rel_path in cpp_files:
        total_lines, entries = analyze_file(rel_path)
        category = "Core"
        if "dsp" in rel_path:
            category = "Audio & DSP Engine"
        elif "gui" in rel_path:
            category = "GUI & Visual Elements"

        link_str = f"[{os.path.basename(rel_path)}](file://{os.path.abspath(rel_path)})"
        md_lines.append(f"| {link_str} | {total_lines} lines | {category} |")
        file_details.append((rel_path, total_lines, entries))

    md_lines.append("\n---\n")
    md_lines.append("## Detailed Symbol Location Map\n")

    for rel_path, total_lines, entries in file_details:
        file_basename = os.path.basename(rel_path)
        abs_path = os.path.abspath(rel_path)
        md_lines.append(f"### [{file_basename}](file://{abs_path}) ({total_lines} lines)\n")

        if not entries:
            md_lines.append("_No major top-level class or method symbols detected._\n")
            continue

        md_lines.append("| Line | Symbol / Description | Type |")
        md_lines.append("| :--- | :--- | :--- |")

        for e in entries:
            line_num = e["line"]
            line_link = f"[{line_num}](file://{abs_path}#L{line_num})"
            name_clean = e["name"].replace("<", "&lt;").replace(">", "&gt;")
            md_lines.append(f"| {line_link} | `{name_clean}` | {e['type'].upper()} |")

        md_lines.append("")

    content = "\n".join(md_lines)

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write(content)

    os.makedirs(os.path.dirname(AGENTS_OUTPUT_FILE), exist_ok=True)
    with open(AGENTS_OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"Generated {OUTPUT_FILE} and {AGENTS_OUTPUT_FILE} successfully!")

if __name__ == "__main__":
    main()
