/*s: portdat_processes.h */

// in lib.h: Waitmsg, ERRMAX
// see also ArchProcMMU, MAXSYSARG in 386/

//*****************************************************************************
// Proc components
//*****************************************************************************
// All the ref<Proc> here are references to Proc in the array<Proc> of
// Procalloc.arena (pool allocator)

//--------------------------------------------------------------------
// State
//--------------------------------------------------------------------

// TODO: state transition diagram
/*s: enum procstate */
/* Process states, Proc.state */
enum procstate
{
    Dead = 0,
    Running,
    /*s: enum procstate cases */
    Queueing, // see qlock()
    /*x: enum procstate cases */
    QueueingR, // see rlock()
    /*x: enum procstate cases */
    QueueingW, // see wlock()
    /*x: enum procstate cases */
    Scheding,
    /*x: enum procstate cases */
    Moribund,
    /*x: enum procstate cases */
    Broken,
    /*x: enum procstate cases */
    Ready,
    /*x: enum procstate cases */
    Wakeme,
    /*x: enum procstate cases */
    Rendezvous,
    /*x: enum procstate cases */
    Stopped,
    /*x: enum procstate cases */
    Waitrelease, // for real-time scheduling
    /*e: enum procstate cases */
};
/*e: enum procstate */

// hash<enum<procstate>, string>, to debug
extern  char* statename[];

//--------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------
/*s: enum procseg */
/*
 *  process memory segments - NSEG always last !
 */
enum procseg
{
    SSEG, TSEG, DSEG, BSEG, // Stack, Text, Data, Bss
    ESEG, // E = Extra (used for temporary stack segment),
    _SEG0, _SEG1, _SEG2, _SEG3, _SEG4, // free slots for for segattach
    NSEG // to count, see Proc.seg array
};
/*e: enum procseg */

//--------------------------------------------------------------------
// Files
//--------------------------------------------------------------------

enum
{
   /*s: constant DELTAFD */
       DELTAFD = 20    /* incremental increase in Fgrp.fd's */
   /*e: constant DELTAFD */
};

/*s: struct Fgrp */
struct Fgrp
{
    // array<ref_counted<Chan>, smalloc'ed?
    Chan  **fd;
    // nelem(fd) ?
    int nfd;      /* number allocated */
    int maxfd;      /* highest fd in use */
  
    int exceed;     /* debugging */
  
    // extra
    Ref;
};
/*e: struct Fgrp */


/*s: function MOUNTH */
enum
{
    MNTLOG  = 5,
    MNTHASH = 1<<MNTLOG,  /* Hash to walk mount table */
};
#define MOUNTH(p,qid) ((p)->mnthash[(qid).path&((1<<MNTLOG)-1)])
/*e: function MOUNTH */

/*s: struct Pgrp */
// Namespace process group
struct Pgrp
{
    // hash<qid.path, list<ref<Mhead> (next = Mhead.next)>
    Mhead *mnthash[MNTHASH];
    ulong pgrpid;
    bool noattach;
  
    // extra
    Ref;        /* also used as a lock when mounting */
    QLock debug;      /* single access via devproc.c */
    RWlock  ns;     /* Namespace n read/one write lock */
};
/*e: struct Pgrp */

//--------------------------------------------------------------------
// System call
//--------------------------------------------------------------------

/*s: struct Sargs */
// syscall arguments copied from user stack
struct Sargs
{
    ulong args[MAXSYSARG];
};
/*e: struct Sargs */

//--------------------------------------------------------------------
// Notes
//--------------------------------------------------------------------

enum {
    /*s: constant NNOTE */
        NNOTE = 5,
    /*e: constant NNOTE */
};

/*s: enum notekind */
enum notekind
{
    NUser,        /* note provided externally */
    NExit,        /* deliver note quietly */
    NDebug,       /* print debug message */
};
/*e: enum notekind */

/*s: struct Note */
// a kind of unix signal
struct Note
{
    char  msg[ERRMAX];
    // enum<notekind>
    int flag;     /* whether system posted it */
};
/*e: struct Note */
extern Counter  noteidalloc;

