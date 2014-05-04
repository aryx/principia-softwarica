
// in lib.h: Waitmsg, ERRMAX
// see also ArchProcMMU, ArchProcNotsave, MAXSYSARG in 386/

//*****************************************************************************
// Proc components
//*****************************************************************************
// All the ref<Proc> here are references to Proc in the array<Proc> of
// Procalloc.arena (pool allocator)

//--------------------------------------------------------------------
// State
//--------------------------------------------------------------------

// TODO: state transition diagram
enum procstate
{
  /* Process states, Proc.state */
  Dead = 0,
  Moribund,
  Ready,
  Scheding,
  Running,
  Queueing, // see lock()
  QueueingR, // see rlock()
  QueueingW, // see wlock ()
  Wakeme,
  Broken,
  Stopped,
  Rendezvous,
  Waitrelease,
};

// hash<enum<procstate>, string>, to debug
extern  char* statename[];

//--------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------
/*
 *  process memory segments - NSEG always last !
 */
enum procseg
{
  SSEG, TSEG, DSEG, BSEG, // Stack, Text, Data, Bss
  ESEG, LSEG, // Extra (temporary stack segment), L?
  SEG1, SEG2, SEG3, SEG4,
  NSEG // to count, see Proc.seg array
};

//--------------------------------------------------------------------
// Files
//--------------------------------------------------------------------

enum
{
  DELTAFD = 20    /* incremental increase in Fgrp.fd's */
};

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



enum
{
  MNTLOG  = 5,
  MNTHASH = 1<<MNTLOG,  /* Hash to walk mount table */
};
#define MOUNTH(p,qid) ((p)->mnthash[(qid).path&((1<<MNTLOG)-1)])

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

//--------------------------------------------------------------------
// System call
//--------------------------------------------------------------------

// syscall arguments passed in kernel stack
struct Sargs
{
  ulong args[MAXSYSARG];
};


//--------------------------------------------------------------------
// Notes
//--------------------------------------------------------------------

enum {
  NNOTE = 5,
};

enum notekind
{
  NUser,        /* note provided externally */
  NExit,        /* deliver note quietly */
  NDebug,       /* print debug message */
};

// a kind of unix signal
struct Note
{
  char  msg[ERRMAX];
  // enum<notekind>
  int flag;     /* whether system posted it */
};
extern Ref  noteidalloc;

//--------------------------------------------------------------------
// Process children waiting
//--------------------------------------------------------------------

struct Waitq
{
  Waitmsg w;

  // extra
  // list<ref<Waitq>> Proc.waitq
  Waitq *next;
};

//--------------------------------------------------------------------
// Synchronization (Rendez vous)
//--------------------------------------------------------------------

enum
{
  RENDLOG = 5,
  RENDHASH =  1<<RENDLOG, /* Hash to lookup rendezvous tags */
};
#define REND(p,s) ((p)->rendhash[(s)&((1<<RENDLOG)-1)])

struct Rgrp
{
  // hash<??, list<ref<Proc>>>
  Proc  *rendhash[RENDHASH];  /* Rendezvous tag hash */

  // extra
  Ref;        /* the Ref's lock is also the Rgrp's lock */
};

//--------------------------------------------------------------------
// Alarms, timers
//--------------------------------------------------------------------

/*
 * fasttick timer interrupts
 */
enum timermode 
{
  Trelative,  /* timer programmed in ns from now */
  Tperiodic,  /* periodic timer, period in ns */
};

struct Timer
{
  /* Public interface */
  // enum<timermode>
  int tmode;    /* See above */
  vlong tns;    /* meaning defined by mode */
  void  (*tf)(Ureg*, Timer*);
  void  *ta;

  /* Internal */
  Lock;
  // ref<list<Timer>> ??
  Timers  *tt;    /* Timers queue this timer runs on */
  Tval  tticks;   /* tns converted to ticks */
  Tval  twhen;    /* ns represented in fastticks */

  // list<Timer> of Timers.head?
  Timer *tnext;
};

// was in clock.c
struct Timers
{
  // list<Timer> (next = Timer.tnext?)
  Timer *head;

  // extra
  Lock;
};

//--------------------------------------------------------------------
// Scheduling
//--------------------------------------------------------------------

enum {
  Npriq   = 20,   /* number of scheduler priority levels */
  Nrq   = Npriq+2,  /* number of priority levels including real time */
  //NFD =   100,    /* per process file descriptors */
};

enum priority 
{
  PriRelease  = Npriq,  /* released edf processes */
  PriEdf    = Npriq+1,  /* active edf processes */
  PriNormal = 10,   /* base priority for normal processes */
  PriExtra  = Npriq-1,  /* edf processes at high best-effort pri */
  PriKproc  = 13,   /* base priority for kernel processes */
  PriRoot   = 13,   /* base priority for root processes */
};


