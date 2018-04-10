This repository contains the source code of a fork of Plan9,
an operating system considered by many to be the successor
to Unix (see https://en.wikipedia.org/wiki/Plan_9_from_Bell_Labs).

The original Plan9 is available at https://9p.io/plan9/
There are a few other forks or close descendants of Plan9:
 - 9front
 - 9legacy
 - 9atom
 - Harvey
 - Nix
 - Jehanne
 - Inferno

The specificity of this fork is to see Plan9 as a great
educational platform to learn about programming and computer science.
The code of Plan9 is so elegant and small that you
can realistically understand the whole operating system.
In fact, I explained the whole code in my Principia Softwarica
book series (see https://principia-softwarica.org).

This fork, because it is used to support my Principia Softwarica
book series, contains only the essential programs used by a programmer
(e.g., compiler, linker, assembler, kernel, windowing system).
This fork does not contain all the programs of the original Plan9.
I've selected only what I consider to be the essence of an operating system.
Moreover, this fork supports only the ARM and x86 architectures. Even though
the original Plan9 supported more architectures, I think ARM and x86 are
enough for the educational purpose of Principia Softwarica.
Finally, as opposed to the original Plan9, this fork can also be
cross-compiled from Linux or MacOS, which makes it easy to experiment with.
