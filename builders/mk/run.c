/*s: mk/run.c */
#include	"mk.h"

typedef struct RunEvent RunEvent;
typedef struct Process Process;

int	nextslot(void);
int pidslot(int);
void killchildren(char *msg);

static void sched(void);

static void pnew(int, int);
static void pdelete(Process *);

/*s: struct [[RunEvent]] */
struct RunEvent {
    // option<Pid> (None = 0)
    int pid;

    // ref_own<Job>
    Job *job;
};
/*e: struct [[RunEvent]] */

/*s: global [[events]] */
// growing_array<Runevent> (size = nevents (== nproclimit))
static RunEvent *events;
/*e: global [[events]] */
/*s: global [[nevents]] */
static int nevents;
/*e: global [[nevents]] */
/*s: global [[nrunning]] */
static int nrunning;
/*e: global [[nrunning]] */
/*s: global [[nproclimit]] */
static int nproclimit;
/*e: global [[nproclimit]] */

/*s: struct [[Process]] */
struct Process {
    int pid;
    int status;

    // Extra
    // double_list<ref_own<Process> backward, forward
    Process *b, *f;
};
/*e: struct [[Process]] */
/*s: global [[phead]] */
// double_list<ref_own<Process> (next = Process.f)
static Process *phead;
/*e: global [[phead]] */
/*s: global [[pfree]] */
// double_list<ref_own<Process> (next = Process.f)
static Process *pfree;
/*e: global [[pfree]] */

/*s: function [[run]] */
void
run(Job *j)
{
    Job *jj;

    // enqueue(j, jobs)
    if(jobs){
        for(jj = jobs; jj->next; jj = jj->next)
            ;
        jj->next = j;
    } else 
        jobs = j;
    j->next = nil;

    /* this code also in waitup after parse redirect */
    if(nrunning < nproclimit)
        sched();
}
/*e: function [[run]] */

// shell is back in Plan9.c
// shellname is back in Plan9.c

/*s: function [[sched]] */
static void
sched(void)
{
    Job *j;
    int slot;
    char *flags;
    ShellEnvVar *e;
    /*s: [[sched()]] other locals */
    Bufblock *buf;
    /*x: [[sched()]] other locals */
    Node *n;
    /*e: [[sched()]] other locals */

    /*s: [[sched()]] return if no jobs */
    if(jobs == nil){
        usage();
        return;
    }
    /*e: [[sched()]] return if no jobs */

    // j = pop(jobs)
    j = jobs;
    jobs = j->next;
    /*s: [[sched()]] if [[DEBUG(D_EXEC)]] */
    if(DEBUG(D_EXEC))
        fprint(STDOUT, "firing up job for target %s\n", wtos(j->t, ' '));
    /*e: [[sched()]] if [[DEBUG(D_EXEC)]] */

    slot = nextslot();
    events[slot].job = j;

    e = buildenv(j, slot);
    /*s: [[sched()]] print recipe command on stdout */
    buf = newbuf();
    shprint(j->r->recipe, e, buf);
    if(!tflag && (nflag || !(j->r->attr&QUIET)))
        Bwrite(&bout, buf->start, (long)strlen(buf->start));
    freebuf(buf);
    /*e: [[sched()]] print recipe command on stdout */

    /*s: [[sched()]] if dry mode or touch mode, alternate to execsh */
    if(nflag || tflag){
        for(n = j->n; n; n = n->next){
            /*s: [[sched()]] if touch mode */
            if(tflag){
                if(!(n->flags&VIRTUAL))
                    touch(n->name);
                else if(explain)
                    Bprint(&bout, "no touch of virtual '%s'\n", n->name);
            }
            /*e: [[sched()]] if touch mode */
            n->time = time((long *)nil);
            MADESET(n, MADE);
        }
    }
    /*e: [[sched()]] if dry mode or touch mode, alternate to execsh */
    else {
       /*s: [[sched()]] if [[DEBUG(D_EXEC)]] print recipe */
       if(DEBUG(D_EXEC))
           fprint(STDOUT, "recipe='%s'\n", j->r->recipe);
       Bflush(&bout);
       /*e: [[sched()]] if [[DEBUG(D_EXEC)]] print recipe */
        flags = "-e";
       /*s: [[sched()]] reset flags if NOMINUSE rule */
       if (j->r->attr&NOMINUSE)
           flags = nil;
       /*e: [[sched()]] reset flags if NOMINUSE rule */

        // launching the job!
        events[slot].pid = execsh(flags, j->r->recipe, nil, e);

        usage();
        nrunning++;
       /*s: [[sched()]] if [[DEBUG(D_EXEC)]] print pid */
       if(DEBUG(D_EXEC))
           fprint(STDOUT, "pid for target %s = %d\n", wtos(j->t, ' '), events[slot].pid);
       /*e: [[sched()]] if [[DEBUG(D_EXEC)]] print pid */
    }
}
/*e: function [[sched]] */