// was in edf.h
enum 
{
  Maxsteps = 200 * 100 * 2, /* 100 periods of 200 procs */
  Infinity = ~0ULL,
};

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

//--------------------------------------------------------------------
// Error managment
//--------------------------------------------------------------------
enum {
  NERR = 64,
};

//--------------------------------------------------------------------
// Stats, profiling
//--------------------------------------------------------------------
enum proctime 
{
  TUser = 0,    /* Proc.time */
  TSys,
  TReal,
  TCUser,
  TCSys,
  TCReal,
};

//--------------------------------------------------------------------
// Debugger
//--------------------------------------------------------------------

enum devproc 
{
  Proc_stopme = 1,  /* devproc requests */
  Proc_exitme,
  Proc_traceme,
  Proc_exitbig,
  Proc_tracesyscall,
};

//--------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------

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


//*****************************************************************************
// Proc, the big one
//*****************************************************************************

// the most important fields are set by newproc()
struct Proc
{

//--------------------------------------------------------------------
// Assembly requirements, Low level, have to be first
//--------------------------------------------------------------------
  Label sched;    /* known to l.s */
  char  *kstack;  /* known to l.s */

//--------------------------------------------------------------------
// State
//--------------------------------------------------------------------
  ulong pid;

  // enum<procstate>
  int state; // Dead, Queuing, etc,
  bool insyscall;
  char  *psstate; /* What /proc/#/status reports */

  // e.g. "*init*", or name of executable
  char  *text;
  // e.g.. "eve" (no uid/gid in plan9, because of distributed nature of it?)
  char  *user;

  // set by??
  char  *args;
  int nargs;    /* number of bytes of args */

  int kp;   /* true if a kernel process */

//--------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------
  // hash<enum<procseg>, option<ref_own<Segment>>>, elt smalloc'ed?
  Segment *seg[NSEG];
  QLock seglock;  /* locked whenever seg[] changes */

//--------------------------------------------------------------------
// Scheduling
//--------------------------------------------------------------------
  // enum<priority>
  ulong priority; /* priority level */

  ulong delaysched;

  ulong basepri;  /* base priority level */
  uchar fixedpri; /* priority level deson't change */

  ulong cpu;    /* cpu average */
  ulong lastupdate;
  uchar yield;    /* non-zero if the process just did a sleep(0) */
  ulong readytime;  /* time process came ready */
  ulong movetime; /* last time process switched processors */
  int preempted;  /* true if this process hasn't finished the interrupt
         *  that last preempted it
         */
  // option<ref_own?<edf>>
  Edf *edf;   /* if non-null, real-time proc, edf contains scheduling params */

//--------------------------------------------------------------------
// Files
//--------------------------------------------------------------------
  // ref_counted<pgrp>
  Pgrp  *pgrp;    /* Process group for namespace */
  // ref_counted<egrp>
  Egrp  *egrp;    /* Environment group */
  // ref_counted<fgrp>
  Fgrp  *fgrp;    /* File descriptor group */

  // ref<Chan>
  Chan  *slash; // The root!
  // ref_counted<Chan>
  Chan  *dot; // The current directory

//--------------------------------------------------------------------
// Notes
//--------------------------------------------------------------------
  ulong noteid;   /* Equivalent of note group */

  int notepending;  /* note issued but not acted on */

  Note  note[NNOTE];
  short nnote;
  short notified; /* sysnoted is due */
  Note  lastnote;
  int (*notify)(void*, char*);

  void  *ureg;    /* User registers for notes */

//--------------------------------------------------------------------
// Process hierarchy
//--------------------------------------------------------------------
  //list<ref<Waitq>>
  Waitq *waitq;   /* Exited processes wait children */
  Lock  exl;    /* Lock count and waitq */

  Proc  *parent;
  ulong parentpid;

  int nchild;   /* Number of living children */
  int nwait;    /* Number of uncollected wait records */
  QLock qwaitr;
  Rendez  waitr;    /* Place to hang out in wait */


//--------------------------------------------------------------------
// Synchronization
//--------------------------------------------------------------------

  Rgrp  *rgrp;    /* Rendez group */

  uintptr rendtag;  /* Tag for rendezvous */
  uintptr rendval;  /* Value for rendezvous */
  //??
  Proc  *rendhash;  /* Hash list for tag values */

  Rendez  *r;   /* rendezvous point slept on */
  Rendez  sleep;    /* place for syssleep/debug */

//--------------------------------------------------------------------
// Error managment
//--------------------------------------------------------------------

  // array<Label>, error labels, poor's man exceptions in C
  Label errlab[NERR];
  // length(errlab) used.
  int nerrlab;

