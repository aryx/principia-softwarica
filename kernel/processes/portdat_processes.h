
// in lib.h: Waitmsg, ERRMAX
// see also Perf, Fpstate (enum), in 386/ (but used in port)
// see also ArchMMU, ArchNotsave, MAXSYSARG in 386/

//*****************************************************************************
// Proc components
//*****************************************************************************

struct Waitq
{
  Waitmsg w;

  // extra
  // list<ref<Waitq>> of ??
  Waitq *next;
};


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



enum
{
  DELTAFD = 20    /* incremental increase in Fgrp.fd's */
};

struct Fgrp
{
  // array<ref_own?<Chan>, smalloc'ed??
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
  // hash<??, list<ref<Mhead>?? ??
  Mhead *mnthash[MNTHASH];
  ulong pgrpid;
  bool noattach;

  // extra
  Ref;        /* also used as a lock when mounting */
  QLock debug;      /* single access via devproc.c */
  RWlock  ns;     /* Namespace n read/one write lock */
};


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

// was in portclock.c
struct Timers
{
  // list<Timer> (next = Timer.tnext?)
  Timer *head;

  // extra
  Lock;
};




// syscall arguments passed in kernel stack
struct Sargs
{
  ulong args[MAXSYSARG];
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


//*****************************************************************************
// Proc, the big one
//*****************************************************************************

// All the ref<Page> here are references to Proc in the array<Proc> of 
// Procalloc.arena (pool allocator)

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

enum devproc 
{
  Proc_stopme = 1,  /* devproc requests */
  Proc_exitme,
  Proc_traceme,
  Proc_exitbig,
  Proc_tracesyscall,
};

enum proctime 
{
  TUser = 0,    /* Proc.time */
  TSys,
  TReal,
  TCUser,
  TCSys,
  TCReal,
};

enum {
  NERR = 64,
  NNOTE = 5,
};

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

/*
 *  process memory segments - NSEG always last !
 */
enum procseg
{
  SSEG, TSEG, DSEG, BSEG, // Stack, Text, Data, Bss
  ESEG, LSEG, // Extra, L?
  SEG1, SEG2, SEG3, SEG4,    
  NSEG // to count, see Proc.seg array
};

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


// the most important fields are set by newproc()
struct Proc
{

//--------------------------------------------------------------------
// Assembly requirements, Low level, have to be first
//--------------------------------------------------------------------
  // TODO: have to be first?
  Label sched;    /* known to l.s */
  char  *kstack;  /* known to l.s */

//--------------------------------------------------------------------
// State
//--------------------------------------------------------------------
  ulong pid;

  // enum<procstate>
  int state; // Dead, Queuing, etc, see the enum below
  char  *psstate; /* What /proc/#/status reports */

  // e.g. "*init*"
  char  *text;
  char  *user;
  // set by??
  char  *args;
  int nargs;    /* number of bytes of args */

//--------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------
  // array<option<ref_own<Segment>>>, elt smalloc'ed?
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
  // ref<pgrp>, can be shared?
  Pgrp  *pgrp;    /* Process group for namespace */
  // ref<egrp>, can be shared?
  Egrp  *egrp;    /* Environment group */
  // ref<fgrp>, can be shared?
  Fgrp  *fgrp;    /* File descriptor group */

  // ref<Chan>
  Chan  *slash; // The root!
  // ref<Chan>
  Chan  *dot; // The current directory

//--------------------------------------------------------------------
// Notes
//--------------------------------------------------------------------
  ulong noteid;   /* Equivalent of note group */

  Note  note[NNOTE];
  short nnote;
  short notified; /* sysnoted is due */
  Note  lastnote;
  int (*notify)(void*, char*);

//--------------------------------------------------------------------
// Stats
//--------------------------------------------------------------------
  // hash<enum<proctime>, ulong>
  ulong time[6];  /* User, Sys, Real; child U, S, R */

//--------------------------------------------------------------------
// Error managment
//--------------------------------------------------------------------

  int nerrlab;
  Label errlab[NERR];
  char  *syserrstr; /* last error from a system call, errbuf0 or 1 */
  char  *errstr;  /* reason we're unwinding the error stack, errbuf1 or 0 */
  char  errbuf0[ERRMAX];
  char  errbuf1[ERRMAX];

//--------------------------------------------------------------------
// Debugging
//--------------------------------------------------------------------

  void  *dbgreg;  /* User registers for devproc */

//--------------------------------------------------------------------
// Other
//--------------------------------------------------------------------

  Rgrp  *rgrp;    /* Rendez group */


  Waitq *waitq;   /* Exited processes wait children */
  Lock  exl;    /* Lock count and waitq */

  Proc  *parent;
  int nchild;   /* Number of living children */
  int nwait;    /* Number of uncollected wait records */
  QLock qwaitr;
  Rendez  waitr;    /* Place to hang out in wait */

  Fgrp  *closingfgrp; /* used during teardown */

  ulong parentpid;

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

  int insyscall;
  // enum<fpsavestatus>
  int fpstate;

  QLock debug;    /* to access debugging elements of User */
  Proc  *pdbg;    /* the debugging process */
  ulong procmode; /* proc device default file mode */
  ulong privatemem; /* proc does not let anyone read mem */
  int hang;   /* hang at next exec for debug */
  int procctl;  /* Control for /proc debugging */
  ulong pc;   /* DEBUG only */

  Lock  rlock;    /* sync sleep/wakeup with postnote */
  Rendez  *r;   /* rendezvous point slept on */
  Rendez  sleep;    /* place for syssleep/debug */
  int notepending;  /* note issued but not acted on */
  int kp;   /* true if a kernel process */
  int newtlb;   /* Pager has changed my pte's, I must flush */
  int noswap;   /* process is not swappable */

  uintptr rendtag;  /* Tag for rendezvous */
  uintptr rendval;  /* Value for rendezvous */
  Proc  *rendhash;  /* Hash list for tag values */

  Timer;      /* For tsleep and real-time */
  Rendez  *trend;
  int (*tfn)(void*);
  void  (*kpfun)(void*);
  void  *kparg;

  ArchFPsave  fpsave;   /* address of this is known by db */

  int scallnr;  /* sys call number - known by db */
  Sargs s;    /* address of this is known by db */

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

  int trace;    /* process being traced? */

  ulong qpc;    /* pc calling last blocking qlock */

  int setargs;

  void  *ureg;    /* User registers for notes */

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

