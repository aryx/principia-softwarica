#!/bin/bash
# nw_density.sh — Measure explanation density (prose/code ratio) in .nw files
#
# Usage:
#   ./nw_density.sh <file.nw>              # per-chapter breakdown
#   ./nw_density.sh <file.nw> --summary    # file-level summary only
#
# Reference targets:
#   Assembler.nw  ~1.19  (excellent)
#   Linker.nw     ~0.72  (good)
#   Below 0.5: needs work
#   Below 0.2: critical
#
# What counts:
#   CODE:  lines between <<...>>= and @ markers
#   PROSE: non-blank lines between chunks that are actual explanatory text
#          (excludes % comments, \l/\t/\n annotations, pure LaTeX structure)

set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <file.nw> [--summary]"
    exit 1
fi

FILE="$1"
SUMMARY_ONLY="${2:-}"

if [ ! -f "$FILE" ]; then
    echo "Error: $FILE not found"
    exit 1
fi

# Use awk to classify each line and tally per chapter
awk '
BEGIN {
    chapter = "(preamble)"
    in_code = 0
    code_lines = 0
    prose_lines = 0
    total_code = 0
    total_prose = 0
    nchapters = 0
}

# Track chapter boundaries
/^\\chapter\{/ {
    # Save previous chapter stats
    if (code_lines > 0 || prose_lines > 0) {
        chapters[nchapters] = chapter
        ch_code[nchapters] = code_lines
        ch_prose[nchapters] = prose_lines
        nchapters++
    }
    # Extract chapter name (POSIX awk compatible)
    chapter = $0
    sub(/.*\\chapter\{/, "", chapter)
    sub(/\}.*/, "", chapter)
    code_lines = 0
    prose_lines = 0
    next
}

# Code chunk start
/^<<.*>>=$/ {
    in_code = 1
    next
}

# Code chunk end
/^@[[:space:]]*$/ || /^@ $/ || /^@$/ {
    if (in_code) {
        in_code = 0
        next
    }
}

# Inside code chunk — count as code
in_code {
    code_lines++
    total_code++
    next
}

# Outside code chunks — classify the line
!in_code {
    # Skip blank lines
    if ($0 ~ /^[[:space:]]*$/) next

    # Skip pure LaTeX structure
    if ($0 ~ /^\\(section|subsection|subsubsection|chapter|part|label|begin|end|ifallcode|fi|input|include)\{?/) next
    if ($0 ~ /^\\(usepackage|documentclass|newcommand|renewcommand|makeatletter|makeatother|patchcmd)/) next

    # Skip author annotations (\l, \t, \n at start of line)
    if ($0 ~ /^\\[ltn] /) next
    if ($0 ~ /^\\[ltn]$/) next

    # Skip % comment lines (author notes, not explanatory prose)
    if ($0 ~ /^%/) next

    # Skip systab/enum/constant/macro chunk references
    if ($0 ~ /^<</) next

    # What remains is prose (author explanations or %claude: tagged text)
    # Note: %claude: lines were already skipped above since they start with %
    # But the PROSE that follows %claude: tags does NOT start with %
    prose_lines++
    total_prose++
}

END {
    # Save last chapter
    if (code_lines > 0 || prose_lines > 0) {
        chapters[nchapters] = chapter
        ch_code[nchapters] = code_lines
        ch_prose[nchapters] = prose_lines
        nchapters++
    }

    if (ENVIRON["SUMMARY_ONLY"] == "--summary") {
        if (total_code > 0)
            printf "%-40s  code=%5d  prose=%5d  ratio=%.3f\n", FILENAME, total_code, total_prose, total_prose/total_code
        else
            printf "%-40s  code=%5d  prose=%5d  ratio=N/A\n", FILENAME, total_code, total_prose
    } else {
        printf "=== %s ===\n", FILENAME
        printf "%-35s %6s %6s %7s\n", "Chapter", "Code", "Prose", "Ratio"
        printf "%-35s %6s %6s %7s\n", "-------", "----", "-----", "-----"

        for (i = 0; i < nchapters; i++) {
            if (ch_code[i] > 0)
                ratio = ch_prose[i] / ch_code[i]
            else
                ratio = 0

            # Flag chapters below threshold
            flag = ""
            if (ch_code[i] > 50) {  # skip tiny chapters
                if (ratio < 0.2) flag = " !!!"
                else if (ratio < 0.5) flag = " *"
            }

            printf "%-35s %6d %6d %7.3f%s\n", substr(chapters[i], 1, 35), ch_code[i], ch_prose[i], ratio, flag
        }

        printf "%-35s %6s %6s %7s\n", "-------", "----", "-----", "-----"
        if (total_code > 0)
            printf "%-35s %6d %6d %7.3f\n", "TOTAL", total_code, total_prose, total_prose/total_code
        else
            printf "%-35s %6d %6d %7s\n", "TOTAL", total_code, total_prose, "N/A"
        printf "\nLegend: !!! = critical (<0.2)  * = needs work (<0.5)\n"
        printf "Reference: Assembler.nw ~1.19, Linker.nw ~0.72\n"
    }
}
' SUMMARY_ONLY="$SUMMARY_ONLY" "$FILE"
