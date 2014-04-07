

// in lib.h: Waitmsg, ERRMAX
// see also Perf, Fpstate (enum), Active in 386/ (but used in port)
// see also PMMU, Notsave, MAXSYSARG in 386/

//*****************************************************************************
// Proc components
//*****************************************************************************

struct Waitq
{
	Waitmsg	w;
	Waitq	*next;
};


enum
{
	RENDLOG	=	5,
	RENDHASH =	1<<RENDLOG,	/* Hash to lookup rendezvous tags */
};
#define REND(p,s)	((p)->rendhash[(s)&((1<<RENDLOG)-1)])

struct Rgrp
{
	Ref;				/* the Ref's lock is also the Rgrp's lock */
	Proc	*rendhash[RENDHASH];	/* Rendezvous tag hash */
};

enum
{
	DELTAFD	= 20		/* incremental increase in Fgrp.fd's */
};

struct Fgrp
{
	Ref;
	Chan	**fd;
	int	nfd;			/* number allocated */
	int	maxfd;			/* highest fd in use */
	int	exceed;			/* debugging */
};


enum
{
	MNTLOG	=	5,
	MNTHASH =	1<<MNTLOG,	/* Hash to walk mount table */
};
#define MOUNTH(p,qid)	((p)->mnthash[(qid).path&((1<<MNTLOG)-1)])

// Namespace process group
struct Pgrp
{
	Ref;				/* also used as a lock when mounting */
	int	noattach;
	ulong	pgrpid;
	QLock	debug;			/* single access via devproc.c */
	RWlock	ns;			/* Namespace n read/one write lock */
	Mhead	*mnthash[MNTHASH];
};



/*
 * fasttick timer interrupts
 */
enum {
	/* Mode */
	Trelative,	/* timer programmed in ns from now */
	Tperiodic,	/* periodic timer, period in ns */
};

struct Timer
{
	/* Public interface */
	int	tmode;		/* See above */
	vlong	tns;		/* meaning defined by mode */
	void	(*tf)(Ureg*, Timer*);
	void	*ta;
	/* Internal */
	Lock;
	Timers	*tt;		/* Timers queue this timer runs on */
	Tval	tticks;		/* tns converted to ticks */
	Tval	twhen;		/* ns represented in fastticks */
	Timer	*tnext;
};

// was in portclock.c
struct Timers
{
	Lock;
	Timer	*head;
};




// syscall arguments passed in kernel stack
struct Sargs
{
	ulong	args[MAXSYSARG];
};


enum
{
	NUser,				/* note provided externally */
	NExit,				/* deliver note quietly */
	NDebug,				/* print debug message */
};

// a kind of unix signal
struct Note
{
	char	msg[ERRMAX];
	int	flag;			/* whether system posted it */
};



// was in edf.h
enum {
	Maxsteps = 200 * 100 * 2,	/* 100 periods of 200 procs */

	/* Edf.flags field */
	Admitted		= 0x01,
	Sporadic		= 0x02,
	Yieldonblock		= 0x04,
	Sendnotes		= 0x08,
	Deadline		= 0x10,
	Yield			= 0x20,
	Extratime		= 0x40,

	Infinity = ~0ULL,
};

struct Edf {
	/* All times in µs */
	/* time intervals */
	long		D;		/* Deadline */
	long		Delta;		/* Inherited deadline */
	long		T;		/* period */
	long		C;		/* Cost */
	long		S;		/* Slice: time remaining in this period */
	/* times (only low-order bits of absolute time) */
	long		r;		/* (this) release time */
	long		d;		/* (this) deadline */
	long		t;		/* Start of next period, t += T at release */
	long		s;		/* Time at which this proc was last scheduled */
	/* for schedulability testing */
	long		testDelta;
	int		testtype;	/* Release or Deadline */
	long		testtime;
	Proc		*testnext;
	/* other */
	ushort		flags;
	Timer;
	/* Stats */
	long		edfused;
	long		extraused;
	long		aged;
	ulong		periods;
	ulong		missed;
};


