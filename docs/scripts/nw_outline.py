#!/usr/bin/env python3
"""Extract the structural skeleton of a .nw file.

Keeps:
  - \\part / \\chapter / \\section / \\subsection / \\subsubsection /
    \\paragraph / \\subparagraph (including their starred * forms)
  - \\label{...} lines
  - %*** banner-comment blocks (a contiguous run of %-prefixed lines
    that contains at least one line starting with `%*`); this preserves
    the banner separators *and* the title comment line(s) inside them,
    e.g.
        %*****
        % Prelude
        %*****

Everything else (code chunks, prose, %claude: paragraphs, \\l/\\t/\\n
annotations, normal % comments outside a banner) is dropped.

Useful as a starting point for outline editing, ToC drafting, or for
diffing the high-level shape of a book.

Usage:
    ./nw_outline.py <file.nw>          # print to stdout
    ./nw_outline.py <file.nw> > out.txt
"""

import re
import signal
import sys

# Avoid noisy traceback when piping into head/less.
signal.signal(signal.SIGPIPE, signal.SIG_DFL)

SECTION = re.compile(
    r'^\\(part|chapter|section|subsection|subsubsection|'
    r'paragraph|subparagraph)\*?[\[\{]'
)
LABEL = re.compile(r'^\\label\{')
BANNER_STAR = re.compile(r'^%\*')
COMMENT = re.compile(r'^%')


LEVELS = {
    'part':           0,
    'chapter':        1,
    'section':        2,
    'subsection':     3,
    'subsubsection':  4,
    'paragraph':      5,
    'subparagraph':   6,
}

# Number of leading blank lines to insert before a group whose dominant
# (highest-ranking) heading is at this level.
LEAD_BLANKS = {
    0: 3,   # \part
    1: 2,   # \chapter
    2: 1,   # \section
    3: 1,   # \subsection
    4: 1,   # \subsubsection
    5: 0,   # \paragraph
    6: 0,   # \subparagraph
}


def section_level(line):
    m = SECTION.match(line)
    if not m:
        return None
    return LEVELS.get(m.group(1))


def outline(path):
    with open(path, encoding='utf-8', errors='replace') as f:
        lines = [l.rstrip('\n') for l in f]

    n = len(lines)
    keep = [False] * n

    # Pass 1: mark %-comment runs that contain at least one banner (%*) line.
    i = 0
    while i < n:
        if COMMENT.match(lines[i]):
            j = i
            has_star = False
            while j < n and COMMENT.match(lines[j]):
                if BANNER_STAR.match(lines[j]):
                    has_star = True
                j += 1
            if has_star:
                for k in range(i, j):
                    keep[k] = True
            i = j
        else:
            i += 1

    # Pass 2: mark sectioning commands and \label lines.
    for i, line in enumerate(lines):
        if SECTION.match(line) or LABEL.match(line):
            keep[i] = True

    # Group adjacent kept lines (a banner+chapter+banner+\label block stays
    # together because the source lines are consecutive).
    groups = []
    cur = []
    last = -2
    for i in range(n):
        if keep[i]:
            if i == last + 1:
                cur.append(i)
            else:
                if cur:
                    groups.append(cur)
                cur = [i]
            last = i
    if cur:
        groups.append(cur)

    def lead_blanks(group):
        levels = []
        has_banner = False
        for i in group:
            lvl = section_level(lines[i])
            if lvl is not None:
                levels.append(lvl)
            elif BANNER_STAR.match(lines[i]):
                has_banner = True
        if levels:
            return LEAD_BLANKS.get(min(levels), 1)
        if has_banner:
            return 1
        return 0  # label-only group: attach to preceding group

    # Emit with blank-line padding sized to the highest heading in each group.
    first = True
    for group in groups:
        if not first:
            for _ in range(lead_blanks(group)):
                print()
        first = False
        for i in group:
            print(lines[i])


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <file.nw>", file=sys.stderr)
        sys.exit(1)
    outline(sys.argv[1])


if __name__ == "__main__":
    main()
