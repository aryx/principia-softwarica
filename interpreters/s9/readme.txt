
Scheme 9 from Empty Space
An Interpreter for a Subset of Scheme
Copyright (C) 2007 Nils M Holm

S9fES is an interpreter for a broad subset of R5RS Scheme.
For details about to dos and omissions see the manual page.

To compile S9fES, run make test (this will also run the test
suite to make sure that the interpreter works properly).

To install S9fES, copy the s9 binary to a location where it can
be executed (i.e. /usr/local/bin) and the library s9.scm to a
publically readable directory (i.e. /usr/local/share/s9fes).

The nroff source code of the manual page is in the file s9.1.
You may want to install it in your manual section 1. In case
you are not running Unix, there is a pre-formatted copy in the
file s9.1.txt.

You can install S9fES by typing "make install" as root but if
you do so, please edit the *DIR variables at the beginning of
Makefile first.

Finally make the environment variable S9FES_LIBRARY_PATH point
to the directory containing s9.scm. The variable may contain a
single directory or a colon-separated list of directories which
will be searched in sequence.

Optionally, you can set up an rc file in your home directory
($HOME/.s9fes/rc). This is an ordinary Scheme source file that
will be read after loading the library, but before running any
programs.

Bug reports, patches, suggestions, questions, etc are welcome.

Nils M Holm <nmh@t3x.org>

