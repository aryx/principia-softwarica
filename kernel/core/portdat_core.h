
// in lib.h: Waitmsg, ERRMAX
// in 386: Lock (interdepends on Proc), Mach, PMMU, Notsave, MAXSYSARG

struct Waitq
{
	Waitmsg	w;
	Waitq	*next;
};


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

struct Note
{
	char	msg[ERRMAX];
	int	flag;			/* whether system posted it */
};


struct Evalue
{
	char	*name;
	char	*value;
	int	len;
	Evalue	*link;
	Qid	qid;
};





struct Rendez
{
	Lock;
	Proc	*p;
};

struct Sema
{
	Rendez;
	long	*addr;
	int	waiting;
	Sema	*next;
	Sema	*prev;
};


struct KQLock
{
	Lock	use;		/* to access Qlock structure */
	Proc	*head;		/* next process waiting for object */
	Proc	*tail;		/* last process waiting for object */
	int	locked;		/* flag */
	uintptr	qpc;		/* pc of the holder */
};

struct RWlock
{
	Lock	use;
	Proc	*head;		/* list of waiting processes */
	Proc	*tail;
	ulong	wpc;		/* pc of writer */
	Proc	*wproc;		/* writing proc */
	int	readers;	/* number of readers */
	int	writer;		/* number of writers */
};

struct Ref
{
	Lock;
	long	ref;
};






struct Mount
{
	ulong	mountid;
	Mount*	next;
	Mhead*	head;
	Mount*	copy;
	Mount*	order;
	Chan*	to;			/* channel replacing channel */
	int	mflag;
	char	*spec;
};

struct Mhead
{
	Ref;
	RWlock	lock;
	Chan*	from;			/* channel mounted upon */
	Mount*	mount;			/* what's mounted upon it */
	Mhead*	hash;			/* Hash chain */
};

struct Path
{
	Ref;
	char	*s;
	Chan	**mtpt;			/* mtpt history */
	int	len;			/* strlen(s) */
	int	alen;			/* allocated length of s */
	int	mlen;			/* number of path elements */
	int	malen;			/* allocated length of mtpt */
};

struct Chan
{
	Ref;				/* the Lock in this Ref is also Chan's lock */
	Chan*	next;			/* allocation */
	Chan*	link;
	vlong	offset;			/* in fd */
	vlong	devoffset;		/* in underlying device; see read */
	ushort	type;
	ulong	dev;
	ushort	mode;			/* read/write */
	ushort	flag;
	Qid	qid;
	int	fid;			/* for devmnt */
	ulong	iounit;			/* chunk size for i/o; 0==default */
	Mhead*	umh;			/* mount point that derived Chan; used in unionread */
	Chan*	umc;			/* channel in union; held for union read */
	QLock	umqlock;		/* serialize unionreads */
	int	uri;			/* union read index */
	int	dri;			/* devdirread index */
	uchar*	dirrock;		/* directory entry rock for translations */
	int	nrock;
	int	mrock;
	QLock	rockqlock;
	int	ismtpt;
	Mntcache* mcp;			/* Mount cache pointer */
	Mnt*	mux;			/* Mnt for clients using me for messages */
	union {
		void*	aux;
		Qid	pgrpid;		/* for #p/notepg */
		ulong	mid;		/* for ns in devproc */
	};
	Chan*	mchan;			/* channel to mounted server */
	Qid	mqid;			/* qid of root of mount point */
	Path*	path;
};








struct Egrp
{
	Ref;
	RWlock;
	Evalue	**ent;
	int	nent;
	int	ment;
	ulong	path;	/* qid.path of next Evalue to be allocated */
	ulong	vers;	/* of Egrp */
};


enum
{
	RENDLOG	=	5,
	RENDHASH =	1<<RENDLOG,	/* Hash to lookup rendezvous tags */
	MNTLOG	=	5,
	MNTHASH =	1<<MNTLOG,	/* Hash to walk mount table */
	NFD =		100,		/* per process file descriptors */
	PGHLOG  =	9,
	PGHSIZE	=	1<<PGHLOG,	/* Page hash for image lookup */
};
#define REND(p,s)	((p)->rendhash[(s)&((1<<RENDLOG)-1)])
#define MOUNTH(p,qid)	((p)->mnthash[(qid).path&((1<<MNTLOG)-1)])

struct Rgrp
{
	Ref;				/* the Ref's lock is also the Rgrp's lock */
	Proc	*rendhash[RENDHASH];	/* Rendezvous tag hash */
};

struct Fgrp
{
	Ref;
	Chan	**fd;
	int	nfd;			/* number allocated */
	int	maxfd;			/* highest fd in use */
	int	exceed;			/* debugging */
};


struct Pgrp
{
	Ref;				/* also used as a lock when mounting */
	int	noattach;
	ulong	pgrpid;
	QLock	debug;			/* single access via devproc.c */
	RWlock	ns;			/* Namespace n read/one write lock */
	Mhead	*mnthash[MNTHASH];
};








enum
{
	PG_NOFLUSH	= 0,
	PG_TXTFLUSH	= 1,		/* flush dcache and invalidate icache */
	PG_DATFLUSH	= 2,		/* flush both i & d caches (UNUSED) */
	PG_NEWCOL	= 3,		/* page has been recolored */

