/*s: init/arm/raspi2.c */
/*
 * bcm2836 (e.g.raspberry pi 2) architecture-specific stuff
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"
#include "arm.h"

#include "../port/netif.h"
#include "../port/etherif.h"

typedef struct Mbox Mbox;
typedef struct Mboxes Mboxes;

/*s: constant ARMLOCAL(arm) */
#define ARMLOCAL    (VIRTIO+IOSIZE)
/*e: constant ARMLOCAL(arm) */

Soc soc = {
    .dramsize   = 1024*MiB,
    .physio     = 0x3F000000,
    .busdram    = 0xC0000000,
    .busio      = 0x7E000000,
    .armlocal   = 0x40000000,
    .l1ptedramattrs = Cached | Buffered | L1wralloc | L1sharable,
    .l2ptedramattrs = Cached | Buffered | L2wralloc | L2sharable,
};


/*
 * Arm local regs for smp
 */
struct Mbox {
    u32int  doorbell;
    u32int  mbox1;
    u32int  mbox2;
    u32int  startcpu;
};
struct Mboxes {
    Mbox    set[4];
    Mbox    clr[4];
};

enum {
    Mboxregs    = 0x80
};

static Lock startlock[MAXCPUS + 1];

// in startv7.s
extern u32int cpidget(void);

char*
cputype2name(char *buf, int size)
{
    ulong r;

    r = cpidget();          /* main id register */
    assert((r >> 24) == 'A');
    seprint(buf, buf + size, "Cortex-A7 r%ldp%ld",
        (r >> 20) & MASK(4), r & MASK(4));
    return buf;
}

void
arch_cpuidprint(void)
{
    char name[64];

    cputype2name(name, sizeof name);
    arch_delay(50);             /* let uart catch up */
    print("cpu%d: %dMHz ARM %s\n", cpu->cpuno, cpu->cpumhz, name);
}

int
getncpus(void)
{
    int n, max;
    char *p;

    n = 4;
    if(n > MAXCPUS)
        n = MAXCPUS;
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


// in main.c
extern void cpuinit(void);
extern void machon(uint);

//TODO: cpus[] set? and cpu->cpuno??
void
cpustart(int xcpu)
{
    Mboxes *mb;

    up = nil;
    cpuinit();
    mmuinit1((void*)cpu->mmul1);

    mb = (Mboxes*)(ARMLOCAL + Mboxregs);
    mb->clr[xcpu].doorbell = 1;

    arch_trapinit();
    clockinit();
    timersinit();
    arch_cpuidprint();
    archreset();

    machon(cpu->cpuno);
    unlock(&startlock[xcpu]);

    // schedule a ready process (not necessarily the first one now)
    schedinit();

    panic("schedinit returned");
}
/*e: init/arm/raspi2.c */
