#!/usr/bin/env python3
# Claude Code
#
# Copyright (C) 2026 Yoann Padioleau
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
"""Count lines of code (LOC) the way `syncweb -loc` does, per source file.

syncweb's counter (syncweb/frontend/Stat.ml) walks a .nw file and counts a
line inside a `<<chunk>>=` ... `@` code chunk as one LOC unless it is:
  - empty or whitespace-only, or
  - a nested chunk-name reference `<<...>>` (counted as LOE, not LOC).
Note that ordinary code comments ARE counted as LOC; only blank lines and
the chunk *structure* are excluded.

Applied to the *tangled* source files that syncweb writes out, the same
definition becomes:

  LOC = non-blank lines that are not a syncweb chunk marker.

A chunk marker is the in-code comment syncweb emits for a chunk boundary
or a nested `<<...>>` reference, i.e. a line whose first non-space content
is `/*s:`, `/*e:` or `/*x:` (C) or `(*s:`, `(*e:`, `(*x:` (OCaml).

This reproduces syncweb's LOC for the actual source files. It will be a bit
lower than `syncweb -loc` run on the whole .nw, because the latter also
counts illustrative code chunks shown only in the prose (e.g. hello.mk),
which are not part of the compiled source.

Usage:
  loc.py FILE...           # one LOC count per file, plus a total

Typically driven from a mkfile, e.g.:
  loc:V:
      python3 $TOP/docs/scripts/loc.py $SRC_VIEWS
"""

import re
import sys

# A syncweb chunk-structure marker line, e.g. "/*s: function [[foo]] */".
# These stand for the noweb chunk boundaries and nested <<...>> references,
# which syncweb does NOT count as LOC.
MARKER = re.compile(r"^\s*[/(]\*\s*[sex]:")


def loc_of_file(path):
    n = 0
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.strip() == "":
                continue
            if MARKER.match(line):
                continue
            n += 1
    return n


def main(argv):
    files = argv[1:]
    if not files:
        sys.exit("usage: loc.py FILE...")
    width = max(len(f) for f in files)
    total = 0
    for f in files:
        n = loc_of_file(f)
        total += n
        print(f"{f:<{width}}  {n:>6}")
    print(f"{'-' * width}  {'-' * 6}")
    print(f"{'TOTAL':<{width}}  {total:>6}")


if __name__ == "__main__":
    main(sys.argv)
