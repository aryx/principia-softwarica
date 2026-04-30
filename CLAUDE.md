# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Principia Softwarica is a set of literate programming books (.nw files) explaining a curated fork of Plan 9 from Bell Labs. Each book covers a core programmer tool (shell, build system, assembler, linker, compiler, kernel, etc.) using Noweb format to produce LaTeX/PDF documents.

The primary task is **completing these books** by adding explanatory prose (`%claude:` tagged paragraphs) before code chunks in .nw files. See `PROMPT.txt` for the author's detailed instructions and `~/.claude/CLAUDE.md` for the full Noweb writing style guide.

## Build System

The project uses Plan 9's `mk` (not GNU make) with `rc` shell. Requires [Goken](https://github.com/aryx/goken9cc) toolchain.

```bash
# Environment setup (required before any build commands)
source env.sh

# Build targets (run with mk, not make)
mk all          # Build all libraries and programs
mk libs         # Build libraries only
mk cmds         # Build programs only
mk kernel       # Build kernel
mk disk         # Create bootable disk image
mk clean        # Clean build artifacts

# Literate programming targets
mk pdf          # Generate PDF books from .nw files
mk sync         # Synchronize .nw files with source code (uses syncweb)
mk syncfast     # Sync only SRC_VIEWS (faster)
mk lpclean      # Clean LaTeX/PDF artifacts
mk pdfinstall   # Install PDFs to ROOT/pdfs/
```

There is also a thin `Makefile` wrapper, but the real build logic is in `mkfile` (rc shell syntax). Each subdirectory has its own `mkfile`.

## Book/Directory Structure

Each major component directory (e.g., `shells/`, `builders/`, `assemblers/`) contains:
- `<Tool>.nw` — main book (e.g., `Shell.nw`, `Make.nw`) that `#include`s the extra chapters
- `Intro.nw` — introduction chapter
- `*_extra.nw` — appendix material
- `mkfile` — literate programming build rules (sets DOC, VERSION, SRC_VIEWS, SRC_ORIG, includes `docs/latex/mkcommon`)
- `defs_and_uses.list` — syncweb cross-reference index
- Source subdirectories with actual C code (e.g., `shells/rc/`, `builders/mk/`)

## Key Configuration Files

- `mkconfig` — lists which LIBS and PROGRAMS to build
- `mkfile-host-{Linux,macOS,Cygwin,Plan9}` — host-specific build rules
- `mkfile-target-{pc,pi}` — target architecture (x86 or ARM)
- `docs/latex/mkcommon` — shared literate programming build rules (syncweb → pdflatex pipeline)
- `docs/latex/Config.tex` — shared LaTeX configuration

## Working with .nw Files

The `.nw` files are the primary work product. When adding explanations:

1. Read existing completed books as style models (e.g., `assemblers/Intro.nw`, `builders/Intro.nw`, `builders/Make.nw`)
2. Add `%claude:` on the line before every paragraph you write
3. Preserve all existing `%`, `\l`, `\t`, `\n` annotations — never remove or relocate them
4. Place explanations **before** code chunks (`<<name>>=`), never inside them
5. Focus on the "why" and non-obvious aspects; assume readers are competent programmers
6. For appendix chapters, even less explanation are needed usually
7. See also the shared ~/.claude/CLAUDE.md for more conventions

The `syncweb` tool keeps .nw files and source code in sync. Comments marked `(* s: ... *)`, `(* e: ... *)`, `(* x: ... *)` in source files are syncweb chunk markers — never modify them.

## Tag Conventions

`docs/latex/Tags.tex` is the authoritative reference for all `%`-tags used in `.nw` files. It documents provenance tags, theme tags, and the tagging process. Read it before doing any tagging pass.

Key points:

- **Provenance**: `%claude:` (required on every AI-written paragraph), `%yoann:`/`%yoann1/2/3:` (author directives).
- **Core theme tags** (highest priority; will become visual LaTeX boxes): `%terminology:`, `%cs-history:`, `%modern:`, `%others:`, `%plan9-is-cleaner:`. One theme tag per paragraph at most.
- `%plan9-is-cleaner:` **replaces the older `%unix:` tag** — do not write `%unix:` in new prose.
- **Don't tag core content.** Paragraphs that explain what Plan 9's code *is* or *does* stay untagged. Tag only paragraphs that a reader could skip without losing the code.
- **Placement**: tag on its own `%`-comment line, or grouped with `%claude:` (e.g. `%claude: %cs-history:`). When only part of a paragraph is on-theme, put the tag before the relevant sentence, not at the paragraph start.
- `%history:` and `%alt:` are the author's *telegraphic* notes and are NOT theme tags; `%cs-history:` and `%modern:` are their rendered-prose counterparts.

Systematic tagging passes should focus on Principles sections — find them with `grep -rn 'section{.*principles' --include='*.nw'`.
