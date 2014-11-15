/*s: lib_core/libthread/386.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function launcher386 */
static void
launcher386(void (*f)(void *arg), void *arg)
{
    (*f)(arg);
    threadexits(nil);
}
/*e: function launcher386 */

/*s: function _threadinitstack */
void
_threadinitstack(Thread *t, void (*f)(void*), void *arg)
{
    ulong *tos;

    tos = (ulong*)&t->stk[t->stksize&~7];
    *--tos = (ulong)arg;
    *--tos = (ulong)f;
    t->sched[JMPBUFPC] = (ulong)launcher386+JMPBUFDPC;
    t->sched[JMPBUFSP] = (ulong)tos - 8;		/* old PC and new PC */
}
/*e: function _threadinitstack */

/*e: lib_core/libthread/386.c */
