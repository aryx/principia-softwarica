
// all those structs used to be in 386/, but many of their fields were
// used from port/ code so I've moved them here.

// memory "bank"
struct Confmem
{
  ulong base; // phys?
  ulong npage;

  ulong kbase; // phys?
  ulong klimit; // phys?
};

struct Conf
{
  ulong nmach;    /* processors */
  ulong nproc;    /* processes */
  Confmem mem[4];   /* physical memory */

  bool_ulong monitor;  /* has monitor? */

  ulong npage;    /* total physical pages of memory */
  ulong upages;   /* user page pool */
  ulong nimage;   /* number of page cache image headers */
  ulong nswap;    /* number of swap pages */

  int nswppo;   /* max # of pageouts per segment pass */

  bool_ulong copymode; /* 0 is copy on write, 1 is copy on reference */
  ulong ialloc;   /* max interrupt time allocation in bytes */
  ulong pipeqsize;  /* size in bytes of pipe queues */
  int nuart;    /* number of uart devices */

  // 386/ specific?
  ulong base0;    /* base of bank 0 */
  ulong base1;    /* base of bank 1 */
};


// =~ a jumpbuf in C, for coroutines
struct Label
{
  ulong sp; // virt_addr?
  ulong pc; // virt_addr?
};

/*
 *  performance timers, all units in perfticks
 */
struct Perf
{
  ulong intrts;   /* time of last interrupt */
  ulong inintr;   /* time since last clock tick in interrupt handlers */
  ulong avg_inintr; /* avg time per clock tick in interrupt handlers */
  ulong inidle;   /* time since last clock tick in idle loop */
  ulong avg_inidle; /* avg time per clock tick in idle loop */
  ulong last;   /* value of perfticks() at last clock tick */
  ulong period;   /* perfticks() per clock tick */
};



// TODO have a MachArch
struct Mach
{
  int machno;     /* physical id of processor (KNOWN TO ASSEMBLY) */
  // must be second field at 0x04, used by splhi()
  ulong splpc;      /* pc of last caller to splhi */

  Proc* proc;     /* current process on this processor */

  ulong ticks;      /* of the clock since boot time */
  Label sched;      /* scheduler wakeup */
  Lock  alarmlock;    /* access to alarm list */
  void* alarm;      /* alarms bound to this clock */

  Proc* readied;    /* for runproc */
  ulong schedticks;   /* next forced context switch */

  int flushmmu;   /* make current proc flush it's mmu state */

  // 386 specific part
  // TODO: have a MMMU like in bcm/
  ulong*  pdb;      /* page directory base for this processor (va) */
  Tss*  tss;      /* tss for this processor */
  Segdesc *gdt;     /* gdt for this processor */
  Proc* externup;   /* extern register Proc *up */
  Page* pdbpool;
  int pdbcnt;
  int inclockintr;

	/* stats */
  int tlbfault;
  int tlbpurge;
  int pfault;
  int cs;
  int syscall;
  int load;
  int intr;


  ulong spuriousintr;
  int lastintr;
  int ilockdepth;
  Perf  perf;     /* performance counters */

  int cpumhz;
  uvlong  cpuhz;
  uvlong  cyclefreq;    /* Frequency of user readable cycle counter */

  // 386/ specific
  int loopconst;
  Lock  apictimerlock;
  int cpuidax;
  int cpuiddx;
  char  cpuidid[16];
  char* cpuidtype;
  int havetsc;
  int havepge;
  uvlong tscticks;
  int pdballoc;
  int pdbfree;
  FPsave *fpsavalign;
  vlong mtrrcap;
  vlong mtrrdef;
  vlong mtrrfix[11];
  vlong mtrrvar[32];    /* 256 max. */



  int stack[1];
};
