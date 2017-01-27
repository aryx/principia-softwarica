/*
 * bcm2835 (e.g. original raspberry pi) architecture-specific stuff
 */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"
#include "arm.h"

#include "../port/netif.h"
#include "../port/etherif.h"

Soc soc = {
	.dramsize	= 512*MiB,
	.physio		= 0x20000000,
	.busdram	= 0x40000000,
	.busio		= 0x7E000000,
	.armlocal	= 0,

	.l1ptedramattrs = Cached | Buffered,
	.l2ptedramattrs = Cached | Buffered,
};

	
char*
cputype2name(char *buf, int size)
{
	seprint(buf, buf + size, "1176JZF-S");
	return buf;
}

void
arch_cpuidprint(void)
{
	char name[64];

	cputype2name(name, sizeof name);
	arch_delay(50);				/* let uart catch up */
	print("cpu%d: %dMHz ARM %s\n", cpu->cpuno, cpu->cpumhz, name);
}

int
getncpus(void)
{
	return 1;
}

int
startcpus(uint)
{
	return 1;
}


int
archether(unsigned ctlrno, Ether *ether)
{
	ether->type = "usb";
	ether->ctlrno = ctlrno;
	ether->irq = -1;
	ether->nopt = 0;
	return 1;
}

int
l2ap(int ap)
{
	return (AP(3, (ap))|AP(2, (ap))|AP(1, (ap))|AP(0, (ap)));
}

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

int
cas32(void* addr, u32int old, u32int new)
{
	int r, s;

	s = arch_splhi();
	if(r = (*(u32int*)addr == old))
		*(u32int*)addr = new;
	arch_splx(s);
	if (r)
		arch_coherence();
	return r;
}

int
arch_cmpswap(long *addr, long old, long new)
{
	return cas32(addr, old, new);
}
