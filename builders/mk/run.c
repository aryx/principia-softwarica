/*s: mk/run.c */
#include	"mk.h"

typedef struct RunEvent RunEvent;
typedef struct Process Process;

int	nextslot(void);
int pidslot(int);
static void sched(void);
static void pnew(int, int);
static void pdelete(Process *);

/*s: struct RunEvent */
struct RunEvent {
    int pid;
    Job *job;
};
/*e: struct RunEvent */

/*s: global events */
static RunEvent *events;
/*e: global events */
/*s: global nevents */
static int nevents;
/*e: global nevents */
/*s: global nrunning */
static int nrunning;
/*e: global nrunning */
/*s: global nproclimit */
static int nproclimit;
/*e: global nproclimit */

/*s: struct Process */
struct Process {
    int pid;
    int status;

    // Extra
    // double linked list, backward, forward
    Process *b, *f;
};
/*e: struct Process */
/*s: global phead */
// double_list<ref_own<Process> (next = Process.f)
static Process *phead;
/*e: global phead */
/*s: global pfree */
// double_list<ref_own<Process> (next = Process.f)
static Process *pfree;
/*e: global pfree */

/*s: function run */
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
/*e: function run */

/*s: function sched */
static void
sched(void)
{
    Job *j;
    int slot;
    Envy *e;

    Bufblock *buf;
    char *flags;
    Node *n;

    if(jobs == nil){
        usage();
        return;
    }

    // j = pop(jobs)
    j = jobs;
    jobs = j->next;
    /*s: [[sched()]] if DEBUG(D_EXEC) */
    if(DEBUG(D_EXEC))
        fprint(STDOUT, "firing up job for target %s\n", wtos(j->t, ' '));
    /*e: [[sched()]] if DEBUG(D_EXEC) */

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

    /*s: [[sched()]] if dry mode or touch mode */
    if(nflag||tflag){
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
    /*e: [[sched()]] if dry mode or touch mode */
    else {
       /*s: [[sched()]] if DEBUG(D_EXEC) print recipe */
       if(DEBUG(D_EXEC))
           fprint(STDOUT, "recipe='%s'\n", j->r->recipe);	/**/
       Bflush(&bout);
       /*e: [[sched()]] if DEBUG(D_EXEC) print recipe */
        flags = (j->r->attr&NOMINUSE)? nil : "-e";
        // launching the job!
        events[slot].pid = execsh(flags, j->r->recipe, 0, e);
        usage();
        nrunning++;
       /*s: [[sched()]] if DEBUG(D_EXEC) print pid */
       if(DEBUG(D_EXEC))
           fprint(STDOUT, "pid for target %s = %d\n", wtos(j->t, ' '), events[slot].pid);
       /*e: [[sched()]] if DEBUG(D_EXEC) print pid */
    }
}
/*e: function sched */

/*s: function waitup */
int
waitup(int echildok, int *retstatus)
{
    Envy *e;
    int pid;
    int slot;
    Symtab *s;
    Word *w;
    Job *j;
    char buf[ERRMAX];
    Bufblock *bp;
    int uarg = 0;
    int done;
    Node *n;
    Process *p;
    extern int runerrs;

    /* first check against the proces slist */
    if(retstatus)
        for(p = phead; p; p = p->f)
            if(p->pid == *retstatus){
                *retstatus = p->status;
                pdelete(p);
                return -1;
            }
again:		/* rogue processes */
    pid = waitfor(buf);
    if(pid == -1){
        if(echildok > 0)
            return 1;
        else {
            fprint(STDERR, "mk: (waitup %d) ", echildok);
            perror("mk wait");
            Exit();
        }
    }
    /*s: [[waitup()]] if DEBUG(D_EXEC) print pid */
    if(DEBUG(D_EXEC))
        fprint(STDOUT, "waitup got pid=%d, status='%s'\n", pid, buf);
    /*e: [[waitup()]] if DEBUG(D_EXEC) print pid */
    if(retstatus && pid == *retstatus){
        *retstatus = buf[0]? 1:0;
        return -1;
    }
    slot = pidslot(pid);
    if(slot < 0){
       /*s: [[waitup()]] if DEBUG(D_EXEC) and slot < 0 */
        if(DEBUG(D_EXEC))
            fprint(STDERR, "mk: wait returned unexpected process %d\n", pid);
       /*e: [[waitup()]] if DEBUG(D_EXEC) and slot < 0 */
        pnew(pid, buf[0]? 1:0);
        goto again;
    }
    j = events[slot].job;
    usage();
    nrunning--;
    events[slot].pid = -1;
    if(buf[0]){
        e = buildenv(j, slot);
        bp = newbuf();
        shprint(j->r->recipe, e, bp);
        front(bp->start);
        fprint(STDERR, "mk: %s: exit status=%s", bp->start, buf);
        freebuf(bp);
        for(n = j->n, done = 0; n; n = n->next)
            if(n->flags&DELETE){
                if(done++ == 0)
                    fprint(STDERR, ", deleting");
                fprint(STDERR, " '%s'", n->name);
                delete(n->name);
            }
        fprint(STDERR, "\n");
        if(kflag){
            runerrs++;
            uarg = 1;
        } else {
            jobs = 0;
            Exit();
        }
    }
    for(w = j->t; w; w = w->next){
        if((s = symlook(w->s, S_NODE, 0)) == 0)
            continue;	/* not interested in this node */
        update(uarg, s->u.ptr);
    }
    if(nrunning < nproclimit)
        sched();
    return 0;
}
/*e: function waitup */

/*s: function nproc */
void
nproc(void)
{
    Symtab *sym;
    Word *w;

    if(sym = symlook("NPROC", S_VAR, nil)) {
        w = sym->u.ptr;
        if (w && w->s && w->s[0])
            nproclimit = atoi(w->s);
    }
    if(nproclimit < 1)
        nproclimit = 1;
    /*s: [[nproc()]] if DEBUG(D_EXEC) */
    if(DEBUG(D_EXEC))
        fprint(1, "nprocs = %d\n", nproclimit);
    /*e: [[nproc()]] if DEBUG(D_EXEC) */
    if(nproclimit > nevents){
        if(nevents)
            events = (RunEvent *)Realloc((char *)events, nproclimit*sizeof(RunEvent));
        else
            events = (RunEvent *)Malloc(nproclimit*sizeof(RunEvent));

        while(nevents < nproclimit)
            events[nevents++].pid = 0;
    }
}
/*e: function nproc */

/*s: function nextslot */
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
/*e: function nextslot */

/*s: function pidslot */
int
pidslot(int pid)
{
    int i;

    for(i = 0; i < nevents; i++)
        if(events[i].pid == pid) 
            return i;
    /*s: [[pidslot()]] if DEBUG(D_EXEC) */
    if(DEBUG(D_EXEC))
        fprint(STDERR, "mk: wait returned unexpected process %d\n", pid);
    /*e: [[pidslot()]] if DEBUG(D_EXEC) */
    return -1;
}
/*e: function pidslot */


/*s: function pnew */
static void
pnew(int pid, int status)
{
    Process *p;

    if(pfree){
        p = pfree;
        pfree = p->f;
    } else
        p = (Process *)Malloc(sizeof(Process));

    p->pid = pid;
    p->status = status;

    p->f = phead;
    phead = p;
    if(p->f)
        p->f->b = p;
    p->b = nil;
}
/*e: function pnew */

/*s: function pdelete */
static void
pdelete(Process *p)
{
    if(p->f)
        p->f->b = p->b;
    if(p->b)
        p->b->f = p->f;
    else
        phead = p->f;
    p->f = pfree;
    pfree = p;
}
/*e: function pdelete */

/*s: function killchildren */
void
killchildren(char *msg)
{
    Process *p;

    kflag = true;	/* to make sure waitup doesn't exit */
    jobs = nil;		/* make sure no more get scheduled */
    for(p = phead; p; p = p->f)
        expunge(p->pid, msg);
    while(waitup(1, (int *)0) == 0)
        ;
    Bprint(&bout, "mk: %s\n", msg);
    Exit();
}
/*e: function killchildren */

/*s: global tslot */
static ulong tslot[1000];
/*e: global tslot */
/*s: global tick */
static ulong tick;
/*e: global tick */

/*s: function usage */
void
usage(void)
{
    ulong t;

    t = time(0);
    if(tick)
        tslot[nrunning] += t - tick;
    tick = t;
}
/*e: function usage */

/*s: function prusage */
void
prusage(void)
{
    int i;

    usage();
    for(i = 0; i <= nevents; i++)
        fprint(1, "%d: %lud\n", i, tslot[i]);
}
/*e: function prusage */
/*e: mk/run.c */