//--------------------------------------------------------------------
// Process children waiting
//--------------------------------------------------------------------

/*s: struct Waitq */
// essentially a stack<ref_own<Waitmsg>>
struct Waitq
{
    Waitmsg w;
  
    // extra
    // list<ref_own<Waitq>> Proc.waitq
    Waitq *next;
};
/*e: struct Waitq */

//--------------------------------------------------------------------
// Synchronization (Rendez vous)
//--------------------------------------------------------------------

/*s: function REND */
enum
{
    RENDLOG = 5,
    RENDHASH =  1<<RENDLOG, /* Hash to lookup rendezvous tags */
};
#define REND(p,s) ((p)->rendhash[(s)&((1<<RENDLOG)-1)])
/*e: function REND */

/*s: struct Rgrp */
struct Rgrp
{
    // hash<??, list<ref<Proc>>>
    Proc  *rendhash[RENDHASH];  /* Rendezvous tag hash */
  
    // extra
    Ref;        /* the Ref's lock is also the Rgrp's lock */
};
/*e: struct Rgrp */

//--------------------------------------------------------------------
// Alarms, timers
//--------------------------------------------------------------------

/*s: enum timermode */
/*
 * fasttick timer interrupts
 */
enum timermode 
{
    Trelative,  /* timer programmed in ns from now */
    Tperiodic,  /* periodic timer, period in ns */
};
/*e: enum timermode */

/*s: type Txxx */
typedef vlong   Tval; // ticks
typedef vlong   Tnano; // nanoseconds
typedef vlong   Tmicro; // microseconds
typedef int     Tms; // milliseconds
typedef vlong   Tsec; // seconds
/*e: type Txxx */

/*s: struct Timer */
struct Timer
{
    /* Public interface */
    // enum<timermode>
    int tmode;    /* See above */
    Tnano tns;    /* meaning defined by mode */ //nanosecond
    void  (*tf)(Ureg*, Timer*);
    void  *ta;
  
    /* Internal */
    Lock;
    Tval  tticks;   /* tns converted to ticks */
    Tval  twhen;    /* ns represented in fastticks */

    /*s: [[Timer extra fields */
    // list<Timer> of Timers.head
    Timer *tnext;
    // ref<list<Timer>> Timers.head
    Timers  *tt;    /* Timers queue this timer runs on */
    /*e: [[Timer extra fields */
    };
/*e: struct Timer */

// was in clock.c
/*s: struct Timers */
struct Timers
{
    // list<Timer> (next = Timer.tnext)
    Timer *head;
    // extra
    Lock;
};
/*e: struct Timers */

//--------------------------------------------------------------------
// Scheduling
//--------------------------------------------------------------------

enum {
    /*s: constant Npriq */
    Npriq   = 20,   /* number of scheduler priority levels */
    /*e: constant Npriq */
    /*s: constant Nrq */
    Nrq   = Npriq+2,  /* number of priority levels including real time */
    /*e: constant Nrq */
};

/*s: enum priority */
enum priority 
{
    PriNormal = 10,   /* base priority for normal processes */
    PriKproc  = 13,   /* base priority for kernel processes */
    PriRoot   = 13,   /* base priority for root processes */

    /*s: constants for real-time priority */
        PriRelease  = Npriq,  /* released edf processes */
        PriEdf    = Npriq+1,  /* active edf processes */
        PriExtra  = Npriq-1,  /* edf processes at high best-effort pri */
    /*e: constants for real-time priority */
};
/*e: enum priority */

/*s: enum edfflags */
enum edfflags 
{
    /* Edf.flags field */
    Admitted    = 0x01,
    Sporadic    = 0x02,
    Yieldonblock    = 0x04,
    Sendnotes   = 0x08,
    Deadline    = 0x10,
    Yield     = 0x20,
    Extratime   = 0x40,
};
/*e: enum edfflags */