  char  *syserrstr; /* last error from a system call, errbuf0 or 1 */
  char  *errstr;  /* reason we're unwinding the error stack, errbuf1 or 0 */
  char  errbuf0[ERRMAX];
  char  errbuf1[ERRMAX];

//--------------------------------------------------------------------
// Stats, profiling
//--------------------------------------------------------------------
  // hash<enum<proctime>, ulong>
  ulong time[6];  /* User, Sys, Real; child U, S, R */

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

//--------------------------------------------------------------------
// For debugger
//--------------------------------------------------------------------

  void  *dbgreg;  /* User registers for devproc */
  ulong pc;   /* DEBUG only */

  // e.g. Proc_tracesyscall
  int procctl;  /* Control for /proc debugging */

  // Syscall
  int scallnr;  /* sys call number - known by db */
  Sargs s;    /* address of this is known by db */

  QLock debug;    /* to access debugging elements of User */
  Proc  *pdbg;    /* the debugging process */
  bool hang;   /* hang at next exec for debug */

//--------------------------------------------------------------------
// Other
//--------------------------------------------------------------------

  Fgrp  *closingfgrp; /* used during teardown */


  ulong procmode; /* proc device default file mode */
  ulong privatemem; /* proc does not let anyone read mem */

  Lock  rlock;    /* sync sleep/wakeup with postnote */
  int newtlb;   /* Pager has changed my pte's, I must flush */
  int noswap;   /* process is not swappable */

  Timer;      /* For tsleep and real-time */
  Rendez  *trend;
  int (*tfn)(void*);
  void  (*kpfun)(void*);
  void  *kparg;

  ArchFPsave  fpsave;   /* address of this is known by db */

  char  genbuf[128];  /* buffer used e.g. for last name element from namec */


  Lock  *lockwait;
  Lock  *lastlock;  /* debugging */
  Lock  *lastilock; /* debugging */

  Mach  *wired;
  Mach  *mp;    /* machine this process last ran on */

  // As long as the current process hold locks (to kernel data structures),
  // we will not schedule another process in unlock(); only the last unlock
  // will eventually cause a rescheduling.
  Ref nlocks;   /* number of locks held by proc */

  bool trace;    /* process being traced? */

  ulong qpc;    /* pc calling last blocking qlock */

  int setargs;

  // enum<fpsavestatus>
  int fpstate;

  ArchProcNotsave;

  /*
   *  machine specific MMU
   */
  ArchProcMMU;

  char  *syscalltrace;  /* syscall trace */

//--------------------------------------------------------------------
// Extra
//--------------------------------------------------------------------

  // list<ref<Proc>> KQlock.head or RWLock.head
  Proc  *qnext;   /* next process on queue for a QLock */
  // option<ref<Qlock>> ??
  QLock *qlock;   /* addr of qlock being queued for DEBUG */

  // list<ref<Proc>> ?? Schedq.head chain?
  Proc  *rnext;   /* next process in run queue */

  // Alarms.head chain?
  Proc  *palarm;  /* Next alarm time */
  ulong alarm;    /* Time of call */

  // hash<?, list<ref<Proc>> Procalloc.ht ?
  Proc  *pidhash; /* next proc in pid hash */ 

  // option<ref<Mach>>, null when not associated to a machine?
  Mach  *mach;    /* machine running this proc */

};

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


//*****************************************************************************
// Internal to process/
//*****************************************************************************

// Proc allocator (singleton), was actually in proc.c, but important so here
struct Procalloc
{
  // array<Proc>, xalloc'ed in procinit0() (conf.nproc)
  Proc* arena;

  // list<ref<Proc>> (next = ?)
  Proc* free;
  // hash<Proc.pid?, list<ref<Proc>> (next = Proc.pidhash)>
  Proc* ht[128];

  // extra
  Lock;
};
//IMPORTANT: static struct Procalloc procalloc; (in proc.c)

struct Schedq
{
  // list<ref<Proc>> (next = Proc.rnext?)
  Proc* head;
  // list<ref<Proc>> (next = ??)
  Proc* tail;

  int n;

  // extra
  Lock;
};
// hash<enum<priority>, Schedq>, Nrq is the number of priority level (20+2)
//IMPORTANT: Schedq  runq[Nrq];  (in proc.c)

// was in alarm.c, but important so here
struct Alarms
{
  // list<ref<Proc> (next = ??)
  Proc  *head;

  // extra
  QLock;
};
//IMPORTANT: static Alarms alarms; (in alarm.c)
//IMPORTANT: static Rendez alarmr; (in alarm.c)


struct Active
{
  int machs;      /* bitmap of active CPUs */
  int exiting;    /* shutdown */
  int ispanic;    /* shutdown in response to a panic */

  // 386 specific?
  int thunderbirdsarego;  /* lets the added processors continue to schedinit */
  int rebooting;    /* just idle cpus > 0 */

  // extra
  Lock;
};
extern struct Active active;

#pragma varargck  type  "t"   long
#pragma varargck  type  "U"   uvlong

