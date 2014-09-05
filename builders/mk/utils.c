#include "mk.h"

void *
Malloc(int n)
{
	register void *s;

	s = malloc(n);
	if(!s) {
		fprint(2, "mk: cannot alloc %d bytes\n", n);
		Exit();
	}
	return(s);
}

void *
Realloc(void *s, int n)
{
	if(s)
		s = realloc(s, n);
	else
		s = malloc(n);
	if(!s) {
		fprint(2, "mk: cannot alloc %d bytes\n", n);
		Exit();
	}
	return(s);
}


