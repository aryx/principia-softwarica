#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

/*
 * atomic ops
 * make sure that we don't drag in the C library versions
 */

long
arch_xdec(long *p)
{
	int s, v;

	s = arch_splhi();
	v = --*p;
	arch_splx(s);
	return v;
}

void
arch_xinc(long *p)
{
	int s;

	s = arch_splhi();
	++*p;
	arch_splx(s);
}

// overwrite libc!
int
ainc(int *p)
{
	int s, v;

	s = arch_splhi();
	v = ++*p;
	arch_splx(s);
	return v;
}

int
adec(int *p)
{
	int s, v;

	s = arch_splhi();
	v = --*p;
	arch_splx(s);
	return v;
}

bool
cas32(void* addr, u32int old, u32int new)
{
	bool r;
    int s;

	s = arch_splhi();
    r = (*(u32int*)addr == old);
	if(r)
		*(u32int*)addr = new;
	arch_splx(s);
	if (r)
		arch_coherence();
	return r;
}

bool
arch_cmpswap(long *addr, long old, long new)
{
	return cas32(addr, old, new);
}
