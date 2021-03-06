/*s: proc.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include    <trace.h>

//*****************************************************************************
// Globals
//*****************************************************************************

/*s: global [[runq]] */
// The run queue!!
// hash<enum<priority>, queue<ref<Proc>>>
Schedq  runq[Nrq];
/*e: global [[runq]] */
/*s: global [[runveq]] */
// array<bool>, each bit i represents whether the runq at pri i has processes
ulong   runvec; // coupling: sizeof(ulong) must be >= Nrq
/*e: global [[runveq]] */

/*s: global [[procalloc]] */
static struct Procalloc procalloc;
/*e: global [[procalloc]] */

/*s: global [[nrdy]] */
int nrdy;
/*e: global [[nrdy]] */
/*s: global [[noteidalloc]] */
// also used by sysrfork()
Counter noteidalloc;
/*e: global [[noteidalloc]] */

/*s: proc.c statistics */
long preempts;
ulong delayedscheds;    /* statistics */
long skipscheds;
/*e: proc.c statistics */

/*s: global [[pidalloc]] */
static Counter  pidalloc;
/*e: global [[pidalloc]] */

enum
{
    /*s: constant [[Schedagain]] */
    schedgain = 30, /* units in seconds */
    /*e: constant [[Schedagain]] */
    /*s: constant [[Scaling]] */
    Scaling=2,
    /*e: constant [[Scaling]] */
};

/*s: global [[statename]] */
// hash<enum<Procstate>, string>, coupling: with enum Procstate
char *statename[] =
{
    [Dead]        = "Dead",
    [Running]     = "Running",
    [Queueing]    = "Queueing",
    [QueueingR]   = "QueueingR",
    [QueueingW]   = "QueueingW",
    [Moribund]    = "Moribund",
    [Ready]       = "Ready",
    [Scheding]    = "Scheding",
    [Wakeme]      = "Wakeme",
    [Broken]      = "Broken",
    [Stopped]     = "Stopped",
    [Rendezvous]  = "Rendezvous",
    [Waitrelease] = "Waitrelease",
};
/*e: global [[statename]] */

/*s: proc.c forward decl */
Proc* runproc(void);
void updatecpu(Proc*);
int reprioritize(Proc*);

static void pidhash(Proc*);
static void pidunhash(Proc*);
static void rebalance(void);
/*e: proc.c forward decl */

//*****************************************************************************
// Error management
//*****************************************************************************

// see also waserror() and poperror() macro in portdat_processes.h

/*s: function [[error]] */
void
proc_error(char *err)
{
    arch_spllo();

    assert(up->nerrlab < NERR);
    kstrcpy(up->errstr, err, ERRMAX);
    arch_setlabel(&up->errlab[NERR-1]);
    nexterror();
}
/*e: function [[error]] */

/*s: function [[nexterror]] */
// raise an exception
void
proc_nexterror(void)
{
    arch_gotolabel(&up->errlab[--up->nerrlab]);
}
/*e: function [[nexterror]] */

/*s: function [[exhausted]] */
void
exhausted(char *resource)
{
    char buf[ERRMAX];

    snprint(buf, sizeof buf, "no free %s", resource);
    iprint("%s\n", buf);
    error(buf);
}
/*e: function [[exhausted]] */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: function [[schedinit]] */
/*
 * Always arch_splhi()'ed.
 */
void
schedinit(void)     /* never returns */
{
    /*s: [[schedinit()]] locals */
    Edf *e;
    /*e: [[schedinit()]] locals */

    arch_setlabel(&cpu->sched);
    /*s: [[schedinit()]] after setlabel, if up */
    if(up) {
        /*s: [[schedinit()]] optional real-time [[edfrecord()]] */
        if((e = up->edf) && (e->flags & Admitted))
            edfrecord(up);
        /*e: [[schedinit()]] optional real-time [[edfrecord()]] */
        //old: cpu->proc = nil;
        // but now that on x86 up = cpu->proc and not cpu->externup
        // we can't do that anymore. Is there a place that rely on
        // cpu->proc and cpu->externup to not be in sync?
        switch(up->state) {
        /*s: [[schedinit()]] switch process state cases */
        case Moribund:
            up->state = Dead;
            /*s: [[schedinit()]] optional real-time [[edfstop()]] */
            edfstop(up);
            if (up->edf)
                free(up->edf);
            up->edf = nil;
            /*e: [[schedinit()]] optional real-time [[edfstop()]] */
            /*
             * Holding locks from pexit:
             *  procalloc
             *  palloc
             */
            arch_mmurelease(up);

            up->qnext = procalloc.free;
            procalloc.free = up;

            unlock(&palloc);
            unlock(&procalloc);
            break;
        /*x: [[schedinit()]] switch process state cases */
        case Running:
            ready(up);
            break;
        /*e: [[schedinit()]] switch process state cases */
        }
        up->cpu = nil;
        /*s: [[schedinit()]] updatecpu before reseting up */
        updatecpu(up);
        /*e: [[schedinit()]] updatecpu before reseting up */
        cpu->proc = nil;
        up = nil; // same instruction than previous line on some archi (e.g. PC)
    }
    /*e: [[schedinit()]] after setlabel, if up */
    // ok at this point up is nil
    sched();
}
/*e: function [[schedinit]] */

/*s: function [[sched]] */
/*
 *  If changing this routine, look also at sleep().  It
 *  contains a copy of the guts of sched().
 */
void
proc_sched(void)
{
    /*s: [[sched()]] locals */
    Proc *p;
    /*e: [[sched()]] locals */

    /*s: [[sched()]] sanity check ilockdepth */
    if(cpu->ilockdepth)
        panic("cpu%d: ilockdepth %d, last lock %#p at %#p, sched called from %#p",
            cpu->cpuno, cpu->ilockdepth, up? up->lastilock: nil,
            (up && up->lastilock)? up->lastilock->pc: 0, getcallerpc(&p+2));
    /*e: [[sched()]] sanity check ilockdepth */
    /*s: [[sched()]] if up */
    if(up){
        /*s: [[sched()]] when up, possibly increment delaysched and return */
        /*
         * Delay the sched until the process gives up the locks
         * it is holding.  This avoids dumb lock loops.
         *
         * But don't delay if the process is Moribund.
         * It called sched to die.
         * But do sched eventually. This avoids a missing unlock
         * from hanging the entire kernel. 
         * But don't reschedule procs holding palloc or procalloc.
         * Those are far too important to be holding while asleep.
         *
         * This test is not exact.  There can still be a few instructions
         * in the middle of taslock when a process holds a lock
         * but Lock.p has not yet been initialized.
         */
        if(up->nlocks.ref)
          if(up->state != Moribund)
            if(up->delaysched < 20
              || palloc.Lock.p == up
              || procalloc.Lock.p == up){

                up->delaysched++;
                delayedscheds++;
                return;
            }
        // else
        up->delaysched = 0;
        /*e: [[sched()]] when up, possibly increment delaysched and return */
        arch_splhi(); // schedinit requires this (also sensitive moment)
        cpu->cs++;

        arch_procsave(up);
        if(arch_setlabel(&up->sched)){
            //
            // here when the process has been scheduled back
            // from a arch_gotolabel(up->sched) by another process, see below
            //
            arch_procrestore(up);
            arch_spllo();
            return;
        }else{
            //
            // here to go to schedinit() (which will call sched() back)
            //
            arch_gotolabel(&cpu->sched); // goto schedinit()
            panic("sched: should never reach this point");
        }
    }
    /*e: [[sched()]] if up */
    // else
    /*s: [[sched()]] if no up */
    // We should execute this code using the main kernel stack, as
    // we should arrive here from schedinit()

    p = runproc();
    /*s: [[sched()]] after found process [[p]] to schedule, possibly update priority */
    /*s: [[sched()]] if real-time process then do nothing, do not reprioritize */
    if(p->edf) {
    }
    /*e: [[sched()]] if real-time process then do nothing, do not reprioritize */
    else {
        updatecpu(p);
        p->priority = reprioritize(p);
    }
    /*e: [[sched()]] after found process [[p]] to schedule, possibly update priority */
    /*s: [[sched()]] if readied process, do not adjust schedticks */
    if(p == cpu->readied) {
    }
    /*e: [[sched()]] if readied process, do not adjust schedticks */
    else {
        /*s: [[sched()]] after found a new proc, adjust schedticks */
        cpu->schedticks = cpu->ticks + Arch_HZ/10; // 100ms of allocated time
        /*e: [[sched()]] after found a new proc, adjust schedticks */
    }
    /*s: [[sched()]] reset readied */
    cpu->readied = nil;
    /*e: [[sched()]] reset readied */

    cpu->proc = p;
    up = p; // same instruction than previous line on some archi (e.g. x86)

    up->state = Running;
    up->cpu = CPUS(cpu->cpuno);

    arch_mmuswitch(up);
    arch_gotolabel(&up->sched);
    /*e: [[sched()]] if no up */
}
/*e: function [[sched]] */

