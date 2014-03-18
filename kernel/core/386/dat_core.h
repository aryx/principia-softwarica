
// Put here only core stuff that are used in port code.
// Put in xxx/386/ (e.g. processes/386/dat_processes.h) data structures 
// that are used only in 386/ code

struct Label
{
	ulong	sp;
	ulong	pc;
};

struct Lock
{
	ulong	key;
	ulong	sr;
	ulong	pc;
	Proc	*p;
	Mach	*m;
	ushort	isilock;
	long	lockcycles;
};



/*
 *  performance timers, all units in perfticks
 */
//used to be in portdat.h, but needed by Mach
struct Perf
{
	ulong	intrts;		/* time of last interrupt */
	ulong	inintr;		/* time since last clock tick in interrupt handlers */
	ulong	avg_inintr;	/* avg time per clock tick in interrupt handlers */
	ulong	inidle;		/* time since last clock tick in idle loop */
	ulong	avg_inidle;	/* avg time per clock tick in idle loop */
	ulong	last;		/* value of perfticks() at last clock tick */
	ulong	period;		/* perfticks() per clock tick */
};






/*
 * FPsave.status
 */
enum
{
	/* this is a state */
	FPinit=		0,
	FPactive=	1,
	FPinactive=	2,

	/* the following is a bit that can be or'd into the state */
	FPillegal=	0x100,
};

struct	FPstate			/* x87 fpu state */
{
	ushort	control;
	ushort	r1;
	ushort	status;
	ushort	r2;
	ushort	tag;
	ushort	r3;
	ulong	pc;
	ushort	selector;
	ushort	r4;
	ulong	operand;
	ushort	oselector;
	ushort	r5;
	uchar	regs[80];	/* floating point registers */
};

struct	FPssestate		/* SSE fp state */
{
	ushort	fcw;		/* control */
	ushort	fsw;		/* status */
	ushort	ftw;		/* tag */
	ushort	fop;		/* opcode */
	ulong	fpuip;		/* pc */
	ushort	cs;		/* pc segment */
	ushort	r1;		/* reserved */
	ulong	fpudp;		/* data pointer */
	ushort	ds;		/* data pointer segment */
	ushort	r2;
	ulong	mxcsr;		/* MXCSR register state */
	ulong	mxcsr_mask;	/* MXCSR mask register */
	uchar	xregs[480];	/* extended registers */
};

struct	SFPssestate		/* SSE fp state with alignment slop */
{
	FPssestate;
	uchar	alignpad[FPalign]; /* slop to allow copying to aligned addr */
	ulong	magic;		/* debugging: check for overrun */
};

/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
union FPsave {
	FPstate;
	SFPssestate;
};








struct Tss {
	ulong	link;			/* link (old TSS selector) */
	ulong	esp0;			/* privilege level 0 stack pointer */
	ulong	ss0;			/* privilege level 0 stack selector */
	ulong	esp1;			/* privilege level 1 stack pointer */
	ulong	ss1;			/* privilege level 1 stack selector */
	ulong	esp2;			/* privilege level 2 stack pointer */
	ulong	ss2;			/* privilege level 2 stack selector */
	ulong	xcr3;			/* page directory base register - not used because we don't use trap gates */
	ulong	eip;			/* instruction pointer */
	ulong	eflags;			/* flags register */
	ulong	eax;			/* general registers */
	ulong 	ecx;
	ulong	edx;
	ulong	ebx;
	ulong	esp;
	ulong	ebp;
	ulong	esi;
	ulong	edi;
	ulong	es;			/* segment selectors */
	ulong	cs;
	ulong	ss;
	ulong	ds;
	ulong	fs;
	ulong	gs;
	ulong	ldt;			/* selector for task's LDT */
	ulong	iomap;			/* I/O map base address + T-bit */
};

struct Segdesc
{
	ulong	d0;
	ulong	d1;
};






struct Mach
{
	int	machno;			/* physical id of processor (KNOWN TO ASSEMBLY) */
	ulong	splpc;			/* pc of last caller to splhi */

	ulong*	pdb;			/* page directory base for this processor (va) */
	Tss*	tss;			/* tss for this processor */
	Segdesc	*gdt;			/* gdt for this processor */

	Proc*	proc;			/* current process on this processor */
	Proc*	externup;		/* extern register Proc *up */

	Page*	pdbpool;
	int	pdbcnt;

	ulong	ticks;			/* of the clock since boot time */
	Label	sched;			/* scheduler wakeup */
	Lock	alarmlock;		/* access to alarm list */
	void*	alarm;			/* alarms bound to this clock */
	int	inclockintr;

	Proc*	readied;		/* for runproc */
	ulong	schedticks;		/* next forced context switch */

	int	tlbfault;
	int	tlbpurge;
	int	pfault;
	int	cs;
	int	syscall;
	int	load;
	int	intr;
	int	flushmmu;		/* make current proc flush it's mmu state */
	int	ilockdepth;
	Perf	perf;			/* performance counters */

	ulong	spuriousintr;
	int	lastintr;

	int	loopconst;

	Lock	apictimerlock;
	int	cpumhz;
	uvlong	cyclefreq;		/* Frequency of user readable cycle counter */
	uvlong	cpuhz;
	int	cpuidax;
	int	cpuiddx;
	char	cpuidid[16];
	char*	cpuidtype;
	int	havetsc;
	int	havepge;
	uvlong	tscticks;
	int	pdballoc;
	int	pdbfree;
	FPsave	*fpsavalign;

	vlong	mtrrcap;
	vlong	mtrrdef;
	vlong	mtrrfix[11];
	vlong	mtrrvar[32];		/* 256 max. */

	int	stack[1];
};



/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// MAXMACH is defined in 386/mem.h
extern Mach* machp[MAXMACH];

#define	MACHP(n)	(machp[n])

// MACHADDR is defined in 386/mem.h
#define up	(((Mach*)MACHADDR)->externup)


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


struct Active
{
	Lock;
	int	machs;			/* bitmap of active CPUs */
	int	exiting;		/* shutdown */
	int	ispanic;		/* shutdown in response to a panic */
	int	thunderbirdsarego;	/* lets the added processors continue to schedinit */
	int	rebooting;		/* just idle cpus > 0 */
};

extern struct Active active;
