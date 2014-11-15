/*s: lib_core/libthread/ref.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function incref */
void
incref(Ref *r)
{
    ainc(&r->ref);
}
/*e: function incref */

/*s: function decref */
long
decref(Ref *r)
{
    return adec(&r->ref);
}
/*e: function decref */
/*e: lib_core/libthread/ref.c */
