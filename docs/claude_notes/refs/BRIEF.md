Brief: finding academic references for Principia Softwarica chunks
=================================================================

Goal
----
The author wants the Principia Softwarica books (literate-programming
.nw files in ~/github/principia-softwarica) to point readers at the
ORIGINAL academic papers and reference books behind the algorithms and
techniques the code implements -- the classic 1950s-1990s papers that
students often never read (e.g. Bresenham 1965, Porter-Duff 1984,
Thompson 1968 regex, Feldman 1979 make, Sethi-Ullman 1970...).

The model to imitate is ~/playground (an OCaml graphics/audio/physics
project). Read a few of its .mli files first to see the style, e.g.:
  ~/playground/graphics/2d/Line.mli
  ~/playground/graphics/2d/Fill.mli
  ~/playground/graphics/core/Framebuffer.mli
  ~/playground/graphics/2d/geometry/Affine.mli
  ~/playground/graphics/3d/Clip.mli
Each "Reference:" there sits right next to the code implementing the
idea, gives author, title, venue, year, and a short parenthetical saying
WHAT in that paper is relevant (or how the code differs).

The references will go into the .nw files as %-comments placed on the
lines immediately BEFORE the `<<chunk name>>=` line of the chunk that
implements the algorithm, in this format:

  %ref: J. E. Bresenham, "Algorithm for computer control of a digital
  %  plotter", IBM Systems Journal 4(1):25-30, 1965.
  %ref: X. Wu, "An efficient antialiasing technique", SIGGRAPH 1991
  %  (road not taken: memdraw draws no antialiased lines).
  <<function [[_memimageline]]>>=

YOUR JOB IS RESEARCH ONLY. DO NOT EDIT ANY .nw FILE OR ANY SOURCE FILE.
Write your findings to the output file named in your prompt, and nothing
else.

What to look for
----------------
- Chunks implementing a recognizable classic algorithm, data structure,
  or technique: e.g. hashing, sorting, allocation (boundary tags, buddy,
  first-fit), regex (Thompson NFA), parsing (LALR, precedence climbing,
  recursive descent), register allocation, Sethi-Ullman numbering,
  peephole, span-dependent instruction sizing (Szymanski 1978), linking
  and relocation, dependency graph/topological sort, scheduling,
  synchronization (semaphores, monitors, CSP, rendezvous), paging/TLB,
  page replacement, TCP congestion control, checksums, CRC, compression
  (LZ77/LZW), rasterization, clipping, compositing, bitblt, layers,
  fonts, color quantization, dithering, event loops, etc.
- Also the key design papers of the systems themselves where they
  explain a specific chunk (Pike et al. Plan 9 papers, Thompson's
  "A New C Compiler", Hume's mk paper, Duff's rc paper, Pike's
  "Graphics in Overlapping Bitmap Layers", the 8 1/2 paper, the
  Plan 9 networking paper, etc.). Only if tied to a specific chunk.
- Classic reference BOOKS with a section number are fine too
  (Knuth TAOCP vol/section, Foley et al. section, Aho-Sethi-Ullman
  "Dragon book" section, Tanenbaum, Bach "Design of the UNIX OS",
  Lions' commentary, Stevens TCP/IP Illustrated).
- Optionally a "road not taken" ref (a classic alternative the code
  does NOT use), clearly marked as such. Use sparingly.

Quality rules (important)
-------------------------
- VERIFY against the actual C code (the .nw chunk body, or the C file
  it syncs with under the same directory) that the chunk really
  implements the thing you cite. Plan 9 code is often NOT the
  textbook algorithm; say so in the parenthetical when it differs.
- NEVER invent a paper. Only cite papers you are confident exist with
  the given author/title/venue/year. If unsure about a detail (volume,
  pages, exact year), leave it out rather than guess, and mark the
  entry "CONFIDENCE: medium" in your notes. Prefer omitting an entry
  over a shaky one.
- Prefer the ORIGINAL paper (the "forgotten classic") over a later
  survey; a modern retelling (e.g. Russ Cox 2007 on regex) can be a
  second %ref: line.
- Don't spray: aim for roughly 10-30 high-value entries per large
  book, fewer for small ones. One place per concept (the chunk where
  the algorithm actually lives), not every chunk that touches it.
- Skip spots where the .nw prose already \cite{}s the same work right
  there. (Existing bib: docs/latex/Principia.bib -- you may mention
  the bib key if one already exists.)
- Tags reference: docs/latex/Tags.tex (just so you know the
  conventions; %ref: is a new tag).

Output format (write to your assigned file)
-------------------------------------------
Plain text. One section per .nw file. For each entry:

  ### <file>.nw : <<exact chunk name as it appears after <<>> >>
  (line ~NNNN, section "<\section title>")
  Algorithm/idea: <one line>
  Verified: <one or two lines: what in the code confirms it, or how it
             differs from the textbook version>
  CONFIDENCE: high|medium
  Proposed lines:
  %ref: ...
  %  ...

At the end, a short "Considered but rejected" list (one line each) and
a "Missing bib keys noticed" line if any.
Your final message back should be just: the output path, the number
of entries, and 3-5 highlight entries (one line each).

ADDENDUM (author, mid-run): teaching value
------------------------------------------
Each %ref: must TEACH, like the playground .mli refs. The parenthetical
is not a bibliographic note; it gives the student a hook: the original
problem/context of the paper and how it maps to THIS code. Model
(~/playground/graphics/2d/Line.mli):

  Reference: Jack E. Bresenham, "Algorithm for computer control of a
  digital plotter", IBM Systems Journal 4(1):25-30, 1965 (for a pen
  plotter, whose motors could only step to neighboring grid points --
  the same problem as lighting pixels).

  Reference: Danny Cohen and Ivan Sutherland, 1967 (unpublished; first
  described in William M. Newman and Robert F. Sproull, "Principles of
  Interactive Computer Graphics", McGraw-Hill, 1973).

So for each entry, 1-3 lines of parenthetical saying e.g.: what machine
or problem the paper was written for, which idea in it is the one the
code uses (name the code's function/variable), and where the Plan 9
code differs or simplifies. Surprising origins (plotters, EDSAC, drum
memories, teletypes) are exactly what students remember.
Also give each per-field FOUNDING classic (first linker/loader, first
make, first shell, first debugger, first window system...) a place,
old ones preferred.