/*s: function [[anyready]] */
bool
anyready(void)
{
    return runvec;
}
/*e: function [[anyready]] */

/*s: function [[anyhigher]] */
int
anyhigher(void)
{
    return runvec & ~((1<<(up->priority+1))-1);
}
/*e: function [[anyhigher]] */

/*s: function [[hzsched]] */
/*
 *  here once per clock tick to see if we should resched
 */
void
hzsched(void)
{
    /*s: [[hzsched()]] rebalance ready procs */
    /* once a second, rebalance will reprioritize ready procs */
    if(cpu->cpuno == 0)
        rebalance();
    /*e: [[hzsched()]] rebalance ready procs */
    /* unless preempted, get to run for at least 100ms */
    if(anyhigher()
    || (!up->fixedpri && cpu->ticks > cpu->schedticks && anyready())){

        cpu->readied = nil;   /* avoid cooperative scheduling */
        up->delaysched++;
    }
}
/*e: function [[hzsched]] */

/*s: function [[preempt]] */
/*
 *  here at the end of non-clock interrupts to see if we should preempt the
 *  current process.
 */
void
preempt(void)
{
    if(up && up->state == Running)
      if(up->preempted == false)
        if(anyhigher())
          if(!active.exiting){
              cpu->readied = nil;   /* avoid cooperative scheduling */
              up->preempted = true;
              sched();
              arch_splhi(); // still in interrupt context
              up->preempted = false;
          }
    return;
}
/*e: function [[preempt]] */

/*s: function [[updatecpu]] */
/*
 * Update the cpu time average for this particular process,
 * which is about to change from up -> not up or vice versa.
 * p->lastupdate is the last time an updatecpu happened.
 *
 * The cpu time average is a decaying average that lasts
 * about D clock ticks.  D is chosen to be approximately
 * the cpu time of a cpu-intensive "quick job".  A job has to run
 * for approximately D clock ticks before we home in on its 
 * actual cpu usage.  Thus if you manage to get in and get out
 * quickly, you won't be penalized during your burst.  Once you
 * start using your share of the cpu for more than about D
 * clock ticks though, your p->cpu hits 1000 (1.0) and you end up 
 * below all the other quick jobs.  Interactive tasks, because
 * they basically always use less than their fair share of cpu,
 * will be rewarded.
 *
 * If the process has not been running, then we want to
 * apply the filter
 *
 *  cpu = cpu * (D-1)/D
 *
 * n times, yielding 
 * 
 *  cpu = cpu * ((D-1)/D)^n
 *
 * but D is big enough that this is approximately 
 *
 *  cpu = cpu * (D-n)/D
 *
 * so we use that instead.
 * 
 * If the process has been running, we apply the filter to
 * 1 - cpu, yielding a similar equation.  Note that cpu is 
 * stored in fixed point (* 1000).
 *
 * Updatecpu must be called before changing up, in order
 * to maintain accurate cpu usage statistics.  It can be called
 * at any time to bring the stats for a given proc up-to-date.
 */
void
updatecpu(Proc *p)
{
    int n, t, ocpu;
    int D = schedgain * Arch_HZ * Scaling;

    /*s: [[updatecpu()]] return if real-time process */
    if(p->edf)
        return;
    /*e: [[updatecpu()]] return if real-time process */

    t = CPUS(0)->ticks*Scaling + Scaling/2;
    n = t - p->lastupdate;
    p->lastupdate = t;

    if(n == 0)
        return;
    if(n > D)
        n = D;

    ocpu = p->cpuavg;
    if(p != up)
        p->cpuavg = (ocpu*(D-n))/D;
    else{
        t = 1000 - ocpu;
        t = (t*(D-n))/D;
        p->cpuavg = 1000 - t;
    }

}
/*e: function [[updatecpu]] */

/*s: function [[reprioritize]] */
/*
 * On average, p has used p->cpuavg of a cpu recently.
 * Its fair share is conf.ncpu/cpu->load of a cpu.  If it has been getting
 * too much, penalize it.  If it has been getting not enough, reward it.
 * I don't think you can get much more than your fair share that 
 * often, so most of the queues are for using less.  Having a priority
 * of 3 means you're just right.  Having a higher priority (up to p->basepri) 
 * means you're not using as much as you could.
 */
int
reprioritize(Proc *p)
{
    int fairshare, n, load, ratio;

    load = CPUS(0)->load;
    if(load == 0)
        return p->basepri;

    /*
     *  fairshare = 1.000 * conf.nproc * 1.000/load,
     * except the decimal point is moved three places
     * on both load and fairshare.
     */
    fairshare = (conf.ncpu*1000*1000)/load;
    n = p->cpuavg;
    if(n == 0)
        n = 1;
    ratio = (fairshare+n/2) / n;
    if(ratio > p->basepri)
        ratio = p->basepri;
    if(ratio < 0)
        panic("reprioritize");
//iprint("pid %d cpu %d load %d fair %d pri %d\n", p->pid, p->cpuavg, load, fairshare, ratio);
    return ratio;
}
/*e: function [[reprioritize]] */

/*s: function [[queueproc]] */
/*
 * add a process to a scheduling queue
 */
void
queueproc(Schedq *rq, Proc *p)
{
    int pri;

    pri = rq - runq;
    lock(runq);
    p->priority = pri; // because a priority can evolve

    // add_queue(p, rq)
    p->rnext = nil;
    if(rq->tail)
        rq->tail->rnext = p;
    else
        rq->head = p;
    rq->tail = p;
    rq->n++;

    nrdy++;
    runvec |= 1<<pri;
    unlock(runq);
}
/*e: function [[queueproc]] */

