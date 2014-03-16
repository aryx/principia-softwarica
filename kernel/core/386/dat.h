#include "dat_forward.h"

// reference things defined in portdat_core.h: Proc
#include "dat_core.h"

// used in portdat.h
#define KMESGSIZE (256*1024)	/* lots, for acpi debugging */
#define STAGESIZE 2048

#define MAXSYSARG	5	/* for mount(fd, afd, mpt, flag, arg) */

// define things used in portdat_core.h Proc
/*
 *  MMU stuff in proc
 */
#define NCOLOR 1
struct PMMU
{
	Page*	mmupdb;			/* page directory base */
	Page*	mmufree;		/* unused page table pages */
	Page*	mmuused;		/* used page table pages */
	Page*	kmaptable;		/* page table used by kmap */
	uint	lastkmap;		/* last entry used by kmap */
	int	nkmap;			/* number of current kmaps */
};

/*
 *  things saved in the Proc structure during a notify
 */
struct Notsave
{
	ulong	svflags;
	ulong	svcs;
	ulong	svss;
};

#include "../port/portdat.h"


/*
 *  parameters for sysproc.c
 */
#define AOUT_MAGIC	(I_MAGIC)


struct Confmem
{
	ulong	base;
	ulong	npage;
	ulong	kbase;
	ulong	klimit;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	ulong	monitor;	/* has monitor? */
	Confmem	mem[4];		/* physical memory */
	ulong	npage;		/* total physical pages of memory */
	ulong	upages;		/* user page pool */
	ulong	nimage;		/* number of page cache image headers */
	ulong	nswap;		/* number of swap pages */
	int	nswppo;		/* max # of pageouts per segment pass */
	ulong	base0;		/* base of bank 0 */
	ulong	base1;		/* base of bank 1 */
	ulong	copymode;	/* 0 is copy on write, 1 is copy on reference */
	ulong	ialloc;		/* max interrupt time allocation in bytes */
	ulong	pipeqsize;	/* size in bytes of pipe queues */
	int	nuart;		/* number of uart devices */
};



/*
 * KMap the structure doesn't exist, but the functions do.
 */
typedef struct KMap		KMap;
#define	VA(k)		((void*)(k))
KMap*	kmap(Page*);
void	kunmap(KMap*);

struct
{
	Lock;
	int	machs;			/* bitmap of active CPUs */
	int	exiting;		/* shutdown */
	int	ispanic;		/* shutdown in response to a panic */
	int	thunderbirdsarego;	/* lets the added processors continue to schedinit */
	int	rebooting;		/* just idle cpus > 0 */
} active;

/*
 *  routines for things outside the PC model, like power management
 */
struct PCArch
{
	char*	id;
	int	(*ident)(void);		/* this should be in the model */
	void	(*reset)(void);		/* this should be in the model */
	int	(*serialpower)(int);	/* 1 == on, 0 == off */
	int	(*modempower)(int);	/* 1 == on, 0 == off */

	void	(*intrinit)(void);
	int	(*intrenable)(Vctl*);
	int	(*intrvecno)(int);
	int	(*intrdisable)(int);
	void	(*introff)(void);
	void	(*intron)(void);

	void	(*clockenable)(void);
	uvlong	(*fastclock)(uvlong*);
	void	(*timerset)(uvlong);

	void	(*resetothers)(void);	/* put other cpus into reset */
};

/* cpuid instruction result register bits */
enum {
	/* dx */
	Fpuonchip = 1<<0,
	Vmex	= 1<<1,		/* virtual-mode extensions */
	Pse	= 1<<3,		/* page size extensions */
	Tsc	= 1<<4,		/* time-stamp counter */
	Cpumsr	= 1<<5,		/* model-specific registers, rdmsr/wrmsr */
	Pae	= 1<<6,		/* physical-addr extensions */
	Mce	= 1<<7,		/* machine-check exception */
	Cmpxchg8b = 1<<8,
	Cpuapic	= 1<<9,
	Mtrr	= 1<<12,	/* memory-type range regs.  */
	Pge	= 1<<13,	/* page global extension */
	Pse2	= 1<<17,	/* more page size extensions */
	Clflush = 1<<19,
	Mmx	= 1<<23,
	Fxsr	= 1<<24,	/* have SSE FXSAVE/FXRSTOR */
	Sse	= 1<<25,	/* thus sfence instr. */
	Sse2	= 1<<26,	/* thus mfence & lfence instr.s */
};

/*
 *  a parsed plan9.ini line
 */
#define NISAOPT		8

struct ISAConf {
	char	*type;
	ulong	port;
	int	irq;
	ulong	dma;
	ulong	mem;
	ulong	size;
	ulong	freq;

	int	nopt;
	char	*opt[NISAOPT];
};

extern PCArch	*arch;			/* PC architecture */


/*
 *  hardware info about a device
 */
struct Devport {
	ulong	port;	
	int	size;
};

struct DevConf
{
	ulong	intnum;			/* interrupt number */
	char	*type;			/* card type, malloced */
	int	nports;			/* Number of ports */
	Devport	*ports;			/* The ports themselves */
};

struct BIOS32ci {		/* BIOS32 Calling Interface */
	u32int	eax;
	u32int	ebx;
	u32int	ecx;
	u32int	edx;
	u32int	esi;
	u32int	edi;
};
