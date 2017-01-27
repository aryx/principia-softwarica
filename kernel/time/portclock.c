/*s: portclock.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"

#include <ureg.h>

enum {
   /*s: constant Maxtimerloops */
       Maxtimerloops = 20*1000,
   /*e: constant Maxtimerloops */
};

/*s: global timers */
static Timers timers[MAXCPUS];
/*e: global timers */
/*s: global timersinited */
static bool timersinited;
/*e: global timersinited */

/*s: clock.c statistics */
ulong intrcount[MAXCPUS];
ulong fcallcount[MAXCPUS];
/*e: clock.c statistics */

/*s: function tadd */
static Tval
tadd(Timers *tt, Timer *nt)
{
    Timer *t, **last;

    /* Called with tt locked */
    assert(nt->tt == nil);
    switch(nt->tmode){
    case Trelative:
        if(nt->tns <= 0)
            nt->tns = 1;
        nt->twhen = arch_fastticks(nil) + ns2fastticks(nt->tns);
        break;
    case Tperiodic:
        assert(nt->tns >= 100000);  /* At least 100 Âµs period */
        if(nt->twhen == 0){
            /* look for another timer at same frequency for combining */
            for(t = tt->head; t; t = t->tnext){
                if(t->tmode == Tperiodic && t->tns == nt->tns)
                    break;
            }
            if (t)
                nt->twhen = t->twhen;
            else
                nt->twhen = arch_fastticks(nil);
        }
        nt->twhen += ns2fastticks(nt->tns);
        break;
    default:
        panic("timer: impossible");
        break;
    }

    for(last = &tt->head; t = *last; last = &t->tnext){
        if(t->twhen > nt->twhen)
            break;
    }
    nt->tnext = *last;
    *last = nt;
    nt->tt = tt;
    if(last == &tt->head)
        return nt->twhen;
    return 0;
}
/*e: function tadd */

/*s: function tdel */
static Tval
tdel(Timer *dt)
{
    Timer *t, **last;
    Timers *tt;

    tt = dt->tt;
    if (tt == nil)
        return 0; // possible? panic("impossible") would be better no?
    for(last = &tt->head; t = *last; last = &t->tnext){
        if(t == dt){
            assert(dt->tt);
            dt->tt = nil;
            *last = t->tnext;
            break;
        }
    }
    if(last == &tt->head && tt->head)
        return tt->head->twhen;
    return 0;
}
/*e: function tdel */

/*s: function timeradd */
/* add or modify a timer */
void
timeradd(Timer *nt)
{
    Timers *tt;
    Tval when;

    /* Must lock Timer struct before Timers struct */
    ilock(nt);

    if(tt = nt->tt){
        ilock(tt);
        tdel(nt);
        iunlock(tt);
    }

    tt = &timers[cpu->cpuno];
    ilock(tt);
    when = tadd(tt, nt);
    if(when)
        arch_timerset(when);
    iunlock(tt);
    iunlock(nt);
}
/*e: function timeradd */

/*s: function timerdel */
void
timerdel(Timer *dt)
{
    Timers *tt;
    Tval when;

    ilock(dt);
    if(tt = dt->tt){
        ilock(tt);
        when = tdel(dt);
        if(when && tt == &timers[cpu->cpuno])
            arch_timerset(tt->head->twhen);
        iunlock(tt);
    }
    iunlock(dt);
}
/*e: function timerdel */

/*s: clock callback hzclock */
void
hzclock(Ureg *ur)
{
    cpu->ticks++;
    if(cpu->proc) // why not using up here? why cpu->proc?
        cpu->proc->pc = ur->pc;

    if(cpu->flushmmu){
        if(up)
            arch_flushmmu();
        cpu->flushmmu = false;
    }

    accounttime();
    //kmapinval();pc: bcm: a nope in both archs

    if(kproftimer != nil)
        kproftimer(ur->pc);

    if((active.cpus&(1<<cpu->cpuno)) == 0)
        return;

    if(active.exiting) {
        print("someone's exiting\n");
        arch_exit(0);
    }

    checkalarms();

    if(up && up->state == Running)
        hzsched();  /* in proc.c */
}
/*e: clock callback hzclock */

