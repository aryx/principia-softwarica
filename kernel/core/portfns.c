/*s: portfns.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// backward dependencies breaker for non functional properties functions
// (logging, security, error, profiling/timing)
/*s: portfns.c backward deps breaker */
// console/devcons.c
int (*print)(char*, ...) = nil;
int (*iprint)(char*, ...) = nil;
int (*pprint)(char *fmt, ...) = nil;

void (*panic)(char*, ...) = nil;
void (*_assert)(char *fmt) = nil;

// process/386/trap.c
void (*dumpstack)(void) = nil;

// process/proc.c
void (*dumpaproc)(Proc*) = nil;

// process/proc.c
void (*error)(char*) = nil;
void (*nexterror)(void) = nil;

void (*sched)(void) = nil;
void (*ready)(Proc*) = nil;
Proc* (*wakeup)(Rendez*) = nil;
void (*sleep)(Rendez*, int(*)(void*), void*) = nil;
void (*tsleep)(Rendez*, int (*)(void*), void*, ulong) = nil;

Proc* (*proctab)(int) = nil;
int (*postnote)(Proc*, int, char*, int) = nil;
void (*pexit)(char*, bool) = nil;

// files/chan.c
void (*cclose)(Chan*);

// init/main.c
void (*exit)(int ispanic) = nil;

// misc/386/devarch.c
void (*arch_coherence)(void) = nil;

// misc/386/devarch.c
uvlong (*arch_fastticks)(uvlong*) = nil;

// processes/386/i8253.c
void (*delay)(int) = nil;
void (*microdelay)(int) = nil;
/*e: portfns.c backward deps breaker */

/*s: function returnfalse */
// usually used as default callback for sleep/tsleep
bool
returnfalse(void*)
{
    return false;
}
/*e: function returnfalse */

// was in devcons.c, could be in lib/misc.c
/*s: function readnum */
int
readnum(ulong off, char *buf, ulong n, ulong val, int size)
{
    char tmp[64];

    snprint(tmp, sizeof(tmp), "%*lud", size-1, val);
    tmp[size-1] = ' ';
    if(off >= size)
        return 0;
    if(off+n > size)
        n = size-off;
    memmove(buf, tmp+off, n);
    return n;
}
/*e: function readnum */

/*s: function readstr */
int
readstr(ulong off, char *buf, ulong n, char *str)
{
    int size;

    size = strlen(str);
    if(off >= size)
        return 0;
    if(off+n > size)
        n = size-off;
    memmove(buf, str+off, n);
    return n;
}
/*e: function readstr */
/*e: portfns.c */
