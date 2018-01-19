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
   /*s: constant [[Maxtimerloops]] */
   Maxtimerloops = 20*1000,
   /*e: constant [[Maxtimerloops]] */
};

/*s: global [[timers]] */
static Timers timers[MAXCPUS];
/*e: global [[timers]] */
/*s: global [[timersinited]] */
static bool timersinited;
/*e: global [[timersinited]] */

/*s: clock.c statistics */
ulong intrcount[MAXCPUS];
ulong fcallcount[MAXCPUS];
/*e: clock.c statistics */

/*s: function [[tadd]] */
static Tfast
tadd(Timers *tt, Timer *nt)
{
    Timer *t, **last;

    /* Called with tt locked */
    /*s: [[tadd()]] sanity check nt */
    assert(nt->tt == nil);
    /*e: [[tadd()]] sanity check nt */
    /*s: [[tadd()]] set [[nt->twhen]] */
    switch(nt->tmode){
    case Trelative:
        if(nt->tns <= 0)
            nt->tns = 1;
        nt->twhen = arch_fastticks(nil) + ns2fastticks(nt->tns);
        break;
    case Tperiodic:
        assert(nt->tns >= 100000);  /* At least 100 Âµs period */
        if(nt->twhen == 0){
            /*s: [[tadd()]] look if can combine with identical timer */
            /* look for another timer at same frequency for combining */
            for(t = tt->head; t; t = t->tnext){
                if(t->tmode == Tperiodic && t->tns == nt->tns)
                    break;
            }
            if (t)
                nt->twhen = t->twhen;
            /*e: [[tadd()]] look if can combine with identical timer */
            else
                nt->twhen = arch_fastticks(nil);
        }
        nt->twhen += ns2fastticks(nt->tns);
        break;
    default:
        panic("timer: impossible");
        break;
    }
    /*e: [[tadd()]] set [[nt->twhen]] */

    // insert_sorted(nt, tt, (a->twhen <=> b->twhen))
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
/*e: function [[tadd]] */

/*s: function [[tdel]] */
static Tfast
tdel(Timer *dt)
{
    Timer *t, **last;
    Timers *tt;

    tt = dt->tt;
    /*s: [[tdel()]] sanity check tt */
    if (tt == nil)
        return 0; //todo: panic instead
    /*e: [[tdel()]] sanity check tt */
    // remove_list(dt, tt)
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
/*e: function [[tdel]] */

/*s: function [[timeradd]] */
/* add or modify a timer */
void
timeradd(Timer *nt)
{
    Timers *tt;
    Tfast when;

    /* Must lock Timer struct before Timers struct */
    ilock(nt);

    /*s: [[timeradd()]] if timer was already in a list */
    if(tt = nt->tt){
        ilock(tt);
        tdel(nt);
        iunlock(tt);
    }
    /*e: [[timeradd()]] if timer was already in a list */
    tt = &timers[cpu->cpuno];
    ilock(tt);
    when = tadd(tt, nt);
    if(when)
        arch_timerset(when);
    iunlock(tt);
    iunlock(nt);
}
/*e: function [[timeradd]] */

/*s: function [[timerdel]] */
void
timerdel(Timer *dt)
{
    Timers *tt;
    Tfast when;

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
/*e: function [[timerdel]] */

/*s: clock callback hzclock */
void
hzclock(Ureg *ur)
{
    cpu->ticks++;
    /*s: [[hzclock()]] adjust pc of proc */
    if(cpu->proc) // why not using up here? why cpu->proc?
        cpu->proc->pc = ur->pc;
    /*e: [[hzclock()]] adjust pc of proc */
    /*s: [[hzclock()]] if flushmmu */
    if(cpu->flushmmu){
        if(up)
            arch_flushmmu();
        cpu->flushmmu = false;
    }
    /*e: [[hzclock()]] if flushmmu */
    /*s: [[hzclock()]] accounttime */
    accounttime();
    /*e: [[hzclock()]] accounttime */
    /*s: [[hzclock()]] if kproftimer */
    if(kproftimer != nil)
        kproftimer(ur->pc);
    /*e: [[hzclock()]] if kproftimer */

    /*s: [[hzclock()]] if not active cpu, return */
    if((active.cpus & (1<<cpu->cpuno)) == 0)
        return;
    /*e: [[hzclock()]] if not active cpu, return */
    // else
    /*s: [[hzclock()]] if exiting, exit */
    if(active.exiting) {
        print("someone's exiting\n");
        arch_exit(0);
    }
    /*e: [[hzclock()]] if exiting, exit */
    // else

    /*s: [[hzclock()]] check the alarms */
    checkalarms();
    /*e: [[hzclock()]] check the alarms */
    if(up && up->state == Running)
        hzsched();  /* in proc.c */
}
/*e: clock callback hzclock */

/*s: interrupt callback timerintr */
void
timerintr(Ureg *u, Tval)
{
    Timer *t;
    Timers *tt;
    Tufast when, now;
    bool callhzclock = false;
    /*s: [[timerintr()]] other locals */
    int count;
    /*e: [[timerintr()]] other locals */

    /*s: [[timerintr()]] start, stat */
    intrcount[cpu->cpuno]++;
    /*e: [[timerintr()]] start, stat */
    tt = &timers[cpu->cpuno];
    now = arch_fastticks(nil);
    /*s: [[timerintr()]] sanity check now */
    if(now == 0)
        panic("timerintr: zero arch_fastticks()");
    /*e: [[timerintr()]] sanity check now */
    ilock(tt);
    /*s: [[timerintr()]] before loop, initialize count */
    count = Maxtimerloops;
    /*e: [[timerintr()]] before loop, initialize count */
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
        // else
        // pop_list(tt)
        tt->head = t->tnext;
        assert(t->tt == tt);
        t->tt = nil;
        /*s: [[timerintr()]] in timers loop, stat */
        fcallcount[cpu->cpuno]++;
        /*e: [[timerintr()]] in timers loop, stat */
        iunlock(tt);

        // call the callback
        if(t->tf)
            (*t->tf)(u, t);
        else
            callhzclock = true;

        ilock(tt);
        if(t->tmode == Tperiodic)
            tadd(tt, t);
        /*s: [[timerintr()]] end of timers loop, sanity check count */
        if (--count <= 0) {
            count = Maxtimerloops;
            iprint("timerintr: probably stuck in while loop; "
                "scrutinise clock.c or use faster cycle "
                "counter\n");
        }
        /*e: [[timerintr()]] end of timers loop, sanity check count */
    }
    iunlock(tt); // unreachable?
}
/*e: interrupt callback timerintr */

/*s: function [[timersinit]] */
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
    t->tns = 1000000000 / Arch_HZ;
    /*
     * T->tf == nil means the HZ clock for this processor.
     */
    t->tf = nil;
    timeradd(t);
}
/*e: function [[timersinit]] */

/*s: function [[addclock0link]] */
Timer*
addclock0link(void (*f)(void), Tms ms)
{
    Timer *nt;
    Tfast when;

    /*s: [[addclock0link()]] sanity check timersinit */
    if(!timersinited)
        panic("addclock0link: timersinit not called yet");
    /*e: [[addclock0link()]] sanity check timersinit */

    nt = malloc(sizeof(Timer));
    /*s: [[addclock0link()]] sanity check nt */
    if(nt == nil)
        error(Enomem);
    /*e: [[addclock0link()]] sanity check nt */
    /*s: [[addclock0link()]] sanitize ms */
    /* Synchronize to hztimer if ms is 0 */
    if(ms == 0)
        ms = 1000 / Arch_HZ; // tk2ms(1)
    /*e: [[addclock0link()]] sanitize ms */

    nt->tns = (Tnano)ms * 1000000LL; // MS2NS(ms)
    nt->tmode = Tperiodic;
    nt->tt = nil;
    nt->tf = (void (*)(Ureg*, Timer*))f;

    // those clock callbacks are all done on the bootstrap processor
    ilock(&timers[0]);
    when = tadd(&timers[0], nt);
    if(when)
        arch_timerset(when);
    iunlock(&timers[0]);
    return nt;
}
/*e: function [[addclock0link]] */

/*s: function [[tk2ms]] */
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
    hz = Arch_HZ;
    t *= 1000L;
    t = t/hz;
    ticks = t;
    return ticks;
}
/*e: function [[tk2ms]] */

/*s: function [[ms2tk]] */
ulong
ms2tk(ulong ms)
{
    /*s: [[ms2tk()]] overflow approximation */
    /* avoid overflows at the cost of precision */
    if(ms >= 1000000000 / Arch_HZ)
        return (ms / 1000) * Arch_HZ;
    /*e: [[ms2tk()]] overflow approximation */
    return (ms * Arch_HZ + 500) / 1000;
}
/*e: function [[ms2tk]] */
/*e: portclock.c */