/*s: function [[dequeueproc]] */
/*
 *  try to remove a process from a scheduling queue (called arch_splhi)
 */
Proc*
dequeueproc(Schedq *rq, Proc *tp)
{
    Proc *l, *p;

    if(!canlock(runq))
        return nil;

    /*
     *  the queue may have changed before we locked runq,
     *  refind the target process.
     */
    l = nil;
    for(p = rq->head; p; p = p->rnext){
        if(p == tp)
            break;
        l = p;
    }
    /*s: [[dequeueproc()]] return if process not found or already associated */
    /*
     *  p->cpu==nil only when process state is saved
     */
    if(p == nil || p->cpu){
        unlock(runq);
        return nil;
    }
    /*e: [[dequeueproc()]] return if process not found or already associated */

    // remove_queue(p, rq)
    if(p->rnext == nil)
        rq->tail = l;
    if(l)
        l->rnext = p->rnext;
    else
        rq->head = p->rnext;

    if(rq->head == nil)
        runvec &= ~(1<<(rq-runq));
    rq->n--;
    nrdy--;
    /*s: [[dequeueproc()]] sanity check p */
    if(p->state != Ready)
        print("dequeueproc %s %lud %s\n", p->text, p->pid, statename[p->state]);
    /*e: [[dequeueproc()]] sanity check p */
    unlock(runq);
    return p;
}
/*e: function [[dequeueproc]] */

/*s: function [[ready]] */
/*
 *  ready(p) picks a new priority for a process and sticks it in the
 *  runq for that priority.
 */
void
proc_ready(Proc *p)
{
    int pri;
    int s; // spl
    Schedq *rq;
    /*s: [[ready()]] other locals */
    void (*pt)(Proc*, int, vlong);
    /*e: [[ready()]] other locals */

    s = arch_splhi();
    /*s: [[ready()]] optional [[edfready()]] for real-time scheduling */
    if(edfready(p)){
        arch_splx(s);
        return;
    }
    /*e: [[ready()]] optional [[edfready()]] for real-time scheduling */

    /*s: [[ready()]] possibly set [[cpu->readied]] */
    if(up != p && (p->wired == nil || p->wired == CPUS(cpu->cpuno))) // pad fix
        cpu->readied = p; /* group scheduling */
    /*e: [[ready()]] possibly set [[cpu->readied]] */
    /*s: [[ready()]] update priority */
    updatecpu(p);
    pri = reprioritize(p);
    p->priority = pri;
    /*e: [[ready()]] update priority */

    rq = &runq[pri];
    p->state = Ready;
    queueproc(rq, p);
    /*s: [[ready()]] hook proctrace */
    pt = proctrace;
    if(pt)
        pt(p, SReady, 0);
    /*e: [[ready()]] hook proctrace */
    arch_splx(s);
}
/*e: function [[ready]] */

/*s: function [[yield]] */
/*
 *  yield the processor and drop our priority
 */
void
yield(void)
{
    if(anyready()){
        /*s: [[yield()]] adjust lastupdate before scheduling another process */
        /* pretend we just used 1/2 tick */
        up->lastupdate -= Scaling/2;  
        /*e: [[yield()]] adjust lastupdate before scheduling another process */
        sched();
    }
}
/*e: function [[yield]] */

/*s: global [[balancetime]] */
/*
 *  recalculate priorities once a second.  We need to do this
 *  since priorities will otherwise only be recalculated when
 *  the running process blocks.
 */
ulong balancetime;
/*e: global [[balancetime]] */

/*s: function [[rebalance]] */
static void
rebalance(void)
{
    int pri, npri, t, x;
    Schedq *rq;
    Proc *p;

    /*s: [[rebalance()]] return if less than one second since last rebalance */
    t = cpu->ticks;
    if(t - balancetime < Arch_HZ)
        return;
    balancetime = t;
    /*e: [[rebalance()]] return if less than one second since last rebalance */

    for(pri=0, rq=runq; pri<Npriq; pri++, rq++){
another:
        p = rq->head;
        if(p == nil)
            continue;
        if(p->lastcpu != CPUS(cpu->cpuno)) // this has been the source of comments on 9fans, todo look if bug (subject = rebalance() funny, nov30)
            continue;
        if(pri == p->basepri)
            continue;
        updatecpu(p);
        npri = reprioritize(p);
        if(npri != pri){
            x = arch_splhi();
            p = dequeueproc(rq, p);
            if(p)
                queueproc(&runq[npri], p);
            arch_splx(x);
            goto another;
        }
    }
}
/*e: function [[rebalance]] */
    
/*s: function [[runproc]] */
/*
 *  pick a process to run
 */
Proc*
runproc(void)
{
    Proc *p;
    Schedq *rq;
    int i;
    /*s: [[runproc()]] other locals */
    void (*pt)(Proc*, int, vlong);
    /*x: [[runproc()]] other locals */
    ulong start, now;
    /*e: [[runproc()]] other locals */

    /*s: [[runproc()]] start */
    start = arch_perfticks();
    /*e: [[runproc()]] start */
    /*s: [[runproc()]] cooperative scheduling if process readied */
    /* cooperative scheduling until the clock ticks */
    p=cpu->readied;
    if(p && p->cpu==nil && p->state==Ready && 
      (p->wired == nil || p->wired == CPUS(cpu->cpuno)) && // pad's bugfix!
      /*s: [[runproc()]] test for empty real-time scheduling queue */
      runq[Nrq-1].head == nil && runq[Nrq-2].head == nil
      /*e: [[runproc()]] test for empty real-time scheduling queue */
    ){
        skipscheds++;
        rq = &runq[p->priority];
        goto found;
    }
    /*e: [[runproc()]] cooperative scheduling if process readied */
    //else

    preempts++;

loop:
    /*
     *  find a process that last ran on this processor (affinity),
     *  or one that hasn't moved in a while (load balancing). Every
     *  time around the loop affinity goes down.
     */
    arch_spllo(); // from schedinit()

    for(i = 0;; i++){
        /*
         *  find the highest priority target process that this
         *  processor can run given affinity constraints.
         *
         */
        for(rq = &runq[Nrq-1]; rq >= runq; rq--){
            for(p = rq->head; p; p = p->rnext){
                if(p->lastcpu == nil || p->lastcpu == CPUS(cpu->cpuno)
                   || (p->wired == nil && i > 0)) // favor affinity for first round
                    goto found;
            }
        }
        // nothing found
        /* waste time or halt the CPU */
        arch_idlehands();
        /*s: [[runproc()]] after [[arch_idlehands]], before restarting loop */
        /* remember how much time we're here */
        now = arch_perfticks();
        cpu->perf.inidle += now-start;
        start = now;
        /*e: [[runproc()]] after [[arch_idlehands]], before restarting loop */
    }

found:
    arch_splhi();
    p = dequeueproc(rq, p);
    if(p == nil) // the runq may have changed between spllo and splhi
        goto loop;

    p->state = Scheding;
    p->lastcpu = CPUS(cpu->cpuno);

    /*s: [[runproc()]] test if p is a real-time process */
    if(edflock(p)){
        edfrun(p, rq == &runq[PriEdf]); /* start deadline timer and do admin */
        edfunlock();
    }
    /*e: [[runproc()]] test if p is a real-time process */
    /*s: [[runproc()]] hook proctrace */
    pt = proctrace;
    if(pt)
        pt(p, SRun, 0);
    /*e: [[runproc()]] hook proctrace */
    return p;
}
/*e: function [[runproc]] */

