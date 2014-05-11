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
    ulong base; // phys?
    ulong npage;
  
    ulong kbase; // phys?
    ulong klimit; // phys?
};
/*e: struct Confmem */

/*s: struct Conf */
struct Conf
{
    ulong nmach;    /* processors */
    ulong nproc;    /* processes */
    Confmem mem[4];   /* physical memory */

    /*s: [[Conf]] extra fields */
    bool_ulong monitor;  /* has monitor? */

    ulong npage;    /* total physical pages of memory */
    ulong upages;   /* user page pool */
    ulong nimage;   /* number of page cache image headers */
    ulong nswap;    /* number of swap pages */

    int nswppo;   /* max # of pageouts per segment pass */

    bool_ulong copymode; /* 0 is copy on write, 1 is copy on reference */
    ulong ialloc;   /* max interrupt time allocation in bytes */
    int nuart;    /* number of uart devices */
    /*x: [[Conf]] extra fields */
    ulong pipeqsize;  /* size in bytes of pipe queues */
    /*e: [[Conf]] extra fields */
  
    struct ArchConf;
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
// Mach
//*****************************************************************************

/*s: struct Label */
// =~ a jumpbuf in C, for coroutines
struct Label
{
    ulong sp; // virt_addr?
    ulong pc; // virt_addr?
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

/*s: struct Mach */
struct Mach
{
    int machno;     /* physical id of processor (KNOWN TO ASSEMBLY) */
    /*s: [[Mach]] second field */
        // must be second field at 0x04, used by splhi()
        ulong splpc;      /* pc of last caller to splhi */
    /*e: [[Mach]] second field */
  
    // ref<Proc>
    Proc* proc;     /* current process on this processor */

    /*s: [[Mach]] stat fields */
    /* stats */
    int tlbfault;
    int tlbpurge;
    int pfault;
    int cs; // context switch?
    int syscall;
    int load;
    int intr;
    /*e: [[Mach]] stat fields */

    /*s: [[Mach]] other fields */
    int ilockdepth;
    /*x: [[Mach]] other fields */
    ulong ticks;      /* of the clock since boot time */
    Label sched;      /* scheduler wakeup */
    Lock  alarmlock;    /* access to alarm list */
    void* alarm;      /* alarms bound to this clock */

    Proc* readied;    /* for runproc */
    ulong schedticks;   /* next forced context switch */

    int flushmmu;   /* make current proc flush it's mmu state */

    ulong spuriousintr;
    int lastintr;
    Perf  perf;     /* performance counters */

    int cpumhz;
    uvlong  cpuhz;
    uvlong  cyclefreq;    /* Frequency of user readable cycle counter */
    /*e: [[Mach]] other fields */

    struct ArchMach;
  
    // must be at the end of the structure!
    int stack[1];
};
/*e: struct Mach */

// ref<Mach>, the actual Mach is where??
extern Mach *m;
// array<ref<Mach>>, MAXMACH is defined in 386/mem.h
extern Mach* machp[MAXMACH];
/*s: macro MACHP */
#define MACHP(n)  (machp[n])
/*e: macro MACHP */

/*s: macro up */
// up = user process, MACHADDR is defined in 386/mem.h
#define up  (((Mach*)MACHADDR)->externup)
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

// used to be in devcons.c, but used also by edf.c
extern int panicking;
/*e: portdat_core.h */
