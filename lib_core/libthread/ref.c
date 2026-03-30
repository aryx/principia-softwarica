/*s: libthread/ref.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <thread.h>
#include "threadimpl.h"

/*s: function [[incref]] */
void
incref(Ref *r)
{
    ainc(&r->ref);
}
/*e: function [[incref]] */
/*s: function [[decref]] */
long
decref(Ref *r)
{
    return adec(&r->ref);
}
/*e: function [[decref]] */
/*e: libthread/ref.c */