/*s: struct Edf */
struct Edf {
    /* All times in µs */
    /* time intervals */
    long    D;    /* Deadline */
    long    Delta;    /* Inherited deadline */
    long    T;    /* period */
    long    C;    /* Cost */
    long    S;    /* Slice: time remaining in this period */
    /* times (only low-order bits of absolute time) */
    long    r;    /* (this) release time */
    long    d;    /* (this) deadline */
    long    t;    /* Start of next period, t += T at release */
    long    s;    /* Time at which this proc was last scheduled */
  
    /* for schedulability testing */
    long    testDelta;
    int   testtype; /* Release or Deadline */
    long    testtime;
    Proc    *testnext;
  
    /* other */
    // set<enum<edfflags>>
    ushort    flags;
  
    Timer;
  
    /* Stats */
    long    edfused;
    long    extraused;
    long    aged;
    ulong   periods;
    ulong   missed;
};
/*e: struct Edf */

//--------------------------------------------------------------------
// Error managment
//--------------------------------------------------------------------
enum {
    /*s: constant NERR */
        NERR = 64,
    /*e: constant NERR */
};

//--------------------------------------------------------------------
// Stats, profiling
//--------------------------------------------------------------------
/*s: enum proctimer */
enum proctime 
{
    TUser = 0,    /* Proc.time */
    TSys,
    TReal,

    // accumulates also the time of its children
    TCUser, 
    TCSys, 
    TCReal,
};
/*e: enum proctimer */

//--------------------------------------------------------------------
// Debugger
//--------------------------------------------------------------------

/*s: enum procctl */
enum procctl
{
    Proc_nothing = 0,
    Proc_stopme,
    /*s: enum procctl cases */
        Proc_exitme,
    /*x: enum procctl cases */
        Proc_tracesyscall,
    /*x: enum procctl cases */
        Proc_traceme,
    /*x: enum procctl cases */
        Proc_exitbig,
    /*e: enum procctl cases */
};
/*e: enum procctl */

//--------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------

/*s: enum fpsavestatus */
/*
 * FPsave.status
 */
enum fpsavestatus
{
    /* this is a state */
    FPinit=   0,
    FPactive= 1,
    FPinactive= 2,
  
    /* the following is a bit that can be or'd into the state */
    FPillegal=  0x100,
};
/*e: enum fpsavestatus */

//*****************************************************************************
// Proc, the big one
//*****************************************************************************

/*s: struct Proc */
struct Proc
{
//--------------------------------------------------------------------
// Assembly requirements, Low level, have to be first
//--------------------------------------------------------------------
    /*s: [[Proc]] assembly fields */
    Label sched;    /* known to l.s */
    char  *kstack;  /* known to l.s */
    /*e: [[Proc]] assembly fields */
//--------------------------------------------------------------------
// State
//--------------------------------------------------------------------
    /*s: [[Proc]] state fields */
    ulong pid;
    ulong parentpid;

    // enum<procstate> 
    int state; // Dead, Queuing, etc, (used by /proc/#/status if psstate==nil)

    // some debugging information, e.g. "New", "PageOut", or name of syscall
    char  *psstate; /* used by /proc/#/status */
    bool insyscall; // true when process inside a syscall

    // e.g. "*init*", or name of executable
    char  *text;
    // e.g.. "eve" (no uid/gid in plan9, because of its distributed nature?)
    char  *user;
    /*x: [[Proc]] state fields */
    char  *args;
    int nargs;    /* number of bytes of args */
    /*e: [[Proc]] state fields */
//--------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------
    /*s: [[Proc]] memory fields */
    // hash<enum<procseg>, option<ref_own<Segment>>>, elt smalloc'ed? ref_counted?
    Segment *seg[NSEG];
    QLock seglock;  /* locked whenever seg[] changes */
    /*x: [[Proc]] memory fields */
    bool newtlb;   /* Pager has changed my pte's, I must flush */
    /*x: [[Proc]] memory fields */
    bool noswap;   /* process is not swappable */
    /*e: [[Proc]] memory fields */

    struct ArchProcMMU;
//--------------------------------------------------------------------
// Scheduling
//--------------------------------------------------------------------
    /*s: [[Proc]] scheduling fields */
    // enum<priority>
    ulong priority; /* priority level */