	PG_MOD		= 0x01,		/* software modified bit */
	PG_REF		= 0x02,		/* software referenced bit */
};

struct Page
{
	Lock;
	ulong	pa;			/* Physical address in memory */
	ulong	va;			/* Virtual address for user */
	ulong	daddr;			/* Disc address on swap */
	ulong	gen;			/* Generation counter for swap */
	ushort	ref;			/* Reference count */
	char	modref;			/* Simulated modify/reference bits */
	char	color;			/* Cache coloring */
	char	cachectl[MAXMACH];	/* Cache flushing control for putmmu */
	Image	*image;			/* Associated text or swap image */
	Page	*next;			/* Lru free list */
	Page	*prev;
	Page	*hash;			/* Image hash chains */
};


struct Image2
{
	Ref;
	Chan	*c;			/* channel to text file */
	Qid 	qid;			/* Qid for page cache coherence */
	Qid	mqid;
	Chan	*mchan;
	ushort	type;			/* Device type of owning channel */
	Segment *s;			/* TEXT segment for image if running */
	Image	*hash;			/* Qid hash chains */
	Image	*next;			/* Free list */
	int	notext;			/* no file associated */
};


struct Pte
{
	Page	*pages[PTEPERTAB];	/* Page map for this chunk of pte */
	Page	**first;		/* First used entry */
	Page	**last;			/* Last used entry */
};



/* Segment types */
enum
{
	SG_TYPE		= 07,		/* Mask type of segment */
	SG_TEXT		= 00,
	SG_DATA		= 01,
	SG_BSS		= 02,
	SG_STACK	= 03,
	SG_SHARED	= 04,
	SG_PHYSICAL	= 05,

	SG_RONLY	= 0040,		/* Segment is read only */
	SG_CEXEC	= 0100,		/* Detach at exec */
};

#define PG_ONSWAP	1
#define onswap(s)	(((ulong)s)&PG_ONSWAP)
#define pagedout(s)	(((ulong)s)==0 || onswap(s))
#define swapaddr(s)	(((ulong)s)&~PG_ONSWAP)

#define SEGMAXSIZE	(SEGMAPSIZE*PTEMAPMEM)

struct Physseg
{
	ulong	attr;			/* Segment attributes */
	char	*name;			/* Attach name */
	ulong	pa;			/* Physical address */
	ulong	size;			/* Maximum segment size in pages */
	Page	*(*pgalloc)(Segment*, ulong);	/* Allocation if we need it */
	void	(*pgfree)(Page*);
};


/*
 *  process memory segments - NSEG always last !
 */
enum
{
	SSEG, TSEG, DSEG, BSEG, ESEG, LSEG, SEG1, SEG2, SEG3, SEG4, NSEG
};

struct Segment
{
	Ref;
	QLock	lk;
	ushort	steal;		/* Page stealer lock */
	ushort	type;		/* segment type */
	ulong	base;		/* virtual base */
	ulong	top;		/* virtual top */
	ulong	size;		/* size in pages */
	ulong	fstart;		/* start address in file for demand load */
	ulong	flen;		/* length of segment in file */
	int	flushme;	/* maintain icache for this segment */
	Image	*image;		/* text in file attached to this segment */
	Physseg *pseg;
	ulong*	profile;	/* Tick profile area */
	Pte	**map;
	int	mapsize;
	Pte	*ssegmap[SSEGMAPSIZE];
	Lock	semalock;
	Sema	sema;
	ulong	mark;		/* portcountrefs */
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






enum
{
	Dead = 0,		/* Process states */
	Moribund,
	Ready,
	Scheding,
	Running,
	Queueing,
	QueueingR,
	QueueingW,
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
};



struct Proc
{
	Label	sched;		/* known to l.s */
	char	*kstack;	/* known to l.s */
	Mach	*mach;		/* machine running this proc */
	char	*text;
	char	*user;
	char	*args;
	int	nargs;		/* number of bytes of args */
	Proc	*rnext;		/* next process in run queue */
	Proc	*qnext;		/* next process on queue for a QLock */
	QLock	*qlock;		/* addr of qlock being queued for DEBUG */
	int	state;
	char	*psstate;	/* What /proc/#/status reports */
	Segment	*seg[NSEG];
	QLock	seglock;	/* locked whenever seg[] changes */
	ulong	pid;
	ulong	noteid;		/* Equivalent of note group */
	Proc	*pidhash;	/* next proc in pid hash */

	Lock	exl;		/* Lock count and waitq */
	Waitq	*waitq;		/* Exited processes wait children */
	int	nchild;		/* Number of living children */
	int	nwait;		/* Number of uncollected wait records */
	QLock	qwaitr;
	Rendez	waitr;		/* Place to hang out in wait */
	Proc	*parent;

	Pgrp	*pgrp;		/* Process group for namespace */
	Egrp 	*egrp;		/* Environment group */
	Fgrp	*fgrp;		/* File descriptor group */
	Rgrp	*rgrp;		/* Rendez group */

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
	Proc	*palarm;	/* Next alarm time */
	ulong	alarm;		/* Time of call */
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
	Chan	*slash;
	Chan	*dot;

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
};
