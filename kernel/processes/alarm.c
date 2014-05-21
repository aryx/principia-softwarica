/*s: alarm.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: global alarms */
static Alarms   alarms;
/*e: global alarms */
/*s: global alarmr */
static Rendez   alarmr;
/*e: global alarmr */

/*s: kernel process alarmkproc */
// Kernel Process for alarm managment
void
alarmkproc(void*)
{
    Proc *rp;
    ulong now;

    for(;;){
        now = CPUS(0)->ticks;
        qlock(&alarms);
        /*
         * the odd test of now vs. rp->alarm is to cope with
         * now wrapping around.
         */
        while((rp = alarms.head) && (long)(now - rp->alarm) >= 0){
            if(rp->alarm != 0L){
                if(canqlock(&rp->debug)){
                    if(!waserror()){
                        postnote(rp, 0, "alarm", NUser);
                        poperror();
                    }
                    qunlock(&rp->debug);
                    rp->alarm = 0L;
                }else
                    break;
            }
            alarms.head = rp->palarm;
        }
        qunlock(&alarms);

        sleep(&alarmr, return0, 0);
    }
}
/*e: kernel process alarmkproc */

/*s: function checkalarms */
/*
 *  called every clock tick
 */
void
checkalarms(void)
{
    Proc *p;
    ulong now;

    p = alarms.head;
    now = CPUS(0)->ticks;

    if(p && (long)(now - p->alarm) >= 0)
        wakeup(&alarmr);
}
/*e: function checkalarms */

/*s: function procalarm */
ulong
procalarm(ulong time)
{
    Proc **l, *f;
    ulong when, old;

    if(up->alarm)
        old = tk2ms(up->alarm - CPUS(0)->ticks);
    else
        old = 0;
    if(time == 0) {
        up->alarm = 0;
        return old;
    }
    when = ms2tk(time)+CPUS(0)->ticks;
    if(when == 0)       /* ticks have wrapped to 0? */
        when = 1;   /* distinguish a wrapped alarm from no alarm */

    qlock(&alarms);
    l = &alarms.head;
    for(f = *l; f; f = f->palarm) {
        if(up == f){
            *l = f->palarm;
            break;
        }
        l = &f->palarm;
    }

    up->palarm = 0;
    if(alarms.head) {
        l = &alarms.head;
        for(f = *l; f; f = f->palarm) {
            if((long)(f->alarm - when) >= 0) {
                up->palarm = f;
                *l = up;
                goto done;
            }
            l = &f->palarm;
        }
        *l = up;
    }
    else
        alarms.head = up;
done:
    up->alarm = when;
    qunlock(&alarms);

    return old;
}
/*e: function procalarm */
/*e: alarm.c */