//*****************************************************************************
// Proc, the big one
//*****************************************************************************

// TODO split and name sub enums
enum
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

	Proc_stopme = 1, 	/* devproc requests */
	Proc_exitme,
	Proc_traceme,
	Proc_exitbig,
	Proc_tracesyscall,

	TUser = 0, 		/* Proc.time */
	TSys,
	TReal,
	TCUser,
	TCSys,
	TCReal,

	NERR = 64,
	NNOTE = 5,

	Npriq		= 20,		/* number of scheduler priority levels */
	Nrq		= Npriq+2,	/* number of priority levels including real time */
	PriRelease	= Npriq,	/* released edf processes */
	PriEdf		= Npriq+1,	/* active edf processes */
	PriNormal	= 10,		/* base priority for normal processes */
	PriExtra	= Npriq-1,	/* edf processes at high best-effort pri */
	PriKproc	= 13,		/* base priority for kernel processes */
	PriRoot		= 13,		/* base priority for root processes */

	//NFD =		100,		/* per process file descriptors */
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



// the most important fields are set by newproc()
// TODO reorganize, move in extra/
struct Proc
{
  // TODO: have to be first?
	Label	sched;		/* known to l.s */
	char	*kstack;	/* known to l.s */

	ulong	pid; // PID!!

	int	state; // Dead, Queuing, etc, see the enum below
	char	*psstate;	/* What /proc/#/status reports */

	Segment	*seg[NSEG];
	QLock	seglock;	/* locked whenever seg[] changes */

	char	*text;
	char	*user;
	char	*args;
	int	nargs;		/* number of bytes of args */

	Pgrp	*pgrp;		/* Process group for namespace */
	Egrp 	*egrp;		/* Environment group */
	Fgrp	*fgrp;		/* File descriptor group */
	Rgrp	*rgrp;		/* Rendez group */

	Chan	*slash; // The root!
	Chan	*dot; // The current directory


	ulong	noteid;		/* Equivalent of note group */


	Waitq	*waitq;		/* Exited processes wait children */
	Lock	exl;		/* Lock count and waitq */

	Proc	*parent;
	int	nchild;		/* Number of living children */
	int	nwait;		/* Number of uncollected wait records */
	QLock	qwaitr;
	Rendez	waitr;		/* Place to hang out in wait */

	Fgrp	*closingfgrp;	/* used during teardown */

	ulong	parentpid;

	ulong	time[6];	/* User, Sys, Real; child U, S, R */

	uvlong	kentry;		/* Kernel entry time stamp (for profiling) */
	/*
	 * pcycles: cycles spent in this process (updated on procsave/restore)
	 * when this is the current proc and we're in the kernel
	 * (procrestores outnumber procsaves by one)
	 * the number of cycles spent in the proc is pcycles + cycles()
	 * when this is not the current process or we're in user mode
	 * (procrestores and procsaves balance), it is pcycles.
	 */
	vlong	pcycles;

	int	insyscall;
	int	fpstate;

	QLock	debug;		/* to access debugging elements of User */
	Proc	*pdbg;		/* the debugging process */
	ulong	procmode;	/* proc device default file mode */
	ulong	privatemem;	/* proc does not let anyone read mem */
	int	hang;		/* hang at next exec for debug */
	int	procctl;	/* Control for /proc debugging */
	ulong	pc;		/* DEBUG only */

	Lock	rlock;		/* sync sleep/wakeup with postnote */
	Rendez	*r;		/* rendezvous point slept on */
	Rendez	sleep;		/* place for syssleep/debug */
	int	notepending;	/* note issued but not acted on */
	int	kp;		/* true if a kernel process */
	int	newtlb;		/* Pager has changed my pte's, I must flush */
	int	noswap;		/* process is not swappable */

	uintptr	rendtag;	/* Tag for rendezvous */
	uintptr	rendval;	/* Value for rendezvous */
	Proc	*rendhash;	/* Hash list for tag values */

