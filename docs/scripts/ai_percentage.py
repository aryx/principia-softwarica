#!/usr/bin/env python3
"""Compute the percentage of AI-written explanation lines per book.

Mirrors docs/scripts/nw_density.sh's prose definition, with one tweak:
LaTeX structure lines (\\section, \\begin, \\label, ...) count as
human-written prose rather than being skipped.

Prose classification:
  - Inside code chunks (<<...>>= ... @): code, not counted.
  - Outside code chunks, skip: blank lines, % comments, \\l/\\t/\\n one-line
    author annotations, <<...>> chunk references on their own line.
  - LaTeX structure lines are counted as prose attributed to the human author.
  - Everything else is prose; attributed to AI if inside a %claude: paragraph,
    to the human author otherwise.

A "%claude: paragraph" is opened by a line starting with "%claude:" and
closed by the next blank line.
"""
import os
import re
import sys

LATEX_STRUCT = re.compile(
    r'^\\(section|subsection|subsubsection|chapter|part|label|begin|end|'
    r'ifallcode|fi|input|include|usepackage|documentclass|newcommand|'
    r'renewcommand|makeatletter|makeatother|patchcmd)\b'
)
ANNOT = re.compile(r'^\\[ltn](\s|$)')
CODE_OPEN = re.compile(r'^<<.*>>=\s*$')
CODE_CLOSE = re.compile(r'^@(\s|$)')
CHUNK_REF = re.compile(r'^<<')
BLANK = re.compile(r'^\s*$')


def count(file_path):
    in_code = False
    in_claude = False
    prose = 0
    ai_prose = 0
    with open(file_path, encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.rstrip('\n')

            if CODE_OPEN.match(line):
                in_code = True
                in_claude = False
                continue
            if in_code:
                if CODE_CLOSE.match(line) or line == '@':
                    in_code = False
                continue

            if BLANK.match(line):
                in_claude = False
                continue

            if line.startswith('%claude:'):
                in_claude = True
                continue

            if line.startswith('%'):
                continue

            # LaTeX structure: counted as human prose, never AI.
            if LATEX_STRUCT.match(line):
                prose += 1
                continue

            if ANNOT.match(line) or line in (r'\l', r'\t', r'\n'):
                continue
            if CHUNK_REF.match(line):
                continue

            prose += 1
            if in_claude:
                ai_prose += 1
    return prose, ai_prose


# Mirrors the book table in docs/index.html.
BOOKS = [
    ("Kernel",           "kernel",          ["Kernel.nw", "Kernel_extra.nw", "Kernel_arm.nw", "Kernel_x86.nw"]),
    ("Core libraries",   "lib_core",        ["Libcore.nw", "Libcore_extra.nw", "Libcore_big.nw", "Libcore_arm.nw"]),
    ("Shell",            "shells",          ["Shell.nw", "Shell_extra.nw", "Intro.nw"]),
    ("C compiler",       "compilers",       ["Compiler.nw", "Compiler_extra.nw", "Compiler_x86.nw", "Intro.nw"]),
    ("Assembler",        "assemblers",      ["Assembler.nw", "Assembler_extra.nw", "Assembler_x86.nw", "Intro.nw"]),
    ("Linker",           "linkers",         ["Linker.nw", "Linker_extra.nw", "Linker_x86.nw", "Intro.nw"]),
    ("Editor",           "editors",         ["ed.nw"]),
    ("Build system",     "builders",        ["Make.nw", "Make_extra.nw", "Intro.nw"]),
    ("Debuggers",        "debuggers",       ["Debugger.nw", "Debugger_extra.nw", "Debugger_x86.nw", "db.nw"]),
    ("Profilers",        "profilers",       ["Profiler.nw", "Profiler_extra.nw"]),
    ("Graphics stack",   "lib_graphics",    ["Graphics.nw", "Graphics_extra.nw"]),
    ("Windowing system", "windows",         ["Windows.nw", "Windows_extra.nw", "Intro.nw"]),
    ("GUI toolkit",      "lib_gui",         ["Widgets.nw", "Widgets_extra.nw"]),
    ("Network stack",    "lib_networking",  ["Network.nw", "Network_extra.nw", "Network_v6.nw"]),
    ("Web browser",      "browsers",        ["Browser.nw", "Browser_extra.nw"]),
    ("CLI utilities",    "utilities",       ["Utilities.nw", "Utilities_extra.nw"]),
    ("Emulator",         "machine",         ["Machine.nw", "Machine_extra.nw"]),
]


def main():
    # Project root: parent of docs/scripts/.
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.abspath(os.path.join(here, "..", ".."))

    print(f"{'Book':<22} {'LOE':>7} {'AI':>7} {'%AI':>6}")
    print("-" * 46)
    total_prose = 0
    total_ai = 0
    for label, d, files in BOOKS:
        bp = 0
        ai = 0
        for fn in files:
            path = os.path.join(root, d, fn)
            if not os.path.exists(path):
                print(f"  [missing] {path}", file=sys.stderr)
                continue
            p, a = count(path)
            bp += p
            ai += a
        pct = 100.0 * ai / bp if bp else 0.0
        total_prose += bp
        total_ai += ai
        print(f"{label:<22} {bp:>7} {ai:>7} {pct:>5.1f}%")

    print("-" * 46)
    tpct = 100.0 * total_ai / total_prose if total_prose else 0.0
    print(f"{'TOTAL':<22} {total_prose:>7} {total_ai:>7} {tpct:>5.1f}%")


if __name__ == "__main__":
    main()