// execsh() is back in Plan9.c
// xwaitfor() is back in Plan9.c
extern int xwaitfor(char *msg);

/*s: function [[waitup]] */
int
waitup(int echildok, int *retstatus)
{
    // child process
    int pid;
    // return string of child process
    char buf[ERRMAX];
    // index in events[]
    int slot;
    Job *j;
    Symtab *sym;
    Node *node;
    Word *w;
    bool fake = false;
    /*s: [[waitup()]] other locals */
    Bufblock *bp;
    /*x: [[waitup()]] other locals */
    ShellEnvVar *e;
    /*x: [[waitup()]] other locals */
    Node *n;
    bool done;
    /*x: [[waitup()]] other locals */
    Process *p;
    /*e: [[waitup()]] other locals */

    /*s: [[waitup()]] if retstatus, check process list */
    /* first check against the process list */
    if(retstatus)
        for(p = phead; p; p = p->f)
            if(p->pid == *retstatus){
                *retstatus = p->status;
                pdelete(p);
                return -1;
            }
    /*e: [[waitup()]] if retstatus, check process list */
again:		/* rogue processes */

    pid = xwaitfor(buf);
    /*s: [[waitup()]] if no more children */
    if(pid == -1){
        if(echildok == EMPTY_CHILDREN_IS_OK)
            return EMPTY_CHILDREN;
        else {
            fprint(STDERR, "mk: (waitup %d) ", echildok);
            perror("mk wait");
            Exit();
        }
    }
    /*e: [[waitup()]] if no more children */
    /*s: [[waitup()]] if [[DEBUG(D_EXEC)]] print pid */
    if(DEBUG(D_EXEC))
        fprint(STDOUT, "waitup got pid=%d, status='%s'\n", pid, buf);
    /*e: [[waitup()]] if [[DEBUG(D_EXEC)]] print pid */
    /*s: [[waitup()]] if retstatus, check if matching pid */
    if(retstatus && pid == *retstatus){
        *retstatus = buf[0]? 1:0;
        return -1;
    }
    /*e: [[waitup()]] if retstatus, check if matching pid */

    slot = pidslot(pid);
    /*s: [[waitup()]] if slot not found, not a job pid, update process list */
    if(slot < 0){
       /*s: [[waitup()]] if [[DEBUG(D_EXEC)]] and slot < 0 */
        if(DEBUG(D_EXEC))
            fprint(STDERR, "mk: wait returned unexpected process %d\n", pid);
       /*e: [[waitup()]] if [[DEBUG(D_EXEC)]] and slot < 0 */
        pnew(pid, buf[0]? 1:0);
        goto again;
    }
    /*e: [[waitup()]] if slot not found, not a job pid, update process list */

    j = events[slot].job;
    usage();
    nrunning--;
    // free events[slot]
    events[slot].pid = -1;

    /*s: [[waitup()]] if error in child process, possibly set fake or exit */
    if(buf[0]){
        bp = newbuf();
        /*s: [[waitup()]] if error in child process, print recipe in [[bp]] */
        e = buildenv(j, slot);
        shprint(j->r->recipe, e, bp);
        front(bp->start);
        /*e: [[waitup()]] if error in child process, print recipe in [[bp]] */
        fprint(STDERR, "mk: %s: exit status=%s", bp->start, buf);
        freebuf(bp);
        /*s: [[waitup()]] when error in child process, delete if DELETE node */
        for(n = j->n, done = false; n; n = n->next)
            if(n->flags&DELETE){
                if(!done) {
                    fprint(STDERR, ", deleting");
                    done = true;
                }
                fprint(STDERR, " '%s'", n->name);
                delete(n->name);
            }
        /*e: [[waitup()]] when error in child process, delete if DELETE node */
        fprint(STDERR, "\n");

        /*s: [[waitup()]] when error in child process, if kflag */
        if(kflag){
            runerrs++;
            fake = true;
        }
        /*e: [[waitup()]] when error in child process, if kflag */
        else {
            jobs = nil;
            Exit();
        }
    }
    /*e: [[waitup()]] if error in child process, possibly set fake or exit */
    // else

    for(w = j->t; w; w = w->next){
        sym = symlook(w->s, S_NODE, nil);
        node = (Node*) sym->u.ptr;
        /*s: [[waitup()]] skip if node not found */
        if(sym == nil)
            continue;	/* not interested in this node */
        /*e: [[waitup()]] skip if node not found */
        update(node, fake);
    }

    if(nrunning < nproclimit)
        sched();
    return JOB_ENDED;
}
/*e: function [[waitup]] */

