/*s: qlock.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: struct QlockStats */
struct QlockStats {
    ulong rlock;
    ulong rlockq;
    ulong wlock;
    ulong wlockq;
    ulong qlock;
    ulong qlockq;
};
/*e: struct QlockStats */
/*s: global rwstats */
struct QlockStats rwstats;
/*e: global rwstats */

/*s: function qlock */
void
qlock(QLock *q)
{
    Proc *p;

    /*s: [[qlock()]] sanity checks */
    if(q->use.key == 0x55555555) // dead code??
        panic("qlock: q %#p, key 5*\n", q);
    /*x: [[qlock()]] sanity checks */
    if(up != nil && up->nlocks.ref)
        print("qlock: %#p: nlocks %lud\n", getcallerpc(&q), up->nlocks.ref);
    /*x: [[qlock()]] sanity checks */
    if(cpu->ilockdepth != 0)
        print("qlock: %#p: ilockdepth %d\n", getcallerpc(&q), cpu->ilockdepth);
    /*e: [[qlock()]] sanity checks */
    lock(&q->use);
    /*s: [[qlock()]] start of qlock, stat */
    rwstats.qlock++;
    /*e: [[qlock()]] start of qlock, stat */
    if(!q->locked) {
        q->locked = true;
        q->qpc = getcallerpc(&q);
        unlock(&q->use);
        return;
    }
    // else
    /*s: [[qlock()]] sanity check up */
    if(up == nil)
        panic("qlock");
    /*e: [[qlock()]] sanity check up */
    /*s: [[qlock()]] when qlock already held, stat */
    rwstats.qlockq++;
    /*e: [[qlock()]] when qlock already held, stat */

    // add_queue(q, up)
    p = q->tail;
    if(p == nil)
        q->head = up;
    else
        p->qnext = up;
    q->tail = up;
    // up->qnext could be non nil before? no otherwise that means
    // the process is already waiting for a lock and so had no occasion
    // to run another qlock() instruction.
    up->qnext = nil;

    up->state = Queueing;
    up->qpc = getcallerpc(&q);
    unlock(&q->use);

    // switch to another process! 
    sched(); 
    // will resume here when another process unlock() the lock and ready() us
    q->qpc = getcallerpc(&q);
}
/*e: function qlock */

/*s: function canqlock */
bool
canqlock(QLock *q)
{
    if(!canlock(&q->use))
        return false;
    if(q->locked){
        unlock(&q->use);
        return false;
    }
    // else

    q->locked = true;
    q->qpc = getcallerpc(&q);
    unlock(&q->use);
    return true;
}
/*e: function canqlock */

/*s: function qunlock */
void
qunlock(QLock *q)
{
    Proc *p;

    lock(&q->use);
    /*s: [[qunlock()]] sanity checks */
    if(!q->locked)
        print("qunlock called with qlock not held, from %#p\n",
            getcallerpc(&q));
    /*e: [[qunlock()]] sanity checks */

    p = q->head;
    if(p){
        // dequeue(q)
        q->head = p->qnext;
        if(q->head == nil)
            q->tail = nil;

        unlock(&q->use);
        ready(p);
    }else{
        q->locked = false;
        q->qpc = nilptr;
        unlock(&q->use);
    }
}
/*e: function qunlock */

/*s: function rlock */
void
rlock(RWlock *q)
{
    Proc *p;

    lock(&q->use);
    rwstats.rlock++;
    if(!q->writer && q->head == nil){
        /* no writer, go for it */
        q->readers++;
        unlock(&q->use);
        return;
    }
    // else, a writer or a waiting queue??

    /*s: [[rlock()]] sanity check up */
    if(up == nil)
        panic("rlock");
    /*e: [[rlock()]] sanity check up */
    rwstats.rlockq++;

    // add_queue(q, up)
    p = q->tail;
    if(p == nil)
        q->head = up;
    else
        p->qnext = up;
    q->tail = up;
    up->qnext = nil;

    up->state = QueueingR;
    unlock(&q->use);
    sched();
    // will resume here when another process unlock() the lock and ready() us
}
/*e: function rlock */

/*s: function runlock */
void
runlock(RWlock *q)
{
    Proc *p;

    lock(&q->use);
    p = q->head;
    if(--(q->readers) > 0 || p == nil){
        unlock(&q->use);
        return;
    }
    // else, p != nil && q->readers == 0

    /* start waiting writer */
    if(p->state != QueueingW)
        panic("runlock");

    // dequeue(q)
    q->head = p->qnext;
    if(q->head == nil)
        q->tail = nil;

    q->writer = true;
    unlock(&q->use);
    ready(p);
}
/*e: function runlock */

/*s: function wlock */
void
wlock(RWlock *q)
{
    Proc *p;

    lock(&q->use);
    rwstats.wlock++;
    if(q->readers == 0 && !q->writer){
        /* noone waiting, go for it */
        q->wpc = getcallerpc(&q);
        q->wproc = up;
        q->writer = true;
        unlock(&q->use);
        return;
    }
    // else, already a writer or readers

    /* wait */
    /*s: [[wlock()]] sanity check up */
    if(up == nil)
        panic("wlock");
    /*e: [[wlock()]] sanity check up */
    rwstats.wlockq++;

    // add_queue(q, up)
    p = q->tail;
    if(p == nil)
        q->head = up;
    else
        p->qnext = up;
    q->tail = up;
    up->qnext = nil;

    up->state = QueueingW;
    unlock(&q->use);
    sched();
}
/*e: function wlock */

/*s: function wunlock */
void
wunlock(RWlock *q)
{
    Proc *p;

    lock(&q->use);
    p = q->head;
    if(p == nil){
        q->writer = false;
        unlock(&q->use);
        return;
    }
    if(p->state == QueueingW){
        /* start waiting writer */
        // dequeue(q)
        q->head = p->qnext;
        if(q->head == nil)
            q->tail = nil;

        unlock(&q->use);
        ready(p);
        return;
    }

    if(p->state != QueueingR)
        panic("wunlock");

    /* waken waiting readers */
    while(q->head != nil && q->head->state == QueueingR){
        p = q->head;
        q->head = p->qnext;
        q->readers++;
        ready(p);
    }

    if(q->head == nil)
        q->tail = nil;
    q->writer = false;
    unlock(&q->use);
}
/*e: function wunlock */

/*s: function canrlock */
/* same as rlock but punts if there are any writers waiting */
bool
canrlock(RWlock *q)
{
    lock(&q->use);
    rwstats.rlock++;
    if(q->writer == false && q->head == nil){
        /* no writer, go for it */
        q->readers++;
        unlock(&q->use);
        return true;
    }
    unlock(&q->use);
    return false;
}
/*e: function canrlock */
/*e: qlock.c */
