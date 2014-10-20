#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"

void
fatal(int syserr, char *fmt, ...)
{
	char buf[ERRMAX], *s;
	va_list arg;

	va_start(arg, fmt);
	vseprint(buf, buf+sizeof(buf), fmt, arg);
	va_end(arg);
	s = "5i: %s\n";
	if(syserr)
		s = "5i: %s: %r\n";
	fprint(2, s, buf);
	exits(buf);
}

void
itrace(char *fmt, ...)
{
	char buf[128];
	va_list arg;

	va_start(arg, fmt);
	vseprint(buf, buf+sizeof(buf), fmt, arg);
	va_end(arg);
	Bprint(bioout, "%8lux %.8lux %2d %s\n", reg.ar, reg.ir, reg.class, buf);	
}

void
dumpreg(void)
{
	int i;

	Bprint(bioout, "PC  #%-8lux SP  #%-8lux \n",
				reg.r[REGPC], reg.r[REGSP]);

	for(i = 0; i < 16; i++) {
		if((i%4) == 0 && i != 0)
			Bprint(bioout, "\n");
		Bprint(bioout, "R%-2d #%-8lux ", i, reg.r[i]);
	}
	Bprint(bioout, "\n");
}

void
dumpfreg(void)
{
}

void
dumpdreg(void)
{
}

void *
emalloc(ulong size)
{
	void *a;

	a = malloc(size);
	if(a == 0)
		fatal(0, "no memory");

	memset(a, 0, size);
	return a;
}

void *
erealloc(void *a, ulong oldsize, ulong size)
{
	void *n;

	n = malloc(size);
	if(n == 0)
		fatal(0, "no memory");
	memset(n, 0, size);
	if(size > oldsize)
		size = oldsize;
	memmove(n, a, size);
	return n;
}
