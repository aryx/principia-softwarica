/*s: libthread/debug.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: global [[_threaddebuglevel]] */
// biset<enum<dbgxxx>>
int _threaddebuglevel;
/*e: global [[_threaddebuglevel]] */

/*s: function [[_threaddebug]] */
void
_threaddebug(ulong flag, char *fmt, ...)
{
    char buf[128];
    va_list arg;
    Fmt f;
    Proc *p;

    if((_threaddebuglevel&flag) == 0)
        return;

    fmtfdinit(&f, 2, buf, sizeof buf);

    p = _threadgetproc();
    if(p==nil)
        fmtprint(&f, "noproc ");
    else if(p->thread)
        fmtprint(&f, "%d.%d ", p->pid, p->thread->id);
    else
        fmtprint(&f, "%d._ ", p->pid);

    va_start(arg, fmt);
    fmtvprint(&f, fmt, arg);
    va_end(arg);
    fmtprint(&f, "\n");
    fmtfdflush(&f);
}
/*e: function [[_threaddebug]] */

/*s: function [[_threadassert]] */
void
_threadassert(char *s)
{
    char buf[256];
    int n;
    Proc *p;

    p = _threadgetproc();
    if(p && p->thread)
        n = sprint(buf, "%d.%d ", p->pid, p->thread->id);
    else
        n = 0;
    snprint(buf+n, sizeof(buf)-n, "%s: assertion failed\n", s);
    write(2, buf, strlen(buf));
    abort();
}
/*e: function [[_threadassert]] */
/*e: libthread/debug.c */
