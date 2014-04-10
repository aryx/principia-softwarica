#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

//todo: weird, if don't include this file, then can't declare arch in this file
// or I get some type signature mismatch
#include "io.h"

PCArch* arch;

void	(*fprestore)(FPsave*);
void	(*fpsave)(FPsave*);

int (*_pcmspecial)(char*, ISAConf*);


void
cpuidprint(void)
{
	int i;
	char buf[128];

	i = snprint(buf, sizeof buf, "cpu%d: %s%dMHz ", m->machno,
		m->machno < 10? " ": "", m->cpumhz);
	if(m->cpuidid[0])
		i += sprint(buf+i, "%12.12s ", m->cpuidid);
	seprint(buf+i, buf + sizeof buf - 1,
		"%s (cpuid: AX 0x%4.4uX DX 0x%4.4uX)\n",
		m->cpuidtype, m->cpuidax, m->cpuiddx);
	print(buf);
}


ulong
µs(void)
{
	return fastticks2us((*arch->fastclock)(nil));
}



static void
simplecycles(uvlong*x)
{
	*x = m->ticks;
}

void	(*cycles)(uvlong*) = simplecycles;

/*
 * 386 has no compare-and-swap instruction.
 * Run it with interrupts turned off instead.
 */
// used to be static, but now shared between arch.c and devarch.c
int
cmpswap386(long *addr, long old, long new)
{
	int r, s;

	s = splhi();
	if(r = (*addr == old))
		*addr = new;
	splx(s);
	return r;
}

/*
 * On a uniprocessor, you'd think that coherence could be nop,
 * but it can't.  We still need a barrier when using coherence() in
 * device drivers.
 *
 * On VMware, it's safe (and a huge win) to set this to nop.
 * Aux/vmware does this via the #P/archctl file.
 */
//now in globals.c: void (*coherence)(void) = nop;

int (*cmpswap)(long*, long, long) = cmpswap386;


// used to be static, but now shared between arch.c and devarch.c
int doi8253set = 1;

/*
 *  set next timer interrupt
 */
void
timerset(Tval x)
{
	if(doi8253set)
		(*arch->timerset)(x);
}