/*s: function [[canpage]] */
int
canpage(Proc *p)
{
    bool ok = false;

    arch_splhi();
    lock(runq);
    /* Only reliable way to see if we are Running */
    if(p->cpu == nil) {
        p->newtlb = true;
        ok = true;
    }
    unlock(runq);
    arch_spllo();

    return ok;
}
/*e: function [[canpage]] */

/*s: function [[noprocpanic]] */
void
noprocpanic(char *msg)
{
    /*
     * setting exiting will make hzclock() on each processor call exit(0).
     * clearing our bit in cpus avoids calling exit(0) from hzclock()
     * on this processor.
     */
    lock(&active);
    active.cpus &= ~(1<<cpu->cpuno);
    active.exiting = true;
    unlock(&active);

    procdump();
    arch_delay(1000);
    panic(msg);
}
/*e: function [[noprocpanic]] */

/*s: constructor [[newproc]] */
Proc*
newproc(void)
{
    Proc *p;
    /*s: [[newproc()]] other locals */
    char msg[64];
    /*e: [[newproc()]] other locals */

    lock(&procalloc);
    // =~ p = procalloc.free
    /*s: [[newproc()]] set [[p]] to [[procalloc.free]] and wait if nil */
    while((p = procalloc.free) == nil) {
        unlock(&procalloc);

        snprint(msg, sizeof msg, "no procs; %s forking", up?up->text:"kernel");
        /*
         * the situation is unlikely to heal itself.
         * dump the proc table and restart by default.
         * *noprocspersist in plan9.ini will yield the old
         * behaviour of trying forever.
         */
        if(getconf("*noprocspersist") == nil)
            noprocpanic(msg);
        resrcwait(msg);
        lock(&procalloc);
    }
    /*e: [[newproc()]] set [[p]] to [[procalloc.free]] and wait if nil */
    procalloc.free = p->qnext;
    unlock(&procalloc);
    p->qnext = nil;

    p->state = Scheding;
    p->pid = incref(&pidalloc);
    /*s: [[newproc()]] sanity check pid */
    if(p->pid==0)
        panic("pidalloc");
    /*e: [[newproc()]] sanity check pid */
    pidhash(p);

    /*s: [[newproc()]] set fields of [[p]] */
    memset(p->seg, nilptr, sizeof p->seg);
    /*x: [[newproc()]] set fields of [[p]] */
    p->psstate = "New";
    kstrdup(&p->text, "*notext");
    /*x: [[newproc()]] set fields of [[p]] */
    p->fgrp = nil;
    p->pgrp = nil;
    p->egrp = nil;
    p->rgrp = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    kstrdup(&p->user, "*nouser");
    /*x: [[newproc()]] set fields of [[p]] */
    kstrdup(&p->args, "");
    p->nargs = 0;
    /*x: [[newproc()]] set fields of [[p]] */
    p->parent = nil;
    p->nchild = 0;
    /*x: [[newproc()]] set fields of [[p]] */
    p->waitq = nil;
    p->nwait = 0;
    /*x: [[newproc()]] set fields of [[p]] */
    p->kp = false;
    /*x: [[newproc()]] set fields of [[p]] */
    p->pdbg = nil;
    p->procctl = Proc_nothing;
    p->syscalltrace = nil; 
    p->trace = false;
    p->dbgreg = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    p->noswap = false;
    /*x: [[newproc()]] set fields of [[p]] */
    /* sched params */
    procpriority(p, PriNormal, false);
    /*x: [[newproc()]] set fields of [[p]] */
    p->cpu = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    p->lastcpu = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    if(p->kstack == nil)
        p->kstack = smalloc(KSTACK);
    /*x: [[newproc()]] set fields of [[p]] */
    p->nlocks.ref = 0;
    p->delaysched = 0;
    /*x: [[newproc()]] set fields of [[p]] */
    p->wired = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    p->cpuavg = 0;
    p->lastupdate = CPUS(0)->ticks * Scaling;
    /*x: [[newproc()]] set fields of [[p]] */
    p->noteid = incref(&noteidalloc);
    if(p->noteid==0)
        panic("pidalloc");
    /*x: [[newproc()]] set fields of [[p]] */
    p->notepending = false;
    p->notified = false;
    /*x: [[newproc()]] set fields of [[p]] */
    p->ureg = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    /*s: [[newproc()]] fp init */
    p->fpstate = FPinit;
    /*e: [[newproc()]] fp init */
    /*x: [[newproc()]] set fields of [[p]] */
    p->edf = nil;
    /*x: [[newproc()]] set fields of [[p]] */
    p->privatemem = false;
    /*x: [[newproc()]] set fields of [[p]] */
    p->setargs = false;
    /*x: [[newproc()]] set fields of [[p]] */
    p->nerrlab = 0;
    p->errstr = p->errbuf0;
    p->syserrstr = p->errbuf1;
    p->errbuf0[0] = '\0';
    p->errbuf1[0] = '\0';
    /*e: [[newproc()]] set fields of [[p]] */

    return p;
}
/*e: constructor [[newproc]] */

/*s: function [[procwired]] */
/*
 * wire this proc to a processor
 */
void
procwired(Proc *p, int bm)
{
    Proc *pp;
    int i;
    char nwired[MAXCPUS];
    Cpu *wm;

    if(bm < 0){
        /* pick a processor to wire to */
        memset(nwired, 0, sizeof(nwired));
        p->wired = nil;
        pp = proctab(0);
        for(i=0; i<conf.nproc; i++, pp++){
            wm = pp->wired;
            if(wm && pp->pid)
                nwired[wm->cpuno]++;
        }
        bm = 0;
        for(i=0; i<conf.ncpu; i++)
            if(nwired[i] < nwired[bm])
                bm = i;
    } else {
        /* use the virtual processor requested */
        bm = bm % conf.ncpu;
    }

    p->wired = CPUS(bm);
    p->lastcpu = p->wired;
}
/*e: function [[procwired]] */

/*s: function [[procpriority]] */
void
procpriority(Proc *p, int pri, bool fixed)
{
    /*s: [[procpriority()]] sanity check pri */
    if(pri >= Npriq)
        pri = Npriq - 1;
    else if(pri < 0)
        pri = 0;
    /*e: [[procpriority()]] sanity check pri */
    p->priority = pri;

    p->basepri = pri;
    p->fixedpri = fixed;
}
/*e: function [[procpriority]] */

/*s: function [[procinit]] */
void
procinit(void)
{
    Proc *p;
    int i;

    procalloc.free = xalloc(conf.nproc * sizeof(Proc));
    /*s: [[procinit()]] sanity check [[procalloc.free]] */
    if(procalloc.free == nil){
        xsummary();
        panic("cannot allocate %lud procs (%ludMB)\n", 
                      conf.nproc, conf.nproc*sizeof(Proc)/MB);
    }
    /*e: [[procinit()]] sanity check [[procalloc.free]] */
    procalloc.arena = procalloc.free;

    p = procalloc.free;
    for(i=0; i<conf.nproc-1; i++,p++)
        p->qnext = p+1;
    p->qnext = nil;
}
/*e: function [[procinit]] */

