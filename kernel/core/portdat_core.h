/*s: portdat_core.h */

// All those structs used to be in dat.h, in 386/, but many of their fields
// were used from port/ so I've moved them here (and put the arch
// specific fields in dat_core.h)

//*****************************************************************************
// Conf
//*****************************************************************************

/*s: struct Confmem */
// memory "bank"
struct Confmem
{
    phys_addr base;
    ulong npage;
    /*s: [[Confmem]] other fields */
    kern_addr kbase; // KADDR(Confmem.base)
    kern_addr klimit; // KADDR(base+ x*BY2PG) where x <= Confmem.npage
    /*e: [[Confmem]] other fields */
};
/*e: struct Confmem */

/*s: struct Conf */
struct Conf
{
    ulong ncpu;    /* processors */
    Confmem mem[4];   /* physical memory */

    ulong nproc;    /* processes */
    /*s: [[Conf]] other fields */
    bool copymode; /* 0 is copy on write, 1 is copy on reference */
    /*x: [[Conf]] other fields */
    ulong nswap;    /* number of swap pages */
    int nswppo;   /* max # of pageouts per segment pass */
    /*x: [[Conf]] other fields */
    ulong npage;    /* total physical pages of memory */

    // kpages = npage - upages
    ulong upages;   /* user page pool */ 
    ulong nimage;   /* number of page cache image headers */
    ulong ialloc;   /* max interrupt time allocation in bytes */
    /*x: [[Conf]] other fields */
    ulong pipeqsize;  /* size in bytes of pipe queues */
    /*e: [[Conf]] other fields */
};
/*e: struct Conf */

extern Conf conf;

/*s: constant MAXCONF */
#define MAXCONF         64
/*e: constant MAXCONF */
// hash<string, string>
extern char *confname[];
extern char *confval[];
// Hashtbl.length(confname)
extern int nconf;

extern bool cpuserver; // defined in $CONF.c

char* getconf(char *name);

//*****************************************************************************
// Cpu
//*****************************************************************************

/*s: struct Label */
// =~ a jumpbuf in C, for coroutines
struct Label
{
    // or virt_addr? used also for saving context of user code?
    kern_addr sp; 
    kern_addr pc; 
};
/*e: struct Label */

/*s: struct Perf */
/*
 *  performance timers, all units in perfticks
 */
struct Perf
{
    // intr-ts? interrupt time stamp?
    ulong intrts;   /* time of last interrupt */
    ulong inintr;   /* time since last clock tick in interrupt handlers */
    ulong avg_inintr; /* avg time per clock tick in interrupt handlers */
    ulong inidle;   /* time since last clock tick in idle loop */
    ulong avg_inidle; /* avg time per clock tick in idle loop */
    ulong last;   /* value of perfticks() at last clock tick */
    ulong period;   /* perfticks() per clock tick */
};
/*e: struct Perf */

/*s: struct Cpu */
struct Cpu
{
    int cpuno;     /* physical id of processor (KNOWN TO ASSEMBLY) */
    /*s: [[Cpu]] second field */
    // must be second field at 0x04, used by splhi()
    ulong splpc;      /* pc of last caller to splhi */
    /*e: [[Cpu]] second field */
  
    // ref<Proc>
    Proc* proc;     /* current process on this processor */

    ulong ticks;      /* of the clock since boot time */

    uvlong  cpuhz;
    int cpumhz; // cpuhz / 1_000_000
    // cyclefre == cpuhz if havetsc, 0 otherwise
    uvlong  cyclefreq;    /* Frequency of user readable cycle counter */

    /*s: [[Cpu]] stat fields */
    Perf  perf;     /* performance counters */
    /*x: [[Cpu]] stat fields */
    int cs; // context switch, sched() and sleep() call
    int syscall;
    int load;
    int tlbfault;
    int tlbpurge;
    int pfault;
    int intr;
    ulong spuriousintr;
    /*e: [[Cpu]] stat fields */
    /*s: [[Cpu]] other fields */
    int ilockdepth;
    /*x: [[Cpu]] other fields */
    bool flushmmu;   /* make current proc flush it's mmu state */
    /*x: [[Cpu]] other fields */
    int lastintr; // debugging
    /*x: [[Cpu]] other fields */
    Label sched;      /* scheduler wakeup */ // address of schedinit()
    /*x: [[Cpu]] other fields */
    Proc* readied;    /* for runproc */
    /*x: [[Cpu]] other fields */
    ulong schedticks;   /* next forced context switch */
    /*e: [[Cpu]] other fields */
    struct ArchCpu;
  
    // must be at the end of the structure!
    int stack[1];
};
/*e: struct Cpu */

// ref<Cpu>, the actual Cpu is where??
extern Cpu *cpu;
// array<ref<Cpu>>, MAXCPUS is defined in 386/mem.h
extern Cpu* cpus[MAXCPUS];
/*s: macro CPUS */
#define CPUS(n)  (cpus[n])
/*e: macro CPUS */

/*s: macro up */
// up = user process
#define up (cpu->proc)
/*e: macro up */

//*****************************************************************************
// Other
//*****************************************************************************

extern char* eve;
int iseve(void);
// accessed by /dev/hostdomain, defined in auth.c
extern  char  hostdomain[];

// defined in ???
extern ulong    kerndate; 

/*e: portdat_core.h */