    ulong basepri;  /* base priority level */
    bool fixedpri; /* priority level doesn't change */
    /*x: [[Proc]] scheduling fields */
    // option<ref<Cpu>>, null when not associated to a processor
    Cpu  *cpu;    /* processor running this proc */
    /*x: [[Proc]] scheduling fields */
    Cpu *lastcpu;    /* processor this process last ran on */
    /*x: [[Proc]] scheduling fields */
    ulong lastupdate; // dimension?? ticks * Scaling;
    ulong cpuavg;    /* cpu average */
    /*x: [[Proc]] scheduling fields */
    ulong delaysched;
    /*x: [[Proc]] scheduling fields */
    bool preempted;  /* true if this process hasn't finished the interrupt
           *  that last preempted it
           */
    /*x: [[Proc]] scheduling fields */
    Cpu  *wired;
    /*x: [[Proc]] scheduling fields */
    /*s: [[Proc]] optional [[edf]] field for real-time scheduling */
    // option<ref_own?<edf>>
    Edf *edf; /* if non-null, real-time proc, edf contains scheduling params */
    /*e: [[Proc]] optional [[edf]] field for real-time scheduling */
    /*x: [[Proc]] scheduling fields */
    ulong readytime;  /* time process came ready */
    /*e: [[Proc]] scheduling fields */
//--------------------------------------------------------------------
// Files
//--------------------------------------------------------------------
    /*s: [[Proc]] files fields */
    // ref<Chan>
    Chan  *slash; // The root! used by namec()
    // ref_counted<Chan>
    Chan  *dot; // The current directory
    /*x: [[Proc]] files fields */
    // ref_counted<fgrp>
    Fgrp  *fgrp;    /* File descriptor group */
    /*x: [[Proc]] files fields */
    Fgrp  *closingfgrp; /* used during teardown */
    /*x: [[Proc]] files fields */
    // ref_counted<pgrp>
    Pgrp  *pgrp;    /* Process group for namespace */
    /*e: [[Proc]] files fields */
//--------------------------------------------------------------------
// Notes
//--------------------------------------------------------------------
    /*s: [[Proc]] notes fields */
    Note  note[NNOTE];
    short nnote;

    int (*notify)(void*, char*);
    bool_ushort notified; /* sysnoted is due */

    ulong noteid;   /* Equivalent of note group */

    bool notepending;  /* note issued but not acted on */

    Note  lastnote;

    void  *ureg;    /* User registers for notes */
    /*e: [[Proc]] notes fields */
//--------------------------------------------------------------------
// Process hierarchy
//--------------------------------------------------------------------
    /*s: [[Proc]] hierarchy fields */
    Proc  *parent;
    int nchild;   /* Number of living children */
    /*x: [[Proc]] hierarchy fields */
    // list<ref_own<Waitq>>> =~ list<ref_own<Waitmsg>>
    Waitq *waitq;   /* Exited processes wait children */
    int nwait;    /* Number of uncollected wait records */ // len(waitq)
    Lock  exl;    /* Lock count and waitq */
    /*x: [[Proc]] hierarchy fields */
    Rendez  waitr;    /* Place to hang out in wait */
    /*x: [[Proc]] hierarchy fields */
    QLock qwaitr;
    /*e: [[Proc]] hierarchy fields */
//--------------------------------------------------------------------
// Synchronization
//--------------------------------------------------------------------
    /*s: [[Proc]] synchronization fields */
    // As long as the current process hold spinlocks (to kernel data structures),
    // we will not schedule another process in unlock(); only the last unlock
    // will eventually cause a rescheduling.
    Ref nlocks;   /* number of locks held by proc */
    /*x: [[Proc]] synchronization fields */
    // option<ref<Rendez>>, can point to waitr, freememr, sleepr, etc
    Rendez  *r;   /* rendezvous point slept on */
    Lock  rlock;    /* sync sleep/wakeup with postnote */
    /*x: [[Proc]] synchronization fields */
    Rendez  sleepr;    /* place for syssleep/debug/tsleep */
    /*x: [[Proc]] synchronization fields */
    Rgrp  *rgrp;    /* Rendez group */

