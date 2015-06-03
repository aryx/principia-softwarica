/*s: kernel/network/ip/loopbackmedium.c */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

#include "ip.h"

/*s: enum _anon_ (kernel/network/ip/loopbackmedium.c) */
enum
{
    Maxtu=  16*1024,
};
/*e: enum _anon_ (kernel/network/ip/loopbackmedium.c) */

typedef struct LB LB;
/*s: struct LB */
struct LB
{
    Queue   *q;
    Fs  *f;
    Proc    *readp;
};
/*e: struct LB */

static void loopbackread(void *a);

/*s: function loopbackbind */
static void
loopbackbind(Ipifc *ifc, int, char**)
{
    LB *lb;

    lb = smalloc(sizeof(LB));
    lb->f = ifc->conv->p->f;
    lb->q = qopen(1024*1024, Qmsg, nil, nil);
    ifc->arg = lb;
    ifc->mbps = 1000;

    kproc("loopbackread", loopbackread, ifc);

}
/*e: function loopbackbind */

/*s: function loopbackunbind */
static void
loopbackunbind(Ipifc *ifc)
{
    LB *lb = ifc->arg;

    if(lb->readp)
        postnote(lb->readp, 1, "unbind", 0);

    /* wait for reader to die */
    while(lb->readp != nil)
        tsleep(&up->sleepr, returnfalse, 0, 300);

    /* clean up */
    qfree(lb->q);
    free(lb);
}
/*e: function loopbackunbind */

/*s: function loopbackbwrite */
static void
loopbackbwrite(Ipifc *ifc, Block *bp, int, uchar*)
{
    LB *lb;

    lb = ifc->arg;
    if(qpass(lb->q, bp) < 0)
        ifc->outerr++;
    ifc->out++;
}
/*e: function loopbackbwrite */

/*s: function loopbackread */
static void
loopbackread(void *a)
{
    Ipifc *ifc;
    Block *bp;
    LB *lb;

    ifc = a;
    lb = ifc->arg;
    lb->readp = up; /* hide identity under a rock for unbind */
    if(waserror()){
        lb->readp = 0;
        pexit("hangup", 1);
    }
    for(;;){
        bp = qbread(lb->q, Maxtu);
        if(bp == nil)
            continue;
        ifc->in++;
        if(!canrlock(ifc)){
            freeb(bp);
            continue;
        }
        if(waserror()){
            runlock(ifc);
            nexterror();
        }
        if(ifc->lifc == nil)
            freeb(bp);
        else
            ipiput4(lb->f, ifc, bp);
        runlock(ifc);
        poperror();
    }
}
/*e: function loopbackread */

/*s: global loopbackmedium */
Medium loopbackmedium =
{
    .name=      "loopback",

    .hsize=     0,
    .mintu=     0,
    .maxtu=     Maxtu,
    .maclen=    0,

    .bind=      loopbackbind,
    .unbind=    loopbackunbind,
    .bwrite=    loopbackbwrite,
};
/*e: global loopbackmedium */

/*s: function loopbackmediumlink */
void
loopbackmediumlink(void)
{
    addipmedium(&loopbackmedium);
}
/*e: function loopbackmediumlink */
/*e: kernel/network/ip/loopbackmedium.c */
