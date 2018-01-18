/*s: libthread/kill.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

static void tinterrupt(Proc*, Thread*);

/*s: function [[threadxxxgrp]] */
static void
threadxxxgrp(int grp, int dokill)
{
    Proc *p;
    Thread *t;

    lock(&_threadpq.lock);
    for(p=_threadpq.head; p; p=p->next){
        lock(&p->lock);
        for(t=p->threads.head; t; t=t->nextt)
            if(t->grp == grp){
                if(dokill)
                    t->moribund = 1;
                tinterrupt(p, t);
            }
        unlock(&p->lock);
    }
    unlock(&_threadpq.lock);
    _threadbreakrendez();
}
/*e: function [[threadxxxgrp]] */

/*s: function [[threadxxx]] */
static void
threadxxx(int id, int dokill)
{
    Proc *p;
    Thread *t;

    lock(&_threadpq.lock);
    for(p=_threadpq.head; p; p=p->next){
        lock(&p->lock);
        for(t=p->threads.head; t; t=t->nextt)
            if(t->id == id){
                if(dokill)
                    t->moribund = 1;
                tinterrupt(p, t);
                unlock(&p->lock);
                unlock(&_threadpq.lock);
                _threadbreakrendez();
                return;
            }
        unlock(&p->lock);
    }
    unlock(&_threadpq.lock);
    _threaddebug(DBGNOTE, "Can't find thread to kill");
    return;
}
/*e: function [[threadxxx]] */

/*s: function [[threadkillgrp]] */
void
threadkillgrp(int grp)
{
    threadxxxgrp(grp, 1);
}
/*e: function [[threadkillgrp]] */

/*s: function [[threadkill]] */
void
threadkill(int id)
{
    threadxxx(id, 1);
}
/*e: function [[threadkill]] */

/*s: function [[threadintgrp]] */
void
threadintgrp(int grp)
{
    threadxxxgrp(grp, 0);
}
/*e: function [[threadintgrp]] */

/*s: function [[threadint]] */
void
threadint(int id)
{
    threadxxx(id, 0);
}
/*e: function [[threadint]] */

/*s: function [[tinterrupt]] */
static void
tinterrupt(Proc *p, Thread *t)
{
    switch(t->state){
    case Running:
        postnote(PNPROC, p->pid, "threadint");
        break;
    case Rendezvous:
        _threadflagrendez(t);
        break;
    }
}
/*e: function [[tinterrupt]] */
/*e: libthread/kill.c */