/*s: function [[sleep]] */
/*
 *  sleep if a condition is not true.  Another process will
 *  awaken us after it sets the condition.  When we awaken
 *  the condition may no longer be true.
 *
 *  we lock both the process and the rendezvous to keep r->p
 *  and p->r synchronized.
 */
void
proc_sleep(Rendez *r, bool (*f)(void*), void *arg)
{
    int s; // spl
    /*s: [[sleep()]] other locals */
    void (*pt)(Proc*, int, vlong);
    /*e: [[sleep()]] other locals */

    s = arch_splhi();
    /*s: [[sleep()]] sanity check nlocks */
    if(up->nlocks.ref)
        print("process %lud sleeps with %lud locks held, last lock %#p locked at pc %#lux, sleep called from %#p\n",
            up->pid, up->nlocks.ref, up->lastlock, up->lastlock->pc, getcallerpc(&r));
    /*e: [[sleep()]] sanity check nlocks */
    lock(r);
    lock(&up->rlock);
    /*s: [[sleep()]] sanity check double sleep on this rendez vous */
    if(r->p){
        print("double sleep called from %#p, %lud %lud\n", getcallerpc(&r), r->p->pid, up->pid);
        arch_dumpstack();
    }
    /*e: [[sleep()]] sanity check double sleep on this rendez vous */

    /*
     *  Wakeup only knows there may be something to do by testing
     *  r->p in order to get something to lock on.
     *  Flush that information out to memory in case the sleep is
     *  committed.
     */
    r->p = up;

    if((*f)(arg) || up->notepending){
        /*
         *  if condition happened or a note is pending
         *  never mind
         */
        r->p = nil;
        unlock(&up->rlock);
        unlock(r);
    } else {
        /*
         *  now we are committed to
         *  change state and call scheduler
         */
        /*s: [[sleep()]] hook proctrace */
        pt = proctrace;
        if(pt)
            pt(up, SSleep, 0);
        /*e: [[sleep()]] hook proctrace */
        up->state = Wakeme;
        up->r = r;

        // similar code to sched(), why not call sched()? because of the unlock?

        /* statistics */
        cpu->cs++;

        arch_procsave(up);
        if(arch_setlabel(&up->sched)) {
            /*
             *  here when the process is awakened
             */
            arch_procrestore(up);
            arch_spllo();
        } else {
            /*
             *  here to go to sleep (i.e. stop Running)
             */
            unlock(&up->rlock);
            unlock(r);

            arch_gotolabel(&cpu->sched);
            panic("sleep: should never reach this point");
        }
    }
    /*s: [[sleep()]] if notepending */
    if(up->notepending) {
        up->notepending = false;
        arch_splx(s);
        /*s: [[sleep()]] forceclosefgrp */
        if(up->procctl == Proc_exitme && up->closingfgrp)
            forceclosefgrp();
        /*e: [[sleep()]] forceclosefgrp */
        error(Eintr);
    }
    /*e: [[sleep()]] if notepending */
    arch_splx(s);
}
/*e: function [[sleep]] */

/*s: function [[tfn]] */
static int
tfn(void *arg)
{
    return up->trend == nil || up->tfn(arg);
}
/*e: function [[tfn]] */

/*s: function [[twakeup]] */
void
twakeup(Ureg*, Timer *t)
{
    Proc *p;
    Rendez *trend;

    p = t->ta;
    trend = p->trend;
    p->trend = nil;
    if(trend)
        wakeup(trend);
}
/*e: function [[twakeup]] */

/*s: function [[tsleep]] */
void
proc_tsleep(Rendez *r, int (*fn)(void*), void *arg, ulong ms)
{
    /*s: [[tsleep()]] sanity check up timer */
    if (up->tt){
        print("tsleep: timer active: mode %d, tf %#p\n", up->tmode, up->tf);
        timerdel(up);
    }
    /*e: [[tsleep()]] sanity check up timer */
    up->tns = MS2NS(ms);
    up->tmode = Trelative;
    up->tf = twakeup;

    up->ta = up; // so can access r from wakeup
    up->trend = r;

    up->tfn = fn; // needed?

    timeradd(up);

    if(waserror()){
        timerdel(up);
        nexterror();
    }
    sleep(r, tfn, arg);
    // back after sleep
    /*s: [[tsleep()]] reset timer after sleep */
    if(up->tt)
        timerdel(up);
    up->twhen = 0;
    /*e: [[tsleep()]] reset timer after sleep */
    poperror();
}
/*e: function [[tsleep]] */

/*s: function [[wakeup]] */
/*
 *  Expects that only one process can call wakeup for any given Rendez.
 *  We hold both locks to ensure that r->p and p->r remain consistent.
 *  Richard Miller has a better solution that doesn't require both to
 *  be held simultaneously, but I'm a paranoid - presotto.
 */
Proc*
proc_wakeup(Rendez *r)
{
    Proc *p;
    int s; // spl

    s = arch_splhi();
    lock(r);
    p = r->p;

    if(p != nil){
        lock(&p->rlock);
        /*s: [[wakeup()]] sanity check process state */
        if(p->state != Wakeme || p->r != r){
            iprint("%p %p %d\n", p->r, r, p->state);
            panic("wakeup: state");
        }
        /*e: [[wakeup()]] sanity check process state */
        r->p = nil;
        p->r = nil;
        ready(p);
        unlock(&p->rlock);
    }
    unlock(r);
    arch_splx(s);
    return p;
}
/*e: function [[wakeup]] */

/*s: function [[postnote]] */
/*
 *  if waking a sleeping process, this routine must hold both
 *  p->rlock and r->lock.  However, it can't know them in
 *  the same order as wakeup causing a possible lock ordering
 *  deadlock.  We break the deadlock by giving up the p->rlock
 *  lock if we can't get the r->lock and retrying.
 */
int
proc_postnote(Proc *p, int dolock, char *n, int flag)
{
    int s, ret;
    Rendez *r;
    Proc *d, **l;

    if(dolock)
        qlock(&p->debug);

    if(flag != NUser && (p->notify == 0 || p->notified))
        p->nnote = 0;

    ret = 0;
    if(p->nnote < NNOTE) {
        strcpy(p->note[p->nnote].msg, n);
        p->note[p->nnote++].flag = flag;
        ret = 1;
    }
    p->notepending = true;
    if(dolock)
        qunlock(&p->debug);

    /* this loop is to avoid lock ordering problems. */
    for(;;){
        s = arch_splhi();
        lock(&p->rlock);
        r = p->r;

        /* waiting for a wakeup? */
        if(r == nil)
            break;  /* no */

        /* try for the second lock */
        if(canlock(r)){
            if(p->state != Wakeme || r->p != p)
                panic("postnote: state %d %d %d", r->p != p, p->r != r, p->state);
            p->r = nil;
            r->p = nil;
            ready(p);
            unlock(r);
            break;
        }

        /* give other process time to get out of critical section and try again */
        unlock(&p->rlock);
        arch_splx(s);
        sched();
    }
    unlock(&p->rlock);
    arch_splx(s);

    if(p->state != Rendezvous)
        return ret;

    /* Try and pull out of a rendezvous */
    lock(p->rgrp);
    if(p->state == Rendezvous) {
        p->rendval = ~0;
        l = &REND(p->rgrp, p->rendtag);
        for(d = *l; d; d = d->rendhash) {
            if(d == p) {
                *l = p->rendhash;
                break;
            }
            l = &d->rendhash;
        }
        ready(p);
    }
    unlock(p->rgrp);
    return ret;
}
/*e: function [[postnote]] */

