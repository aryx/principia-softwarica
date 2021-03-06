/*s: processes/386/main_processes.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: function [[procsetup]](x86) */
void
arch_procsetup(Proc* p)
{
  /*s: [[procsetup()]] fp setup(x86) */
  /*
   *  set up floating point for a new process
   */
      p->fpstate = FPinit;
      fpoff();
  /*e: [[procsetup()]] fp setup(x86) */
}
/*e: function [[procsetup]](x86) */

/*s: function [[procsave]](x86) */
/*
 *  Save the cpu dependent part of the process state.
 */
void
arch_procsave(Proc *p)
{
    /*s: [[arch_procsave()]] cycles adjustments */
    uvlong t;

    arch_cycles(&t);
    p->pcycles += t;
    /*e: [[arch_procsave()]] cycles adjustments */
    /*s: [[procsave()]] fp adjustments(x86) */
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
    /*e: [[procsave()]] fp adjustments(x86) */
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
/*e: function [[procsave]](x86) */

/*s: function [[procrestore]](x86) */
void
arch_procrestore(Proc *p)
{
    uvlong t;

    if(p->kp)
        return;
    /*s: [[arch_procrestore]] cycles adjustments */
    arch_cycles(&t);
    p->pcycles -= t;
    /*e: [[arch_procrestore]] cycles adjustments */
}
/*e: function [[procrestore]](x86) */

/*s: function [[fpsavealloc]](x86) */
void
fpsavealloc(void)
{
    cpu->fpsavalign = mallocalign(sizeof(FPssestate), FPalign, 0, 0);
    if (cpu->fpsavalign == nil)
        panic("cpu%d: can't allocate fpsavalign", cpu->cpuno);
}
/*e: function [[fpsavealloc]](x86) */

/*s: function [[fpssesave]](x86) */
/*
 * sse fp save and restore buffers have to be 16-byte (FPalign) aligned,
 * so we shuffle the data down as needed or make copies.
 */

void
fpssesave(Arch_FPsave *fps)
{
    Arch_FPsave *afps;

    fps->magic = 0x1234;
    afps = (Arch_FPsave *)ROUND(((uintptr)fps), FPalign);
    fpssesave0(afps);
    if (fps != afps)  /* not aligned? shuffle down from aligned buffer */
        memmove(fps, afps, sizeof(FPssestate));
    if (fps->magic != 0x1234)
        print("fpssesave: magic corrupted\n");
}
/*e: function [[fpssesave]](x86) */

/*s: function [[fpsserestore]](x86) */
void
fpsserestore(Arch_FPsave *fps)
{
    Arch_FPsave *afps;

    fps->magic = 0x4321;
    afps = (Arch_FPsave *)ROUND(((uintptr)fps), FPalign);
    if (fps != afps) {
        afps = cpu->fpsavalign;
        memmove(afps, fps, sizeof(FPssestate)); /* make aligned copy */
    }
    fpsserestore0(afps);
    if (fps->magic != 0x4321)
        print("fpsserestore: magic corrupted\n");
}
/*e: function [[fpsserestore]](x86) */

/*s: function [[idlehands]](x86) */
// current configuration
static bool idle_spin = false;
static int idle_if_nproc = 0;


/*
 *  put the processor in the halt state if we've no processes to run.
 *  an interrupt will get us going again.
 */
void
arch_idlehands(void)
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
/*e: function [[idlehands]](x86) */
/*e: processes/386/main_processes.c */
