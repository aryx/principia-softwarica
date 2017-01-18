/*
 * bcm2836 (e.g.raspberry pi 2) architecture-specific stuff
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"
#include "arm.h"

#include "../port/netif.h"
#include "etherif.h"

typedef struct Mbox Mbox;
typedef struct Mboxes Mboxes;

#define	POWERREGS	(VIRTIO+0x100000)
#define ARMLOCAL	(VIRTIO+IOSIZE)

Soc soc = {
	.dramsize	= 1024*MiB,
	.physio		= 0x3F000000,
	.busdram	= 0xC0000000,
	.busio		= 0x7E000000,
	.armlocal	= 0x40000000,
	.l1ptedramattrs = Cached | Buffered | L1wralloc | L1sharable,
	.l2ptedramattrs = Cached | Buffered | L2wralloc | L2sharable,
};

enum {
	Wdogfreq	= 65536,
	Wdogtime	= 5,	/* seconds, ≤ 15 */
};

/*
 * Power management / watchdog registers
 */
enum {
	Rstc		= 0x1c>>2,
		Password	= 0x5A<<24,
		CfgMask		= 0x03<<4,
		CfgReset	= 0x02<<4,
	Rsts		= 0x20>>2,
	Wdog		= 0x24>>2,
};

/*
 * Arm local regs for smp
 */
struct Mbox {
	u32int	doorbell;
	u32int	mbox1;
	u32int	mbox2;
	u32int	startcpu;
};
struct Mboxes {
	Mbox	set[4];
	Mbox	clr[4];
};

enum {
	Mboxregs	= 0x80
};

static Lock startlock[MAXMACH + 1];

void
archreset(void)
{
	fpon();
}

void
archreboot(void)
{
	u32int *r;

	r = (u32int*)POWERREGS;
	r[Wdog] = Password | 1;
	r[Rstc] = Password | (r[Rstc] & ~CfgMask) | CfgReset;
	coherence();
	for(;;)
		;
}

static void
wdogfeed(void)
{
	u32int *r;

	r = (u32int*)POWERREGS;
	r[Wdog] = Password | (Wdogtime * Wdogfreq);
	r[Rstc] = Password | (r[Rstc] & ~CfgMask) | CfgReset;
}

void
wdogoff(void)
{
	u32int *r;

	r = (u32int*)POWERREGS;
	r[Rstc] = Password | (r[Rstc] & ~CfgMask);
}


char *
cputype2name(char *buf, int size)
{
	ulong r;

	r = cpidget();			/* main id register */
	assert((r >> 24) == 'A');
	seprint(buf, buf + size, "Cortex-A7 r%ldp%ld",
		(r >> 20) & MASK(4), r & MASK(4));
	return buf;
}

void
cpuidprint(void)
{
	char name[64];

	cputype2name(name, sizeof name);
	delay(50);				/* let uart catch up */
	print("cpu%d: %dMHz ARM %s\n", m->machno, m->cpumhz, name);
}

int
getncpus(void)
{
	int n, max;
	char *p;

	n = 4;
	if(n > MAXMACH)
		n = MAXMACH;
	p = getconf("*ncpu");
	if(p && (max = atoi(p)) > 0 && n > max)
		n = max;
	return n;
}

static int
startcpu(uint cpu)
{
	Mboxes *mb;
	int i;
	void cpureset();

	mb = (Mboxes*)(ARMLOCAL + Mboxregs);
	if(mb->clr[cpu].startcpu)
		return -1;
	mb->set[cpu].startcpu = PADDR(cpureset);
	for(i = 0; i < 1000; i++)
		if(mb->clr[cpu].startcpu == 0)
			return 0;
	mb->clr[cpu].startcpu = PADDR(cpureset);
	mb->set[cpu].doorbell = 1;
	return 0;
}

int
startcpus(uint ncpu)
{
	int i;

	for(i = 0; i < ncpu; i++)
		lock(&startlock[i]);
	cachedwbse(startlock, sizeof startlock);
	for(i = 1; i < ncpu; i++){
		if(startcpu(i) < 0)
			return i;
		lock(&startlock[i]);
		unlock(&startlock[i]);
	}
	return ncpu;
}

void
archbcm2link(void)
{
	addclock0link(wdogfeed, HZ);
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
	return (AP(0, (ap)));
}

int
cmpswap(long *addr, long old, long new)
{
	return cas((ulong*)addr, old, new);
}

void
cpustart(int cpu)
{
	Mboxes *mb;
	void machon(int);

	up = nil;
	machinit();
	mmuinit1(m->mmul1);
	mb = (Mboxes*)(ARMLOCAL + Mboxregs);
	mb->clr[cpu].doorbell = 1;
	trapinit();
	clockinit();
	timersinit();
	cpuidprint();
	archreset();
	machon(m->machno);
	unlock(&startlock[cpu]);
	schedinit();
	panic("schedinit returned");
}
