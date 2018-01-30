/*s: rc/exec.h */
/*
 * Definitions used in the interpreter
 */
extern void Xappend(void), Xasync(void), Xbackq(void), Xbang(void), Xclose(void);
extern void Xconc(void), Xcount(void), Xdelfn(void), Xdol(void), Xqdol(void), Xdup(void);
extern void Xexit(void), Xfalse(void), Xfn(void), Xfor(void), Xglob(void);
extern void Xjump(void), Xmark(void), Xmatch(void), Xpipe(void), Xread(void);
extern void Xrdwr(void);
extern void Xrdfn(void), Xreturn(void), Xsubshell(void);
extern void Xtrue(void), Xword(void), Xwrite(void), Xpipefd(void), Xcase(void);
extern void Xlocal(void), Xunlocal(void), Xassign(void), Xsimple(void), Xpopm(void);
extern void Xrdcmds(void), Xwastrue(void), Xif(void), Xifnot(void), Xpipewait(void);
extern void Xdelhere(void), Xpopredir(void), Xsub(void), Xeflag(void), Xsettrue(void);
extern void Xerror(char*);
extern void Xerror1(char*);

/*s: struct [[Redir]] */
struct Redir {
    // enum<redirection_kind_bis>
    char type;	/* what to do */

    /* what to do it to */
    short from;
    short to;

    // Extra
    /*s: [[Redir]] extra fields */
    struct Redir *next;		/* what else to do (reverse order) */
    /*e: [[Redir]] extra fields */
};
/*e: struct [[Redir]] */

/*s: constant [[NSTATUS]] */
#define	NSTATUS	ERRMAX			/* length of status (from plan 9) */
/*e: constant [[NSTATUS]] */
/*s: constant [[ROPEN]] */
/*
 * redir types
 */
#define	ROPEN	1			/* dup2(from, to); close(from); */
/*e: constant [[ROPEN]] */
/*s: constant [[RDUP]] */
#define	RDUP	2			/* dup2(from, to); */
/*e: constant [[RDUP]] */
/*s: constant [[RCLOSE]] */
#define	RCLOSE	3			/* close(from); */
/*e: constant [[RCLOSE]] */

/*s: struct [[List]] */
struct List {
    // list<ref_own<Word>> (next = Word.next)
    word *words;

    // Extra
    list *next;
};
/*e: struct [[List]] */

/*s: struct [[Thread]] */
struct Thread {
    union Code *code;		/* code for this thread */
    int pc;			/* code[pc] is the next instruction */

    /*s: [[Thread]] other fields */
    // list<list<ref_own<word>>> (next = List.next)
    struct List *argv;		/* argument stack */
    /*x: [[Thread]] other fields */
    // list<ref_own<Var>> (next = Var.next)
    struct Var *local;		/* list of local variables */
    /*x: [[Thread]] other fields */
    struct Io *cmdfd;	/* file descriptor for Xrdcmd */
    char *cmdfile;		/* file name in Xrdcmd */
    bool iflag;		/* interactive? */
    /*x: [[Thread]] other fields */
    bool eof;		/* is cmdfd at eof? */
    /*x: [[Thread]] other fields */
    int lineno;			/* linenumber */
    /*x: [[Thread]] other fields */
    // list<ref_own<Redir>> (next = Redir.next)
    struct Redir *redir;	/* redirection stack */
    /*x: [[Thread]] other fields */
    struct Redir *startredir;	/* redir inheritance point */
    /*x: [[Thread]] other fields */
    // option<int> (None = -1)
    int pid;		/* process for Xpipewait to wait for */
    /*x: [[Thread]] other fields */
    char status[NSTATUS];	/* status for Xpipewait */
    /*x: [[Thread]] other fields */
    bool iflast;		/* static `if not' checking */
    /*e: [[Thread]] other fields */

    // Extra
    /*s: [[Thread]] extra fields */
    thread *ret;		/* who continues when this finishes */
    /*e: [[Thread]] extra fields */
};
/*e: struct [[Thread]] */

/*s: struct [[Builtin]] */
struct Builtin {
    char *name;
    void (*fnc)(void);
};
/*e: struct [[Builtin]] */

// globals.c
extern thread *runq;
extern code *codebuf;		/* compiler output */
extern bool eflagok;		/* kludge flag so that -e doesn't exit in startup */
// path.c
extern word nullpath;
// trap.c
extern int ntrap;		/* number of outstanding traps */
// builtins.c
extern struct Builtin builtins[];

// simple.c
void execexec(void);
void execcmds(io *);
// processes.c
int  execforkexec(void);

/*e: rc/exec.h */
