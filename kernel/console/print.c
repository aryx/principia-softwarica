/*s: print.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
// overrides functions defined in lib_core/libc/fmt/fmtlock.c 
// to behave in a special way when libc functions related to fmt
// are called in the kernel.

static Lock fmtl;

//@Scheck: no dead, called from libc
void _fmtlock(void)
{
    lock(&fmtl);
}

//@Scheck: no dead, called from libc
void _fmtunlock(void)
{
    unlock(&fmtl);
}

//@Scheck: no dead, called from libc
int _efgfmt(Fmt*)
{
    return -1;
}
/*e: print.c */