/*s: struct [[Broken]] */
/*
 * weird thing: keep at most NBROKEN around
 */
#define NBROKEN 4
struct Broken
{
    // array<ref<Proc>>
    Proc    *p[NBROKEN];
    // number of entries used in p
    int n;

    // extra
    QLock;
};
/*e: struct [[Broken]] */
/*s: global [[broken]] */
struct Broken broken;
/*e: global [[broken]] */

/*s: function [[addbroken]] */
void
addbroken(Proc *p)
{
    qlock(&broken);
    if(broken.n == NBROKEN) {
        ready(broken.p[0]);
        memmove(&broken.p[0], &broken.p[1], sizeof(Proc*)*(NBROKEN-1));
        --broken.n;
    }
    broken.p[broken.n++] = p;
    qunlock(&broken);

    edfstop(up);
    p->state = Broken;
    p->psstate = nil;
    sched();
}
/*e: function [[addbroken]] */

/*s: function [[unbreak]] */
void
unbreak(Proc *p)
{
    int b;

    qlock(&broken);
    for(b=0; b < broken.n; b++)
        if(broken.p[b] == p) {
            broken.n--;
            memmove(&broken.p[b], &broken.p[b+1],
                    sizeof(Proc*)*(NBROKEN-(b+1)));
            ready(p);
            break;
        }
    qunlock(&broken);
}
/*e: function [[unbreak]] */

/*s: function [[freebroken]] */
int
freebroken(void)
{
    int i, n;

    qlock(&broken);
    n = broken.n;
    for(i=0; i<n; i++) {
        ready(broken.p[i]);
        broken.p[i] = nil;
    }
    broken.n = 0;
    qunlock(&broken);
    return n;
}
/*e: function [[freebroken]] */

/*s: function [[pexit]] */
void
proc_pexit(char *exitstr, bool freemem)
{
    Proc *p; // parent
    /*s: [[pexit()]] other locals */
    Waitq *wq;
    /*x: [[pexit()]] other locals */
    Waitq *f, *next;
    /*x: [[pexit()]] other locals */
    Segment **s, **es;
    /*x: [[pexit()]] other locals */
    Fgrp *fgrp;
    Egrp *egrp;
    Rgrp *rgrp;
    Pgrp *pgrp;
    Chan *dot;
    /*x: [[pexit()]] other locals */
    void (*pt)(Proc*, int, vlong);
    /*x: [[pexit()]] other locals */
    long utime, stime;
    /*e: [[pexit()]] other locals */

    // free 1
    /*s: [[pexit()]] at the start, free resources */
    if (up->tt)
        timerdel(up);
    /*x: [[pexit()]] at the start, free resources */
    up->alarm = 0;
    /*x: [[pexit()]] at the start, free resources */
    if(up->syscalltrace)
        free(up->syscalltrace);
    /*e: [[pexit()]] at the start, free resources */
    /*s: [[pexit()]] hook proctrace */
    pt = proctrace;
    if(pt)
        pt(up, SDead, 0);
    /*e: [[pexit()]] hook proctrace */
    // free 2
    /*s: [[pexit()]] nil out resources under lock and free them */
    /* nil out all the resources under lock (free later) */
    qlock(&up->debug);
    fgrp = up->fgrp;
    up->fgrp = nil;
    egrp = up->egrp;
    up->egrp = nil;
    rgrp = up->rgrp;
    up->rgrp = nil;
    pgrp = up->pgrp;
    up->pgrp = nil;
    dot = up->dot;
    up->dot = nil;
    qunlock(&up->debug);

    if(fgrp)
        closefgrp(fgrp);
    if(egrp)
        closeegrp(egrp);
    if(rgrp)
        closergrp(rgrp);
    if(dot)
        cclose(dot);
    if(pgrp)
        closepgrp(pgrp);
    /*e: [[pexit()]] nil out resources under lock and free them */

    /*s: [[pexit()]] if a kernel process, do nothing about parent and child */
    if(up->kp) {
    }
    /*e: [[pexit()]] if a kernel process, do nothing about parent and child */
    else {
        /*
         * if not a kernel process and have a parent,
         * do some housekeeping.
         */
        p = up->parent;
        /*s: [[pexit()]] sanity check parent pointer [[p]] */
        // no parent pointer, must be the very first process
        if(p == nil) {
            if(exitstr == nil)
                exitstr = "unknown";
            panic("boot process died: %s", exitstr);
        }
        /*e: [[pexit()]] sanity check parent pointer [[p]] */
        while(waserror())
            ; // ????

        // exit/await and your parent
        /*s: [[pexit()]] build waitq message [[wq]] */
        wq = smalloc(sizeof(Waitq));
        poperror(); // of ????

        wq->w.pid = up->pid;
        /*s: [[pexit()]] set wait msg time field */
        utime = up->time[TUser] + up->time[TCUser];
        stime = up->time[TSys] + up->time[TCSys];

        wq->w.time[TUser] = tk2ms(utime);
        wq->w.time[TSys] = tk2ms(stime);
        wq->w.time[TReal] = tk2ms(CPUS(0)->ticks - up->time[TReal]);
        /*e: [[pexit()]] set wait msg time field */
        if(exitstr && exitstr[0])
            snprint(wq->w.msg, sizeof(wq->w.msg), "%s %lud: %s", up->text, up->pid, exitstr);
        else
            wq->w.msg[0] = '\0';
        /*e: [[pexit()]] build waitq message [[wq]] */
        /*s: [[pexit()]] add [[wq]] message in parent process [[p]] waitq */
        lock(&p->exl);
        /*
         * Check that parent is still alive.
         */
        // the parent pointer may not be your parent anymore, the parent
        // could have died and its slot reallocated to another process
        if(p->pid == up->parentpid && p->state != Broken) {
            p->nchild--;
            /*s: [[pexit()]] update TC time of parent */
            p->time[TCUser] += utime;
            p->time[TCSys] += stime;
            /*e: [[pexit()]] update TC time of parent */
            /*
             * If there would be more than 128 wait records
             * processes for my parent, then don't leave a wait
             * record behind.  This helps prevent badly written
             * daemon processes from accumulating lots of wait
             * records.
             */
            if(p->nwait < 128) {
                // push(wq, p->waitq)
                wq->next = p->waitq;
                p->waitq = wq;
                p->nwait++;

                wq = nil; // the parent will do the free
                wakeup(&p->waitr); // haswaitq() is true now
            }
        }
        unlock(&p->exl);
        if(wq)
            free(wq);
        /*e: [[pexit()]] add [[wq]] message in parent process [[p]] waitq */
    }
    /*s: [[pexit()]] if not freemem, add process to broken processes list */
    // instead of a core dump we just keep the process around
    if(!freemem)
        addbroken(up); // will call sched()
    /*e: [[pexit()]] if not freemem, add process to broken processes list */

    // free 3, must be after built waitq because exitstr in one of the segment
    /*s: [[pexit()]] free memory segments */
    qlock(&up->seglock);
    es = &up->seg[NSEG];
    for(s = up->seg; s < es; s++) {
        if(*s) {
            putseg(*s);
            *s = nil;
        }
    }
    qunlock(&up->seglock);
    /*e: [[pexit()]] free memory segments */

    // exit/await and your (orphan) children
    /*s: [[pexit()]] wakeup process waiting on waitr and unhash pid */
    lock(&up->exl);     /* Prevent my children from leaving waits */
    pidunhash(up);
    // so my children will not generate a waitq, I will not be here anymore
    up->pid = 0; 
    wakeup(&up->waitr);
    unlock(&up->exl);
    /*e: [[pexit()]] wakeup process waiting on waitr and unhash pid */

    // free 4
    /*s: [[pexit()]] free waitq */
    for(f = up->waitq; f; f = next) {
        next = f->next;
        free(f);
    }
    /*e: [[pexit()]] free waitq */
    // free 5
    /*s: [[pexit()]] release debuggers */
    /* release debuggers */
    qlock(&up->debug);
    if(up->pdbg) {
        wakeup(&up->pdbg->sleepr);
        up->pdbg = nil;
    }
    qunlock(&up->debug);
    /*e: [[pexit()]] release debuggers */

    /* Sched must not loop for these locks */
    lock(&procalloc);
    lock(&palloc);

    /*s: [[pexit()]] optional [[edfstop()]] for real-time scheduling */
    edfstop(up);
    /*e: [[pexit()]] optional [[edfstop()]] for real-time scheduling */
    up->state = Moribund;
    // will arch_gotolabel() to schedinit() which has special code around Moribund
    sched(); 
    panic("pexit: should never reach this point"); 
}
/*e: function [[pexit]] */

