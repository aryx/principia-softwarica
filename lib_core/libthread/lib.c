/*s: libthread/lib.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: global [[totalmalloc]] */
static long totalmalloc;
/*e: global [[totalmalloc]] */

/*s: function [[_threadmalloc]] */
void*
_threadmalloc(long size, int z)
{
    void *m;

    m = malloc(size);
    if (m == nil)
        sysfatal("Malloc of size %ld failed: %r", size);
    setmalloctag(m, getcallerpc(&size));
    totalmalloc += size;
    if (size > 100000000) {
        fprint(2, "Malloc of size %ld, total %ld\n", size, totalmalloc);
        abort();
    }
    if (z)
        memset(m, 0, size);
    return m;
}
/*e: function [[_threadmalloc]] */

/*s: function [[_threadsysfatal]] */
void
_threadsysfatal(char *fmt, va_list arg)
{
    char buf[1024];	/* size doesn't matter; we're about to exit */

    vseprint(buf, buf+sizeof(buf), fmt, arg);
    if(argv0)
        fprint(2, "%s: %s\n", argv0, buf);
    else
        fprint(2, "%s\n", buf);
    threadexitsall(buf);
}
/*e: function [[_threadsysfatal]] */
/*e: libthread/lib.c */