    uintptr rendtag;  /* Tag for rendezvous */
    uintptr rendval;  /* Value for rendezvous */
    //??
    Proc  *rendhash;  /* Hash list for tag values */
    /*e: [[Proc]] synchronization fields */
//--------------------------------------------------------------------
// Error managment
//--------------------------------------------------------------------
    /*s: [[Proc]] error managment fields */
    // array<Label>, error labels, poor's man exceptions in C
    Label errlab[NERR];
    // length(errlab) used.
    int nerrlab;

    // ref<string> point to errbuf0 or to syserrstr (which points to errbuf1)
    char  *errstr;  /* reason we're unwinding the error stack, errbuf1 or 0 */
    char  errbuf0[ERRMAX];
    char  errbuf1[ERRMAX];
    /*x: [[Proc]] error managment fields */
    char  *syserrstr; /* last error from a system call, errbuf0 or 1 */
    /*e: [[Proc]] error managment fields */

//--------------------------------------------------------------------
// Stats, profiling
//--------------------------------------------------------------------
    /*s: [[Proc]] stats and profiling fields */
    // hash<enum<proctime>, ulong>
    ulong time[6];  /* User, Sys, Real; child U, S, R */
    /*x: [[Proc]] stats and profiling fields */
    uvlong  kentry;   /* Kernel entry time stamp (for profiling) */
    /*
     * pcycles: cycles spent in this process (updated on procsave/restore)
     * when this is the current proc and we're in the kernel
     * (procrestores outnumber procsaves by one)
     * the number of cycles spent in the proc is pcycles + cycles()
     * when this is not the current process or we're in user mode
     * (procrestores and procsaves balance), it is pcycles.
     */
    vlong pcycles;
    /*e: [[Proc]] stats and profiling fields */
//--------------------------------------------------------------------
// Debugging (the kernel itself)
//--------------------------------------------------------------------
    /*s: [[Proc]] debugging fields */
    Lock* lastlock;
    /*x: [[Proc]] debugging fields */
    Lock  *lastilock;
    /*x: [[Proc]] debugging fields */
    ulong qpc;    /* pc calling last blocking qlock */
    /*e: [[Proc]] debugging fields */
//--------------------------------------------------------------------
// For debugger, strace
//--------------------------------------------------------------------
    /*s: [[Proc]] debugger fields */
    bool setargs;
    /*x: [[Proc]] debugger fields */
    void  *dbgreg;  /* User registers for devproc */
    /*x: [[Proc]] debugger fields */
    // enum<procctl>
    int procctl;  /* Control for /proc debugging */
    /*x: [[Proc]] debugger fields */
    Proc  *pdbg;    /* the debugging process */
    /*x: [[Proc]] debugger fields */
    QLock debug;    /* to access debugging elements */ // used for many things
    /*x: [[Proc]] debugger fields */
    bool hang;   /* hang at next exec for debug */
    /*x: [[Proc]] debugger fields */
    char  *syscalltrace;  /* syscall trace */
    /*x: [[Proc]] debugger fields */
    bool trace;    /* process being traced? */
    /*e: [[Proc]] debugger fields */
//--------------------------------------------------------------------
// Other
//--------------------------------------------------------------------
    /*s: [[Proc]] other fields */
    bool kp;   /* true if a kernel process */
    void  (*kpfun)(void*);
    void  *kparg;
    /*x: [[Proc]] other fields */
    Sargs sargs;    /* address of this is known by db */
    /*x: [[Proc]] other fields */
    char  genbuf[128];  /* buffer used e.g. for last name element from namec */
    /*x: [[Proc]] other fields */
    Timer;      /* For tsleep and real-time */
    Rendez  *trend;
    int (*tfn)(void*);
    /*x: [[Proc]] other fields */
    ulong alarm;    /* Time of call */
    /*x: [[Proc]] other fields */
    // ref_counted<egrp>
    Egrp  *egrp;    /* Environment group */
    /*x: [[Proc]] other fields */
    ulong procmode; /* proc device default file mode */
    /*x: [[Proc]] other fields */
    bool privatemem; /* proc does not let anyone read mem */
    /*x: [[Proc]] other fields */
    // enum<fpsavestatus>
    int fpstate;
    ArchFPsave  fpsave;   /* address of this is known by db */
    /*e: [[Proc]] other fields */
//--------------------------------------------------------------------
// Extra
//--------------------------------------------------------------------
    /*s: [[Proc]] extra fields */
    // list<ref<Proc>> KQlock.head or RWLock.head (or Procalloc.free)
    Proc  *qnext;   /* next process on queue for a QLock */
    /*x: [[Proc]] extra fields */
    // hash<?, list<ref<Proc>> Procalloc.ht
    Proc  *pidhash; /* next proc in pid hash */ 
    /*x: [[Proc]] extra fields */
    // list<ref<Proc>> of Schedq.head
    Proc  *rnext;   /* next process in run queue */
    /*x: [[Proc]] extra fields */
    // Alarms.head chain?
    Proc  *palarm;  /* Next alarm time */
    /*e: [[Proc]] extra fields */
};
/*e: struct Proc */