/*s: interrupt callback timerintr */
// called via i8253clock
void
timerintr(Ureg *u, Tval)
{
    Timer *t;
    Timers *tt;
    uvlong when, now;
    int count, callhzclock;

    intrcount[cpu->cpuno]++;
    callhzclock = 0;
    tt = &timers[cpu->cpuno];
    now = arch_fastticks(nil);
    if(now == 0)
        panic("timerintr: zero arch_fastticks()");
    ilock(tt);
    count = Maxtimerloops;
    while((t = tt->head) != nil){
        /*
         * No need to ilock t here: any manipulation of t
         * requires tdel(t) and this must be done with a
         * lock to tt held.  We have tt, so the tdel will
         * wait until we're done
         */
        when = t->twhen;
        if(when > now){
            arch_timerset(when);
            iunlock(tt);
            if(callhzclock)
                hzclock(u);
            return;
        }
        tt->head = t->tnext;
        assert(t->tt == tt);
        t->tt = nil;
        fcallcount[cpu->cpuno]++;
        iunlock(tt);
        if(t->tf)
            (*t->tf)(u, t);
        else
            callhzclock++;
        ilock(tt);
        if(t->tmode == Tperiodic)
            tadd(tt, t);
        if (--count <= 0) {
            count = Maxtimerloops;
            iprint("timerintr: probably stuck in while loop; "
                "scrutinise clock.c or use faster cycle "
                "counter\n");
        }
    }
    iunlock(tt);
}
/*e: interrupt callback timerintr */

/*s: function timersinit */
void
timersinit(void)
{
    Timer *t;

    timersinited = true;
    todinit();

    t = malloc(sizeof(Timer));
    if(t == nil)
        error(Enomem);
    t->tmode = Tperiodic;
    t->tt = nil;
    t->tns = 1000000000/HZ;
    /*
     * T->tf == nil means the HZ clock for this processor.
     */
    t->tf = nil;
    timeradd(t);
}
/*e: function timersinit */

/*s: function addclock0link */
Timer*
addclock0link(void (*f)(void), Tms ms)
{
    Timer *nt;
    Tval when;

    if(!timersinited)
        panic("addclock0link: timersinit not called yet");

    /* Synchronize to hztimer if ms is 0 */
    nt = malloc(sizeof(Timer));
    if(nt == nil)
        error(Enomem);
    if(ms == 0)
        ms = 1000/HZ;

    nt->tns = (Tnano)ms*1000000LL;
    nt->tmode = Tperiodic;
    nt->tt = nil;
    nt->tf = (void (*)(Ureg*, Timer*))f;

    // those clock callbacks are all done on the bootstrap processor
    //dupe: timeradd() but with forced processor number
    ilock(&timers[0]);
    when = tadd(&timers[0], nt);
    if(when)
        arch_timerset(when);
    iunlock(&timers[0]);
    return nt;
}
/*e: function addclock0link */

/*s: function tk2ms */
/*
 *  This tk2ms avoids overflows that the macro version is prone to.
 *  It is a LOT slower so shouldn't be used if you're just converting
 *  a delta.
 */
ulong
tk2ms(ulong ticks)
{
    uvlong t, hz;

    t = ticks;
    hz = HZ;
    t *= 1000L;
    t = t/hz;
    ticks = t;
    return ticks;
}
/*e: function tk2ms */

/*s: function ms2tk */
ulong
ms2tk(ulong ms)
{
    /* avoid overflows at the cost of precision */
    if(ms >= 1000000000/HZ)
        return (ms/1000)*HZ;
    return (ms*HZ+500)/1000;
}
/*e: function ms2tk */
/*e: portclock.c */
