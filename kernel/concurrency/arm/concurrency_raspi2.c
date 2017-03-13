/*s: concurrency/arm/concurrency_raspi2.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

extern long _xdec(long *p);
extern long _xinc(long *p);

// use libc version?? ok??
long
arch_xdec(long *p)
{
  return _xdec(p);
}

void
arch_xinc(long *p)
{
  _xinc(p);
}

int
arch_cmpswap(long *addr, long old, long new)
{
    return cas((ulong*)addr, old, new);
}
/*e: concurrency/arm/concurrency_raspi2.c */