/*s: function [[nproc]] */
void
nproc(void)
{
    Symtab *sym;
    Word *w;

    if(sym = symlook("NPROC", S_VAR, nil)) {
        w = sym->u.ptr;
        if (!empty_words(w))
            nproclimit = atoi(w->s);
    }
    if(nproclimit < 1)
        nproclimit = 1;
    /*s: [[nproc()]] if [[DEBUG(D_EXEC)]] */
    if(DEBUG(D_EXEC))
        fprint(STDERR, "nprocs = %d\n", nproclimit);
    /*e: [[nproc()]] if [[DEBUG(D_EXEC)]] */

    /*s: [[nproc()]] grow nevents if necessary */
    if(nproclimit > nevents){
        if(nevents)
            events = (RunEvent *)Realloc((char *)events, 
                                         nproclimit*sizeof(RunEvent));
        else
            events = (RunEvent *)Malloc(nproclimit*sizeof(RunEvent));

        while(nevents < nproclimit)
            events[nevents++].pid = 0;
    }
    /*e: [[nproc()]] grow nevents if necessary */
}
/*e: function [[nproc]] */

/*s: function [[nextslot]] */
int
nextslot(void)
{
    int i;

    for(i = 0; i < nproclimit; i++)
        if(events[i].pid <= 0) 
            return i;
    assert(/*out of slots!!*/ false);
    return 0;	/* cyntax */
}
/*e: function [[nextslot]] */

/*s: function [[pidslot]] */
int
pidslot(int pid)
{
    int i;

    for(i = 0; i < nevents; i++)
        if(events[i].pid == pid) 
            return i;
    // else
    /*s: [[pidslot()]] if [[DEBUG(D_EXEC)]] */
    if(DEBUG(D_EXEC))
        fprint(STDERR, "mk: wait returned unexpected process %d\n", pid);
    /*e: [[pidslot()]] if [[DEBUG(D_EXEC)]] */
    return -1;
}
/*e: function [[pidslot]] */


/*s: function [[pnew]] */
static void
pnew(int pid, int status)
{
    Process *p;

    // p = pop_list(pfree)
    if(pfree){
        p = pfree;
        pfree = p->f;
    } else
        p = (Process *)Malloc(sizeof(Process));

    p->pid = pid;
    p->status = status;

    // add_list(p, phead)
    p->f = phead;
    phead = p;
    if(p->f)
        p->f->b = p;
    p->b = nil;
}
/*e: function [[pnew]] */

/*s: function [[pdelete]] */
static void
pdelete(Process *p)
{
    // remove_double_list(p, phead, pfree)
    if(p->f)
        p->f->b = p->b;
    if(p->b)
        p->b->f = p->f;
    else
        phead = p->f;
    p->f = pfree;
    pfree = p;
}
/*e: function [[pdelete]] */

// Exit() is back in Plan9.c
// notifyf() is back in Plan9.c
// catchnotes() is back in Plan9.c
// expunge() is back in Plan9.c
extern void expunge(int pid, char *msg);

/*s: function [[killchildren]] */
void
killchildren(char *msg)
{
    /*s: [[killchildren()]] locals */
    Process *p;
    /*e: [[killchildren()]] locals */

    jobs = nil;		/* make sure no more get scheduled */
    kflag = true;	/* to make sure waitup doesn't exit */

    /*s: [[killchildren()]] expunge not-job processes */
    for(p = phead; p; p = p->f)
        expunge(p->pid, msg);
    /*e: [[killchildren()]] expunge not-job processes */

    while(waitup(EMPTY_CHILDREN_IS_OK, (int *)nil) == JOB_ENDED)
        ;
    Bprint(&bout, "mk: %s\n", msg);
    Exit();
}
/*e: function [[killchildren]] */

/*s: global [[tslot]] */
// map<nrunning, int>
static ulong tslot[1000];
/*e: global [[tslot]] */
/*s: global [[tick]] */
static ulong tick;
/*e: global [[tick]] */

/*s: function [[usage]] */
void
usage(void)
{
    ulong t;

    t = time(nil);
    if(tick)
        tslot[nrunning] += t - tick;
    tick = t;
}
/*e: function [[usage]] */

/*s: function [[prusage]] */
void
prusage(void)
{
    int i;

    usage();
    for(i = 0; i <= nevents; i++)
        fprint(STDOUT, "%d: %lud\n", i, tslot[i]);
}
/*e: function [[prusage]] */

// pipecmd() is back in Plan9.c
/*e: mk/run.c */
