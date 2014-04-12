
// All those structs used to be in dat.h, in 386/, but many of their fields
// were used from port/ so I've moved them here (and put the arch
// specific fields in dat_core.h)

// less: have a conf.c, mach.c?

//*****************************************************************************
// Conf
//*****************************************************************************

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
  // in bcm/ it's [1], important? 
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

  struct ArchConf;
};


extern Conf conf;

// hash<string, string>
#define MAXCONF         64
extern char *confname[];
extern char *confval[];
// Hashtbl.length(confname)
extern int nconf;
extern bool cpuserver; // defined in conf/pcf.c

char* getconf(char *name);

//*****************************************************************************
// Mach
//*****************************************************************************

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

  struct ArchMach;

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

  int stack[1];
};

// ref<Mach>, the actual Mach is where??
extern Mach *m;
/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// array<ref<Mach>>, MAXMACH is defined in 386/mem.h
extern Mach* machp[MAXMACH];
#define MACHP(n)  (machp[n])

// up = user process, MACHADDR is defined in 386/mem.h
//TODO: mv in 386/ TODO: why not m->externup? m is not valid?
#define up  (((Mach*)MACHADDR)->externup)

//*****************************************************************************
// Other
//*****************************************************************************

extern char* eve;
int iseve(void);

extern ulong	kerndate; // defined in ???

// used to be in devcons.c, but used also by edf.c
extern int panicking;
