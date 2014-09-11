# Scheme 9 From Empty Space
# Copyright (C) 2007 Nils M Holm

CFLAGS=	-g -O -Wall -ansi -pedantic

BINDIR=	/usr/local/bin
LIBDIR=	/usr/local/share/s9fes
MANDIR=	/usr/local/man/man1

s9:	s9.c
	cc $(CFLAGS) -o s9 s9.c

test:	s9
	./s9 -f test.scm && rm -f __testfile__

install:
	strip s9
	install -d -m 0755 $(LIBDIR)
	install -m 0755 s9 $(BINDIR)
	install -m 0644 s9.scm $(LIBDIR)
	install -m 0644 s9.1 $(MANDIR)
	gzip -9f $(MANDIR)/s9.1

clean:
	rm -f s9 __testfile__ *.o *.core core
