/*s: windows/rio/threads_worker.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"

/*s: global [[xfidfree]] */
// list<ref_own<Xfid>> (next = Xfid.free)
static	Xfid	*xfidfree;
/*e: global [[xfidfree]] */
/*s: global [[xfid]] */
// list<ref_own<Xfid>> (next = Xfid.next)
static	Xfid	*xfid;
/*e: global [[xfid]] */
/*s: global [[cxfidalloc]] */
// chan<ref<Xfid>> (listener = filsysproc, sender = xfidallocthread)
static	Channel	*cxfidalloc;	/* chan(Xfid*) */
/*e: global [[cxfidalloc]] */
/*s: global [[cxfidfree]] */
// chan<ref<Xfid>> (listner = ??, sender = ??)
static	Channel	*cxfidfree;	/* chan(Xfid*) */
/*e: global [[cxfidfree]] */

/*s: enum [[Xxxx]] */
enum { 
    Alloc, 
    Free, 

    N 
};
/*e: enum [[Xxxx]] */

/*s: function [[xfidctl]] */
void
xfidctl(void *arg)
{
    Xfid *x = arg;
    void (*f)(Xfid*);
    char buf[64];

    snprint(buf, sizeof buf, "xfid.%p", x);

    threadsetname(buf);

    for(;;){
        f = recvp(x->c);

        // Executing a xfidxxx()
        (*f)(x);

        if(decref(x) == 0)
            sendp(cxfidfree, x);
    }
}
/*e: function [[xfidctl]] */

/*s: function [[xfidflush]] */
void
xfidflush(Xfid *x)
{
    Fcall fc;
    Xfid *xf;

    for(xf=xfid; xf; xf=xf->next)
        if(xf->flushtag == x->req.oldtag){
            xf->flushtag = -1;
            xf->flushing = true;
            incref(xf);	/* to hold data structures up at tail of synchronization */
            if(xf->ref == 1)
                error("ref 1 in flush");
            if(canqlock(&xf->active)){
                qunlock(&xf->active);
                sendul(xf->flushc, 0);
            }else{
                qlock(&xf->active);	/* wait for him to finish */
                qunlock(&xf->active);
            }
            xf->flushing = false;

            if(decref(xf) == 0)
                sendp(cxfidfree, xf);
            break;
        }
    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function [[xfidflush]] */

/*s: function [[xfidallocthread]] */
void
xfidallocthread(void*)
{
    Xfid *x;
    static Alt alts[N+1];

    alts[Alloc].c = cxfidalloc;
    alts[Alloc].v = nil;
    alts[Alloc].op = CHANRCV;
    alts[Free].c = cxfidfree;
    alts[Free].v = &x;
    alts[Free].op = CHANRCV;
    alts[N].op = CHANEND;

    for(;;){
        // event loop
        switch(alt(alts)){
        case Alloc:
            x = xfidfree;
            if(x)
                xfidfree = x->free;
            else{
                x = emalloc(sizeof(Xfid));
                x->c = chancreate(sizeof(void(*)(Xfid*)), 0);
                /*s: [[xfidallocthread()]] create flushc channel */
                x->flushc = chancreate(sizeof(int), 0);	/* notification only; nodata */
                x->flushtag = -1;
                /*e: [[xfidallocthread()]] create flushc channel */

                // insert_list(x, xfid)
                x->next = xfid;
                xfid = x;

                // new Xfid threads!
                threadcreate(xfidctl, x, 16384);
            }
            /*s: [[xfidallocthread()]] sanity check x when Alloc */
            if(x->ref != 0){
                fprint(STDERR, "%p incref %ld\n", x, x->ref);
                error("incref");
            }
            if(x->flushtag != -1)
                error("flushtag in allocate");
            /*e: [[xfidallocthread()]] sanity check x when Alloc */
            incref(x);

            sendp(cxfidalloc, x);
            break;

        case Free:
            /*s: [[xfidallocthread()]] sanity check x when Free */
            if(x->ref != 0){
                fprint(STDERR, "%p decref %ld\n", x, x->ref);
                error("decref");
            }
            if(x->flushtag != -1)
                error("flushtag in free");
            /*e: [[xfidallocthread()]] sanity check x when Free */
            // insert_list(x, xfidfree)
            x->free = xfidfree;
            xfidfree = x;
            break;
        }
    }
}
/*e: function [[xfidallocthread]] */

/*s: function [[xfidinit]] */
Channel*
xfidinit(void)
{
    cxfidalloc = chancreate(sizeof(Xfid*), 0);
    cxfidfree = chancreate(sizeof(Xfid*), 0);
    threadcreate(xfidallocthread, nil, STACK);
    return cxfidalloc;
}
/*e: function [[xfidinit]] */
/*e: windows/rio/threads_worker.c */
