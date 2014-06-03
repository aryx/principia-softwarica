/*s: main_processes.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: function procsetup */
/*
 *  set up floating point for a new process
 */
void
procsetup(Proc*p)
{
    p->fpstate = FPinit;
    fpoff();
}
/*e: function procsetup */

/*s: function procsave */
/*
 *  Save the cpu dependent part of the process state.
 */
void
procsave(Proc *p)
{
    uvlong t;

    cycles(&t);
    p->pcycles += t;
    if(p->fpstate == FPactive){
        if(p->state == Moribund)
            fpclear();
        else{
            /*
             * Fpsave() stores without handling pending
             * unmasked exeptions. Postnote() can't be called
             * here as sleep() already has up->rlock, so
             * the handling of pending exceptions is delayed
             * until the process runs again and generates an
             * emulation fault to activate the FPU.
             */
            fpsave(&p->fpsave);
        }
        p->fpstate = FPinactive;
    }

    /*
     * While this processor is in the scheduler, the process could run
     * on another processor and exit, returning the page tables to
     * the free list where they could be reallocated and overwritten.
     * When this processor eventually has to get an entry from the
     * trashed page tables it will crash.
     *
     * If there's only one processor, this can't happen.
     * You might think it would be a win not to do this in that case,
     * especially on VMware, but it turns out not to matter.
     */
    mmuflushtlb(PADDR(cpu->pdproto));
}
/*e: function procsave */

/*s: function procrestore */
void
procrestore(Proc *p)
{
    uvlong t;

    if(p->kp)
        return;
    cycles(&t);
    p->pcycles -= t;
}
/*e: function procrestore */

/*s: function fpsavealloc */
void
fpsavealloc(void)
{
    cpu->fpsavalign = mallocalign(sizeof(FPssestate), FPalign, 0, 0);
    if (cpu->fpsavalign == nil)
        panic("cpu%d: can't allocate fpsavalign", cpu->cpuno);
}
/*e: function fpsavealloc */

/*s: function fpssesave */
/*
 * sse fp save and restore buffers have to be 16-byte (FPalign) aligned,
 * so we shuffle the data down as needed or make copies.
 */

void
fpssesave(ArchFPsave *fps)
{
    ArchFPsave *afps;

    fps->magic = 0x1234;
    afps = (ArchFPsave *)ROUND(((uintptr)fps), FPalign);
    fpssesave0(afps);
    if (fps != afps)  /* not aligned? shuffle down from aligned buffer */
        memmove(fps, afps, sizeof(FPssestate));
    if (fps->magic != 0x1234)
        print("fpssesave: magic corrupted\n");
}
/*e: function fpssesave */

/*s: function fpsserestore */
void
fpsserestore(ArchFPsave *fps)
{
    ArchFPsave *afps;

    fps->magic = 0x4321;
    afps = (ArchFPsave *)ROUND(((uintptr)fps), FPalign);
    if (fps != afps) {
        afps = cpu->fpsavalign;
        memmove(afps, fps, sizeof(FPssestate)); /* make aligned copy */
    }
    fpsserestore0(afps);
    if (fps->magic != 0x4321)
        print("fpsserestore: magic corrupted\n");
}
/*e: function fpsserestore */

/*s: function idlehands */
// current configuration
static bool idle_spin = false;
static int idle_if_nproc = 0;


/*
 *  put the processor in the halt state if we've no processes to run.
 *  an interrupt will get us going again.
 */
void
idlehands(void)
{
    /*
     * we used to halt only on single-core setups. halting in an SMP system 
     * can result in a startup latency for processes that become ready.
     * if idle_spin is false, we care more about saving energy
     * than reducing this latency.
     *
     * the performance loss with idle_spin == false seems to be slight
     * and it reduces lock contention (thus system time and real time)
     * on many-core systems with large values of NPROC.
     */
    if(conf.ncpu == 1 || idle_spin == false ||
        (idle_if_nproc && conf.ncpu >= idle_if_nproc))
        halt();
}
/*e: function idlehands */
/*e: main_processes.c */
