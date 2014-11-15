/*s: lib_core/libthread/xincport.h */
#include <u.h>
#include <libc.h>
#include <thread.h>

/*s: global xincport_lock */
static Lock xincport_lock;
/*e: global xincport_lock */

/*s: function _xinc */
void
_xinc(long *p)
{

    lock(&xincport_lock);
    (*p)++;
    unlock(&xincport_lock);
}
/*e: function _xinc */

/*s: function _xdec */
long
_xdec(long *p)
{
    long r;

    lock(&xincport_lock);
    r = --(*p);
    unlock(&xincport_lock);
    return r;
}
/*e: function _xdec */
/*e: lib_core/libthread/xincport.h */
