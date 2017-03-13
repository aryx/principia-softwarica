/*s: processes/arm/arch.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include <tos.h>
#include "ureg.h"

#include "arm.h"

/*
 * A lot of this stuff doesn't belong here
 * but this is a convenient dumping ground for
 * later sorting into the appropriate buckets.
 */

/*s: function arch_userpc(arm) */
/*
 *  return the userpc the last exception happened at
 */
uintptr
arch_userpc(void)
{
    Ureg *ureg = up->dbgreg;
    return ureg->pc;
}
/*e: function arch_userpc(arm) */

/*s: function arch_dbgpc(arm) */
/*
 *  pc output by dumpaproc
 */
uintptr
arch_dbgpc(Proc* p)
{
    Ureg *ureg;

    ureg = p->dbgreg;
    if(ureg == 0)
        return 0;

    return ureg->pc;
}
/*e: function arch_dbgpc(arm) */


/*s: function arch_userureg(arm) */
bool
arch_userureg(Ureg* ureg)
{
    return (ureg->psr & PsrMask) == PsrMusr;
}
/*e: function arch_userureg(arm) */




/*s: function arch_setregisters(arm) */
/* This routine must save the values of registers the user is not permitted
 * to write from devproc and then restore the saved values before returning.
 */
void
arch_setregisters(Ureg* ureg, char* pureg, char* uva, int n)
{
    USED(ureg, pureg, uva, n);
}
/*e: function arch_setregisters(arm) */

/*s: function arch_setkernur(arm) */
/* Give enough context in the ureg to produce a kernel stack for
 * a sleeping process
 */
void
arch_setkernur(Ureg* ureg, Proc* p)
{
    ureg->pc = p->sched.pc;
    ureg->sp = p->sched.sp+4;
    ureg->r14 = PTR2UINT(sched);
}
/*e: function arch_setkernur(arm) */

/*s: function arch_validalign(arm) */
/*
 * called in syscallfmt.c, sysfile.c, sysproc.c
 */
void
arch_validalign(uintptr addr, unsigned align)
{
    /*
     * Plan 9 is a 32-bit O/S, and the hardware it runs on
     * does not usually have instructions which move 64-bit
     * quantities directly, synthesizing the operations
     * with 32-bit move instructions. Therefore, the compiler
     * (and hardware) usually only enforce 32-bit alignment,
     * if at all.
     *
     * Take this out if the architecture warrants it.
     */
    if(align == sizeof(vlong))
        align = sizeof(long);

    /*
     * Check align is a power of 2, then addr alignment.
     */
    if((align != 0 && !(align & (align-1))) && !(addr & (align-1)))
        return;
    postnote(up, 1, "sys: odd address", NDebug);
    error(Ebadarg);
    /*NOTREACHED*/
}
/*e: function arch_validalign(arm) */

/*s: function kexit(arm) */
/* go to user space */
void
kexit(Ureg*)
{
    /*s: [[kexit()]] tos adjustments */
        uvlong t;
        Tos *tos;

        /* precise time accounting, kernel exit */
        tos = (Tos*)(USTKTOP-sizeof(Tos));
        arch_cycles(&t);
        tos->kcycles += t - up->kentry;
        tos->pcycles = up->pcycles;
        tos->pid = up->pid;
    /*e: [[kexit()]] tos adjustments */
    tos->cyclefreq = cpu->cpuhz;

    /* make visible immediately to user proc */
    cachedwbinvse(tos, sizeof *tos);
}
/*e: function kexit(arm) */


/*s: function linkproc(arm) */
/*
 *  this is the body for all kproc's
 */
static void
linkproc(void)
{
    arch_spllo();
    up->kpfun(up->kparg);
    // should never reach this place?? kernel processes are supposed
    // to run forever??
    pexit("kproc exiting", 0);
}
/*e: function linkproc(arm) */

/*s: function arch_kprocchild(arm) */
/*
 *  setup stack and initial PC for a new kernel proc.  This is architecture
 *  dependent because of the starting stack location
 */
void
arch_kprocchild(Proc *p, void (*func)(void*), void *arg)
{
    p->sched.pc = PTR2UINT(linkproc);
    p->sched.sp = PTR2UINT(p->kstack+KSTACK);

    p->kpfun = func;
    p->kparg = arg;
}
/*e: function arch_kprocchild(arm) */
/*e: processes/arm/arch.c */
