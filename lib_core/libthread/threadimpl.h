/*s: lib_core/libthread/threadimpl.h */
/* 
 * Some notes on locking:
 *
 *	All the locking woes come from implementing
 *	threadinterrupt (and threadkill).
 *
 *	_threadgetproc()->thread is always a live pointer.
 *	p->threads, p->ready, and _threadrgrp also contain
 * 	live thread pointers.  These may only be consulted
 *	while holding p->lock or _threadrgrp.lock; in procs
 *	other than p, the pointers are only guaranteed to be live
 *	while the lock is still being held.
 *
 *	Thread structures can only be freed by the proc
 *	they belong to.  Threads marked with t->inrendez
 * 	need to be extracted from the _threadrgrp before
 *	being freed.
 *
 *	_threadrgrp.lock cannot be acquired while holding p->lock.
 */

typedef struct Pqueue	Pqueue;
typedef struct Rgrp		Rgrp;
typedef struct Tqueue	Tqueue;
typedef struct Thread	Thread;
typedef struct Execargs	Execargs;
typedef struct Proc		Proc;

/* must match list in sched.c */
typedef enum
{
    Dead,
    Running,
    Ready,
    Rendezvous,
} State;
    
typedef enum
{
    Channone,
    Chanalt,
    Chansend,
    Chanrecv,
} Chanstate;

/*s: enum _anon_ (lib_core/libthread/threadimpl.h) */
enum
{
    RENDHASH = 13,
    Printsize = 2048,
    NPRIV = 8,
};
/*e: enum _anon_ (lib_core/libthread/threadimpl.h) */

/*s: struct Rgrp */
struct Rgrp
{
    Lock		lock;
    Thread	*hash[RENDHASH];
};
/*e: struct Rgrp */

/*s: struct Tqueue */
struct Tqueue		/* Thread queue */
{
    int		asleep;
    Thread	*head;
    Thread	**tail;
};
/*e: struct Tqueue */

/*s: struct Thread */
struct Thread
{
    Lock		lock;		/* protects thread data structure */
    jmp_buf		sched;		/* for context switches */
    int		id;		/* thread id */
    int 		grp;		/* thread group */
    int		moribund;	/* thread needs to die */
    State		state;		/* run state */
    State		nextstate;	/* next run state */
    uchar		*stk;		/* top of stack (lowest address of stack) */
    uint		stksize;	/* stack size */
    Thread		*next;		/* next on ready queue */

    Proc		*proc;		/* proc of this thread */
    Thread		*nextt;		/* next on list of threads in this proc*/
    int		ret;		/* return value for Exec, Fork */

    char		*cmdname;	/* ptr to name of thread */

    int		inrendez;
    Thread		*rendhash;	/* Trgrp linked list */
    void*		rendtag;	/* rendezvous tag */
    void*		rendval;	/* rendezvous value */
    int		rendbreak;	/* rendezvous has been taken */

    Chanstate	chan;		/* which channel operation is current */
    Alt		*alt;		/* pointer to current alt structure (debugging) */

    void*	udata[NPRIV];	/* User per-thread data pointer */
};
/*e: struct Thread */

/*s: struct Execargs */
struct Execargs
{
    char		*prog;
    char		**args;
    int		fd[2];
};
/*e: struct Execargs */

/*s: struct Proc */
struct Proc
{
    Lock		lock;
    jmp_buf		sched;			/* for context switches */
    int		pid;			/* process id */
    int		splhi;			/* delay notes */
    Thread		*thread;		/* running thread */

    int		needexec;
    Execargs	exec;			/* exec argument */
    Proc		*newproc;		/* fork argument */
    char		exitstr[ERRMAX];	/* exit status */

    int		rforkflag;
    int		nthreads;
    Tqueue		threads;		/* All threads of this proc */
    Tqueue		ready;			/* Runnable threads */
    Lock		readylock;

    char		printbuf[Printsize];
    int		blocked;		/* In a rendezvous */
    int		pending;		/* delayed note pending */
    int		nonotes;		/*  delay notes */
    uint		nextID;			/* ID of most recently created thread */
    Proc		*next;			/* linked list of Procs */

    void		*arg;			/* passed between shared and unshared stk */
    char		str[ERRMAX];		/* used by threadexits to avoid malloc */

    void*		wdata;			/* Lib(worker) per-proc data pointer */
    void*		udata;			/* User per-proc data pointer */
    char		threadint;		/* tag for threadexitsall() */
};
/*e: struct Proc */

/*s: struct Pqueue */
struct Pqueue {		/* Proc queue */
    Lock		lock;
    Proc		*head;
    Proc		**tail;
};
/*e: struct Pqueue */

/*s: struct Ioproc */
struct Ioproc
{
    int tid;
    Channel *c, *creply;
    int inuse;
    long (*op)(va_list*);
    va_list arg;
    long ret;
    char err[ERRMAX];
    Ioproc *next;
};
/*e: struct Ioproc */

void	_freeproc(Proc*);
void	_freethread(Thread*);
Proc*	_newproc(void(*)(void*), void*, uint, char*, int, int);
int	_procsplhi(void);
void	_procsplx(int);
void	_sched(void);
int	_schedexec(Execargs*);
void	_schedexecwait(void);
void	_schedexit(Proc*);
int	_schedfork(Proc*);
void	_schedinit(void*);
void	_systhreadinit(void);
void	_threadassert(char*);
void	_threadbreakrendez(void);
void	_threaddebug(ulong, char*, ...);
void	_threadexitsall(char*);
void	_threadflagrendez(Thread*);
Proc*	_threadgetproc(void);
void	_threadsetproc(Proc*);
void	_threadinitstack(Thread*, void(*)(void*), void*);
void*	_threadmalloc(long, int);
void	_threadnote(void*, char*);
void	_threadready(Thread*);
void*	_threadrendezvous(void*, void*);
void	_threadsignal(void);
void	_threadsysfatal(char*, va_list);
void**	_workerdata(void);
void	_xinc(long*);
long	_xdec(long*);

extern int			_threaddebuglevel;
extern char*		_threadexitsallstatus;
extern Pqueue		_threadpq;
extern Channel*	_threadwaitchan;
extern Rgrp		_threadrgrp;

/*s: constant DBGAPPL */
#define DBGAPPL	(1 << 0)
/*e: constant DBGAPPL */
/*s: constant DBGSCHED */
#define DBGSCHED	(1 << 16)
/*e: constant DBGSCHED */
/*s: constant DBGCHAN */
#define DBGCHAN	(1 << 17)
/*e: constant DBGCHAN */
/*s: constant DBGREND */
#define DBGREND	(1 << 18)
/*e: constant DBGREND */
/*s: constant DBGNOTE */
/* #define DBGKILL	(1 << 19) */
#define DBGNOTE	(1 << 20)
/*e: constant DBGNOTE */
/*s: constant DBGEXEC */
#define DBGEXEC	(1 << 21)
/*e: constant DBGEXEC */

/*s: function ioproc_arg */
#define ioproc_arg(io, type)	(va_arg((io)->arg, type))
/*e: function ioproc_arg */
/*e: lib_core/libthread/threadimpl.h */
