This repository contains the source code of a fork of Plan 9 meant for
education.

Plan 9 is an operating system considered by many to be the successor
to Unix (see https://en.wikipedia.org/wiki/Plan_9_from_Bell_Labs).
The code of the original Plan 9 is available at https://9p.io/plan9/
There are a few other forks of Plan 9:
 - 9front: http://9front.org/
 - 9legacy: http://9legacy.org/
 - Nix: https://code.google.com/archive/p/nix-os/
 - 9atom: http://www.9atom.org/ (DEAD in 2025)
 - Harvey: https://harvey-os.org/ (DEAD in 2025)
 - Jehanne: http://jehanne.io/ (DEAD in 2025)
or close descendants  of Plan 9:
 - Inferno: https://github.com/inferno-os/inferno-os

The specificity of my fork is to see Plan 9 as a great educational
platform to learn about programming and computer science.  The code of
Plan 9 is so elegant and small that you can realistically understand
the whole operating system.  In fact, I explained the whole code in my
Principia Softwarica book series (see
https://principia-softwarica.org).

This fork, because it is used to support my Principia Softwarica
book series, contains only the essential programs used by a programmer
(e.g., compiler, linker, assembler, kernel, windowing system).
This fork does not contain all the programs of the original Plan 9;
I've selected only what I consider to be the essence of an operating system.
Moreover, this fork supports only the ARM and x86 architectures. Even though
the original Plan 9 supported more architectures, I think ARM and x86 are
enough for the educational purpose of Principia Softwarica.
Finally, as opposed to the original Plan 9, this fork can also be
cross-compiled from Linux, MacOS, or Windows, which makes it easy
to experiment with. To cross-compile this fork you will need
to install Goken, see https://github.com/aryx/goken9cc

Enjoy.
