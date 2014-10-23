/*s: rc/exec.h */
/*
 * Definitions used in the interpreter
 */
extern void Xappend(void), Xasync(void), Xbackq(void), Xbang(void), Xclose(void);
extern void Xconc(void), Xcount(void), Xdelfn(void), Xdol(void), Xqdol(void), Xdup(void);
extern void Xexit(void), Xfalse(void), Xfn(void), Xfor(void), Xglob(void);
extern void Xjump(void), Xmark(void), Xmatch(void), Xpipe(void), Xread(void);
extern void Xrdwr(void);
extern void Xrdfn(void), Xunredir(void), Xstar(void), Xreturn(void), Xsubshell(void);
extern void Xtrue(void), Xword(void), Xwrite(void), Xpipefd(void), Xcase(void);
extern void Xlocal(void), Xunlocal(void), Xassign(void), Xsimple(void), Xpopm(void);
extern void Xrdcmds(void), Xwastrue(void), Xif(void), Xifnot(void), Xpipewait(void);
extern void Xdelhere(void), Xpopredir(void), Xsub(void), Xeflag(void), Xsettrue(void);
extern void Xerror(char*);
extern void Xerror1(char*);
/*s: struct word */
/*
 * word lists are in correct order,
 * i.e. word0->word1->word2->word3->0
 */
struct Word {
    char *word;
    word *next;
};
/*e: struct word */
/*s: struct list */
struct List {
    word *words;
    list *next;
};
/*e: struct list */
word *newword(char *, word *), *copywords(word *, word *);
/*s: struct redir */
struct Redir {
    char type;			/* what to do */
    short from, to;			/* what to do it to */
    struct Redir *next;		/* what else to do (reverse order) */
};
/*e: struct redir */
/*s: constant NSTATUS */
#define	NSTATUS	ERRMAX			/* length of status (from plan 9) */
/*e: constant NSTATUS */
/*s: constant ROPEN */
/*
 * redir types
 */
#define	ROPEN	1			/* dup2(from, to); close(from); */
/*e: constant ROPEN */
/*s: constant RDUP */
#define	RDUP	2			/* dup2(from, to); */
/*e: constant RDUP */
/*s: constant RCLOSE */
#define	RCLOSE	3			/* close(from); */
/*e: constant RCLOSE */
/*s: struct thread */
struct Thread {
    union Code *code;		/* code for this thread */
    int pc;				/* code[pc] is the next instruction */
    struct List *argv;		/* argument stack */
    struct Redir *redir;		/* redirection stack */
    struct Redir *startredir;	/* redir inheritance point */
    struct Var *local;		/* list of local variables */
    char *cmdfile;			/* file name in Xrdcmd */
    struct Io *cmdfd;		/* file descriptor for Xrdcmd */
    int iflast;			/* static `if not' checking */
    int eof;			/* is cmdfd at eof? */
    int iflag;			/* interactive? */
    int lineno;			/* linenumber */
    int pid;			/* process for Xpipewait to wait for */
    char status[NSTATUS];		/* status for Xpipewait */
    tree *treenodes;		/* tree nodes created by this process */
    thread *ret;		/* who continues when this finishes */
};
/*e: struct thread */
/*s: global runq */
thread *runq;
/*e: global runq */
code *codecopy(code*);
/*s: global codebuf */
code *codebuf;				/* compiler output */
/*e: global codebuf */
/*s: global ntrap */
int ntrap;				/* number of outstanding traps */
/*e: global ntrap */
/*s: global trap */
int trap[NSIG];				/* number of outstanding traps per type */
/*e: global trap */
/*s: struct builtin */
struct Builtin {
    char *name;
    void (*fnc)(void);
};
/*e: struct builtin */
extern struct Builtin Builtin[];
/*s: global eflagok */
int eflagok;			/* kludge flag so that -e doesn't exit in startup */
/*e: global eflagok */
/*s: global havefork */
int havefork;
/*e: global havefork */

void execcd(void), execwhatis(void), execeval(void), execexec(void);
int execforkexec(void);
void execexit(void), execshift(void);
void execwait(void), execumask(void), execdot(void), execflag(void);
void execfunc(var*), execcmds(io *);
/*e: rc/exec.h */