/*s: function [[haswaitq]] */
int
haswaitq(void *x)
{
    Proc *p;
    p = (Proc *)x;
    return p->waitq != nil;
}
/*e: function [[haswaitq]] */

/*s: function [[pwait]] */
ulong
pwait(Waitmsg *w)
{
    ulong cpid; // child pid
    Waitq *wq;

    /*s: [[pwait()]] lock qwaitr */
    if(!canqlock(&up->qwaitr))
        error(Einuse); // someone is reading /proc/pid/wait?
    if(waserror()) {
        qunlock(&up->qwaitr);
        nexterror();
    }
    /*e: [[pwait()]] lock qwaitr */

    /*s: [[pwait()]] sanity check process has children */
    lock(&up->exl);
    if(up->nchild == 0 && up->waitq == nil) {
        unlock(&up->exl);
        error(Enochild);
    }
    unlock(&up->exl);
    /*e: [[pwait()]] sanity check process has children */

    // sleep!
    /*s: [[pwait()]] wait until has a waitq */
    sleep(&up->waitr, haswaitq, up); // qwaitr is still locked
    /*e: [[pwait()]] wait until has a waitq */
    // ok, got a waitmsg

    lock(&up->exl);
    // wq = pop(up->waitq), // can't be null, see haswaitq()
    wq = up->waitq; 
    up->waitq = wq->next;
    up->nwait--;
    unlock(&up->exl);

    /*s: [[pwait()]] unlock qwaitr */
    qunlock(&up->qwaitr);
    poperror();
    /*e: [[pwait()]] unlock qwaitr */

    if(w)
        memmove(w, &wq->w, sizeof(Waitmsg));
    cpid = wq->w.pid;
    free(wq);
    return cpid;
}
/*e: function [[pwait]] */

/*s: function [[proctab]] */
Proc*
proc_proctab(int i)
{
    return &procalloc.arena[i];
}
/*e: function [[proctab]] */

/*s: function [[dumpaproc]] */
void
proc_dumpaproc(Proc *p)
{
    ulong bss;
    char *s;

    if(p == nil)
        return;

    bss = 0;
    if(p->seg[BSEG])
        bss = p->seg[BSEG]->top;

    s = p->psstate;
    if(s == nil)
        s = statename[p->state];
    print("%3lud:%10s pc %8lux dbgpc %8lux  %8s (%s) ut %ld st %ld bss %lux qpc %lux nl %lud nd %lud lpc %lux pri %lud\n",
        p->pid, p->text, p->pc, arch_dbgpc(p),  s, statename[p->state],
        p->time[TUser], p->time[TSys], bss, p->qpc, p->nlocks.ref, p->delaysched, p->lastlock ? p->lastlock->pc : 0, p->priority);
}
/*e: function [[dumpaproc]] */

/*s: function [[procdump]] */
void
procdump(void)
{
    int i;
    Proc *p;

    if(up)
        print("up %lud\n", up->pid);
    else
        print("no current process\n");
    for(i=0; i<conf.nproc; i++) {
        p = &procalloc.arena[i];
        if(p->state == Dead)
            continue;

        dumpaproc(p);
    }
}
/*e: function [[procdump]] */

/*s: function [[procflushseg]] */
/*
 *  wait till all processes have flushed their mmu
 *  state about segment s
 */
void
procflushseg(Segment *s)
{
    int i, ns, nm;
    bool wait;
    Proc *p;

    /*
     *  tell all processes with this
     *  segment to flush their mmu's
     */
    wait = false;
    for(i=0; i<conf.nproc; i++) {
        p = &procalloc.arena[i];
        if(p->state == Dead)
            continue;
        for(ns = 0; ns < NSEG; ns++)
            if(p->seg[ns] == s){
                p->newtlb = true;
                for(nm = 0; nm < conf.ncpu; nm++){
                    if(CPUS(nm)->proc == p){
                        CPUS(nm)->flushmmu = true;
                        wait = true;
                    }
                }
                break;
            }
    }
    if(!wait)
        return;

    /*
     *  wait for all processors to take a clock interrupt
     *  and flush their mmu's
     */
    for(nm = 0; nm < conf.ncpu; nm++)
        if(CPUS(nm) != cpu)
            while(CPUS(nm)->flushmmu)
                sched();
}
/*e: function [[procflushseg]] */

/*s: function [[scheddump]] */
void
scheddump(void)
{
    Proc *p;
    Schedq *rq;

    for(rq = &runq[Nrq-1]; rq >= runq; rq--){
        if(rq->head == 0)
            continue;
        print("rq%ld:", rq-runq);
        for(p = rq->head; p; p = p->rnext)
            print(" %lud(%lud)", p->pid, cpu->ticks - p->readytime);
        print("\n");
        arch_delay(150);
    }
    print("nrdy %d\n", nrdy);
}
/*e: function [[scheddump]] */

/*s: function [[kproc]] */
// kernel process (aka kernel_thread in Linux?)
void
kproc(char *name, void (*func)(void *), void *arg)
{
    Proc *p;
    static Pgrp *kpgrp;

    p = newproc();
    p->kp = true; // Kernel Process!

    p->psstate = nil;
    p->procmode = 0640;
    p->ureg = nil;
    p->noswap = true;

    p->slash = up->slash;
    p->dot = up->dot;
    if(p->dot)
        incref(p->dot);

    p->fpsave = up->fpsave;
    p->sargs = up->sargs;

    memmove(p->note, up->note, sizeof(p->note));
    p->nnote = up->nnote;
    p->lastnote = up->lastnote;
    p->notify = up->notify;

    procpriority(p, PriKproc, false);

    arch_kprocchild(p, func, arg);

    kstrdup(&p->user, eve);
    kstrdup(&p->text, name);

    if(kpgrp == nil)
        kpgrp = newpgrp();
    p->pgrp = kpgrp;
    incref(kpgrp);

    /*s: [[kproc()]] setting time field */
    memset(p->time, 0, sizeof(p->time));
    p->time[TReal] = CPUS(0)->ticks;
    /*e: [[kproc()]] setting time field */
    ready(p);
}
/*e: function [[kproc]] */