/*s: macro waserror poperror */
// poor's man exceptions in C
//  - waserror() =~ try  
//     * if (!waserror()) { } else { } <=> try { } catch { }
//     * if (waserror()) { }  <=> finally { }
//  - poperror() = nothing
//  - error() =~ raise
//  - nexterror() =~ re raise from exn handler
// note, setlabel() return false, so the branch is never taken first
// but nexterror() is using gotolabel() which returns true, see l_switch.s
#define waserror()  (up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
#define poperror()    up->nerrlab--
/*e: macro waserror poperror */


//*****************************************************************************
// Internal to process/
//*****************************************************************************

// Proc allocator (singleton), was actually in proc.c, but important so here
/*s: struct Procalloc */
struct Procalloc
{
    // array<Proc>, xalloc'ed in procinit() (conf.nproc)
    Proc* arena;
  
    // list<ref<Proc>> (next = Proc.qnext, hmmm abuse qnext)
    Proc* free;

    // hash<Proc.pid, list<ref<Proc>> (next = Proc.pidhash)>
    Proc* ht[128];
  
    // extra
    Lock;
};
/*e: struct Procalloc */
//IMPORTANT: static struct Procalloc procalloc; (in proc.c)

/*s: struct Schedq */
// essentially a queue<ref<Proc>>
struct Schedq
{
    // list<ref<Proc>> (next = Proc.rnext)
    Proc* head;
    // ref<Proc>, the tail
    Proc* tail;
    // size of list
    int n; 
  
    // extra
    Lock;
};
/*e: struct Schedq */
// hash<enum<priority>, Schedq>, Nrq is the number of priority level (20+2)
//IMPORTANT: Schedq  runq[Nrq];  (in proc.c)

// was in alarm.c, but important so here
/*s: struct Alarms */
struct Alarms
{
    // list<ref<Proc> (next = Proc.palarm)
    Proc  *head;
    // extra
    QLock;
};
/*e: struct Alarms */
//IMPORTANT: static Alarms alarms; (in alarm.c)
//IMPORTANT: static Rendez alarmr; (in alarm.c)

/*s: struct Active */
struct Active
{
    // array<bool> (coupling: sizeof(int) must be >= MAXCPUS)
    int cpus;      /* bitmap of active CPUs */
    bool exiting;    /* shutdown */
    /*s: [[Active]] other fields */
    bool ispanic;    /* shutdown in response to a panic */
    /*x: [[Active]] other fields */
    bool rebooting;    /* just idle cpus > 0 */
    /*x: [[Active]] other fields */
    bool main_reached_sched;/* lets the added processors continue to schedinit*/
    /*e: [[Active]] other fields */
    // extra
    Lock;
};
/*e: struct Active */
extern struct Active active;

/*s: portdat_processes.h pragmas */
#pragma varargck  type  "t"   long
#pragma varargck  type  "U"   uvlong
/*e: portdat_processes.h pragmas */

/*e: portdat_processes.h */
