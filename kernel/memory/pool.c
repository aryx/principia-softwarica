/*s: pool.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    <pool.h>

/*s: pool.c forward decl */
typedef struct Private  Private;
/*e: pool.c forward decl */

//*****************************************************************************
// Concurrency
//*****************************************************************************

// See Pool.private, for mutual exclusion on memory pools
/*s: pool.c struct Private */
struct Private {
    Lock        lk;
    char        msg[256]; /* a rock for messages to be printed at unlock */
};
/*e: pool.c struct Private */

//*****************************************************************************
// Pool methods
//*****************************************************************************

/*s: function poolprint */
/*
 * because we can't print while we're holding the locks, 
 * we have the save the message and print it once we let go.
 */
static void
poolprint(Pool *p, char *fmt, ...)
{
    va_list v;
    Private *pv;

    pv = p->private;
    va_start(v, fmt);
    vseprint(pv->msg+strlen(pv->msg), pv->msg+sizeof pv->msg, fmt, v);
    va_end(v);
}
/*e: function poolprint */

/*s: function ppanic */
static void
ppanic(Pool *p, char *fmt, ...)
{
    va_list v;
    Private *pv;
    char msg[sizeof pv->msg];

    pv = p->private;
    va_start(v, fmt);
    vseprint(pv->msg+strlen(pv->msg), pv->msg+sizeof pv->msg, fmt, v);
    va_end(v);
    memmove(msg, pv->msg, sizeof msg);
    iunlock(&pv->lk);
    panic("%s", msg);
}
/*e: function ppanic */

/*s: function plock */
static void
plock(Pool *p)
{
    Private *pv;

    pv = p->private;
    ilock(&pv->lk);
    pv->lk.pc = getcallerpc(&p);
    pv->msg[0] = 0;
}
/*e: function plock */

/*s: function punlock */
static void
punlock(Pool *p)
{
    Private *pv;
    char msg[sizeof pv->msg];

    pv = p->private;
    if(pv->msg[0] == 0){
        iunlock(&pv->lk);
        return;
    }

    memmove(msg, pv->msg, sizeof msg);
    iunlock(&pv->lk);
    iprint("%.*s", sizeof pv->msg, msg);
}
/*e: function punlock */

/*s: function poolsummary */
void
poolsummary(Pool *p)
{
    print("%s max %lud cur %lud free %lud alloc %lud\n", p->name,
        p->maxsize, p->cursize, p->curfree, p->curalloc);
}
/*e: function poolsummary */

//*****************************************************************************
// The globals
//*****************************************************************************

/*s: global pmainmem */
static Private pmainpriv;
static Pool pmainmem = {
    .name=  "Main",
    .maxsize=   4*MB,
    .minarena=  128*KB,
    .quantum=   32,
    .alloc= xalloc,
    .merge= xmerge,
    .flags= POOL_TOLERANCE,

    .lock=plock,
    .unlock= punlock,
    .print= poolprint,
    .panic= ppanic,

    .private=   &pmainpriv,
};
/*e: global pmainmem */

/*s: global pimagmem */
static Private pimagpriv;
static Pool pimagmem = {
    .name=  "Image",
    .maxsize=   16*MB,
    .minarena=  2*MB,
    .quantum=   32,
    .alloc= xalloc,
    .merge= xmerge,
    .flags= 0,

    .lock= plock,
    .unlock= punlock,
    .print= poolprint,
    .panic= ppanic,

    .private=   &pimagpriv,
};
/*e: global pimagmem */

/*s: global mainmem and imagmem */
// exported in include/pool.h, defined here!
Pool*   mainmem = &pmainmem;
Pool*   imagmem = &pimagmem;
/*e: global mainmem and imagmem */

/*s: function mallosummary */
void
mallocsummary(void)
{
    poolsummary(mainmem);
    poolsummary(imagmem);
}
/*e: function mallosummary */

/*e: pool.c */
