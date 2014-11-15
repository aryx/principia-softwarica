/*s: lib_core/libthread/globals.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: global _threadpq */
// used to be in create.c
Pqueue _threadpq;
/*e: global _threadpq */

/*s: global procp */
// used to be in main.c
static Proc **procp;
/*e: global procp */

/*s: function _systhreadinit */
void
_systhreadinit(void)
{
    procp = privalloc();
}
/*e: function _systhreadinit */

/*s: function _threadgetproc */
Proc*
_threadgetproc(void)
{
    return *procp;
}
/*e: function _threadgetproc */

/*s: function _threadsetproc */
void
_threadsetproc(Proc *p)
{
    *procp = p;
}
/*e: function _threadsetproc */
/*e: lib_core/libthread/globals.c */