	Timer;			/* For tsleep and real-time */
	Rendez	*trend;
	int	(*tfn)(void*);
	void	(*kpfun)(void*);
	void	*kparg;

	FPsave	fpsave;		/* address of this is known by db */
	int	scallnr;	/* sys call number - known by db */
	Sargs	s;		/* address of this is known by db */

	int	nerrlab;
	Label	errlab[NERR];
	char	*syserrstr;	/* last error from a system call, errbuf0 or 1 */
	char	*errstr;	/* reason we're unwinding the error stack, errbuf1 or 0 */
	char	errbuf0[ERRMAX];
	char	errbuf1[ERRMAX];

	char	genbuf[128];	/* buffer used e.g. for last name element from namec */


	Note	note[NNOTE];
	short	nnote;
	short	notified;	/* sysnoted is due */
	Note	lastnote;
	int	(*notify)(void*, char*);

	Lock	*lockwait;
	Lock	*lastlock;	/* debugging */
	Lock	*lastilock;	/* debugging */

	Mach	*wired;
	Mach	*mp;		/* machine this process last ran on */

  // As long as the current process hold locks (to kernel data structures),
  // we will not schedule another process in unlock(); only the last unlock
  // will eventually cause a rescheduling.
	Ref	nlocks;		/* number of locks held by proc */


	ulong	delaysched;
	ulong	priority;	/* priority level */
	ulong	basepri;	/* base priority level */
	uchar	fixedpri;	/* priority level deson't change */
	ulong	cpu;		/* cpu average */
	ulong	lastupdate;
	uchar	yield;		/* non-zero if the process just did a sleep(0) */
	ulong	readytime;	/* time process came ready */
	ulong	movetime;	/* last time process switched processors */
	int	preempted;	/* true if this process hasn't finished the interrupt
				 *  that last preempted it
				 */
	Edf	*edf;		/* if non-null, real-time proc, edf contains scheduling params */
	int	trace;		/* process being traced? */

	ulong	qpc;		/* pc calling last blocking qlock */

	int	setargs;

	void	*ureg;		/* User registers for notes */
	void	*dbgreg;	/* User registers for devproc */
	Notsave;

	/*
	 *  machine specific MMU
	 */
	PMMU;
	char	*syscalltrace;	/* syscall trace */

  // Extra

  // KQlock.head chain
	Proc	*qnext;		/* next process on queue for a QLock */
	QLock	*qlock;		/* addr of qlock being queued for DEBUG */

  // Schedq.head chain?
	Proc	*rnext;		/* next process in run queue */

  // Alarms.head chain?
	Proc	*palarm;	/* Next alarm time */
	ulong	alarm;		/* Time of call */

  // Procalloc.ht chain?
	Proc	*pidhash;	/* next proc in pid hash */ 

	Mach	*mach;		/* machine running this proc */

};


//*****************************************************************************
// Internal to processe/
//*****************************************************************************

// Proc allocator (singleton), was actually in proc.c, but important so here
struct Procalloc
{
	Proc*	arena; // initial array of proc, xalloc'ed in procinit0() (conf.nproc)

	Proc*	free; // first free proc
	Proc*	ht[128]; // hash pid -> proc (see Proc.pidhash)

  // extra
	Lock;
};
//static struct Procalloc procalloc; // private to proc.c

struct Schedq
{
	Proc*	head;
	Proc*	tail;
	int	n;

  // extra
	Lock;
};
//Schedq	runq[Nrq];  // private to proc.c

// was in alarm.c, but important so here
struct Alarms
{
	QLock;
	Proc	*head;
};
//static Alarms	alarms; // private to alarm.c
//static Rendez	alarmr; // private to alarm.c


// used to be in edf.h
//unused: extern Lock	edftestlock;	/* for atomic admitting/expelling */

#pragma	varargck	type	"t"		long
#pragma	varargck	type	"U"		uvlong

// struct Active defined in 386 (But used in port)
extern struct Active active;