/*s: function [[procctl]] */
/*
 *  called arch_splhi() by notify().  See comment in notify for the
 *  reasoning.
 */
void
procctl(Proc *p)
{
    char *oldstate;
    ulong s;

    switch(p->procctl) {
    /*s: [[procctl()]] [[Proc_traceme]] case (and fallthrough [[Proc_stopme]]) */
    case Proc_traceme:
        if(p->nnote == 0)
            return;
        /* No break */
    /*e: [[procctl()]] [[Proc_traceme]] case (and fallthrough [[Proc_stopme]]) */
    case Proc_stopme:
        p->procctl = Proc_nothing;
        oldstate = p->psstate;
        p->psstate = "Stopped";
        s = arch_spllo();
        /*s: [[procctl()]] wakeup waiting debugger */
        /* free a waiting debugger */
        qlock(&p->debug);
        if(p->pdbg) {
            wakeup(&p->pdbg->sleepr);
            p->pdbg = nil;
        }
        qunlock(&p->debug);
        /*e: [[procctl()]] wakeup waiting debugger */
        arch_splhi();
        p->state = Stopped;
        sched();
        // here when something (debugger, strace, user) ready(p) back
        p->psstate = oldstate;
        arch_splx(s);
        return;
    /*s: [[procctl()]] [[Proc_exitbig]] case */
    case Proc_exitbig:
        arch_spllo();
        pexit("Killed: Insufficient physical memory", true);
        // Fallthrough
    /*e: [[procctl()]] [[Proc_exitbig]] case */
    /*s: [[procctl()]] [[Proc_exitme]] case */
    case Proc_exitme:
        arch_spllo();        /* pexit has locks in it */
        pexit("Killed", true);
    /*e: [[procctl()]] [[Proc_exitme]] case */
    }
}
/*e: function [[procctl]] */

/*s: function [[killbig]] */
void
killbig(char *why)
{
    int i;
    Segment *s;
    ulong l, max;
    Proc *p, *ep, *kp;

    max = 0;
    kp = 0;
    ep = procalloc.arena+conf.nproc;
    for(p = procalloc.arena; p < ep; p++) {
        if(p->state == Dead || p->kp)
            continue;
        l = 0;
        for(i=1; i<NSEG; i++) {
            s = p->seg[i];
            if(s != 0)
                l += s->top - s->base;
        }
        if(l > max && ((p->procmode&0222) || strcmp(eve, p->user)!=0)) {
            kp = p;
            max = l;
        }
    }

    print("%lud: %s killed: %s\n", kp->pid, kp->text, why);
    for(p = procalloc.arena; p < ep; p++) {
        if(p->state == Dead || p->kp)
            continue;
        if(p != kp && p->seg[BSEG] && p->seg[BSEG] == kp->seg[BSEG])
            p->procctl = Proc_exitbig;
    }
    kp->procctl = Proc_exitbig;
    for(i = 0; i < NSEG; i++) {
        s = kp->seg[i];
        if(s != 0 && canqlock(&s->lk)) {
            mfreeseg(s, s->base, (s->top - s->base)/BY2PG);
            qunlock(&s->lk);
        }
    }
}
/*e: function [[killbig]] */

/*s: function [[renameuser]] */
/*
 *  change ownership to 'new' of all processes owned by 'old'.  Used when
 *  eve changes.
 */
void
renameuser(char *old, char *new)
{
    Proc *p, *ep;

    ep = procalloc.arena + conf.nproc;
    for(p = procalloc.arena; p < ep; p++)
        if(p->user!=nil && strcmp(old, p->user)==0)
            kstrdup(&p->user, new);
}
/*e: function [[renameuser]] */

/*s: function [[accounttime]] */
/*
 *  time accounting called by clock() splhi'd
 */
void
accounttime(void)
{
    Proc *p;
    ulong n, per;
    static ulong nrun;

    p = cpu->proc; // why not p = up?
    if(p) {
        nrun++;
        /*s: [[accountime()]] update time of current process */
        p->time[p->insyscall ? TSys : TUser]++;
        /*e: [[accountime()]] update time of current process */
    }

    /* calculate decaying duty cycles */
    n = arch_perfticks();
    per = n - cpu->perf.last;
    cpu->perf.last = n;
    per = (cpu->perf.period*(Arch_HZ-1) + per)/Arch_HZ;
    if(per != 0)
        cpu->perf.period = per;

    cpu->perf.avg_inidle = 
      (cpu->perf.avg_inidle*(Arch_HZ-1)+cpu->perf.inidle)/Arch_HZ;
    cpu->perf.inidle = 0;

    cpu->perf.avg_inintr = 
      (cpu->perf.avg_inintr*(Arch_HZ-1)+cpu->perf.inintr)/Arch_HZ;
    cpu->perf.inintr = 0;

    /* only one processor gets to compute system load averages */
    if(cpu->cpuno != 0)
        return;

    /*
     * calculate decaying load average.
     * if we decay by (n-1)/n then it takes
     * n clock ticks to go from load L to .36 L once
     * things quiet down.  it takes about 5 n clock
     * ticks to go to zero.  so using HZ means this is
     * approximately the load over the last second,
     * with a tail lasting about 5 seconds.
     */
    n = nrun;
    nrun = 0;
    n = (nrdy+n)*1000;
    cpu->load = (cpu->load*(Arch_HZ-1)+n)/Arch_HZ;
}
/*e: function [[accounttime]] */

/*s: function [[pidhash]] */
static void
pidhash(Proc *p)
{
    int h;

    h = p->pid % nelem(procalloc.ht);
    lock(&procalloc);
    // add_hash(procalloc.ht, p->pid, p)
    p->pidhash = procalloc.ht[h];
    procalloc.ht[h] = p;
    unlock(&procalloc);
}
/*e: function [[pidhash]] */

/*s: function [[pidunhash]] */
static void
pidunhash(Proc *p)
{
    int h;
    Proc **l;

    h = p->pid % nelem(procalloc.ht);
    lock(&procalloc);
    // remove_hash(procalloc.ht, p->pid, p)
    for(l = &procalloc.ht[h]; *l != nil; l = &(*l)->pidhash)
        if(*l == p){
            *l = p->pidhash;
            break;
        }
    unlock(&procalloc);
}
/*e: function [[pidunhash]] */

/*s: function [[procindex]] */
int
procindex(ulong pid)
{
    Proc *p;
    int h;
    int s;

    s = -1;
    // hash_lookup(pid, procalloc.ht)
    h = pid % nelem(procalloc.ht);
    lock(&procalloc);
    for(p = procalloc.ht[h]; p != nil; p = p->pidhash)
        if(p->pid == pid){
            s = p - procalloc.arena;
            break;
        }
    unlock(&procalloc);
    return s;
}
/*e: function [[procindex]] */
/*e: proc.c */
