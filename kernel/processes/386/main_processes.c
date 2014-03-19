#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

//TODO: where this is set??
int idle_spin, idle_if_nproc;

/*
 *  set up floating point for a new process
 */
void
procsetup(Proc*p)
{
	p->fpstate = FPinit;
	fpoff();
}

/*
 *  Save the mach dependent part of the process state.
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
	mmuflushtlb(PADDR(m->pdb));
}



void
procrestore(Proc *p)
{
	uvlong t;

	if(p->kp)
		return;
	cycles(&t);
	p->pcycles -= t;
}


void
fpsavealloc(void)
{
	m->fpsavalign = mallocalign(sizeof(FPssestate), FPalign, 0, 0);
	if (m->fpsavalign == nil)
		panic("cpu%d: can't allocate fpsavalign", m->machno);
}

void
machinit(void)
{
	int machno;
	ulong *pdb;
	Segdesc *gdt;

	machno = m->machno;
	pdb = m->pdb;
	gdt = m->gdt;
	memset(m, 0, sizeof(Mach));
	m->machno = machno;
	m->pdb = pdb;
	m->gdt = gdt;
	m->perf.period = 1;

	/*
	 * For polled uart output at boot, need
	 * a default delay constant. 100000 should
	 * be enough for a while. Cpuidentify will
	 * calculate the real value later.
	 */
	m->loopconst = 100000;
}

/*
 * sse fp save and restore buffers have to be 16-byte (FPalign) aligned,
 * so we shuffle the data down as needed or make copies.
 */

void
fpssesave(FPsave *fps)
{
	FPsave *afps;

	fps->magic = 0x1234;
	afps = (FPsave *)ROUND(((uintptr)fps), FPalign);
	fpssesave0(afps);
	if (fps != afps)  /* not aligned? shuffle down from aligned buffer */
		memmove(fps, afps, sizeof(FPssestate));
	if (fps->magic != 0x1234)
		print("fpssesave: magic corrupted\n");
}

void
fpsserestore(FPsave *fps)
{
	FPsave *afps;

	fps->magic = 0x4321;
	afps = (FPsave *)ROUND(((uintptr)fps), FPalign);
	if (fps != afps) {
		afps = m->fpsavalign;
		memmove(afps, fps, sizeof(FPssestate));	/* make aligned copy */
	}
	fpsserestore0(afps);
	if (fps->magic != 0x4321)
		print("fpsserestore: magic corrupted\n");
}

/*
 *  put the processor in the halt state if we've no processes to run.
 *  an interrupt will get us going again.
 */
void
idlehands(void)
{
	/*
	 * we used to halt only on single-core setups. halting in an smp system 
	 * can result in a startup latency for processes that become ready.
	 * if idle_spin is zero, we care more about saving energy
	 * than reducing this latency.
	 *
	 * the performance loss with idle_spin == 0 seems to be slight
	 * and it reduces lock contention (thus system time and real time)
	 * on many-core systems with large values of NPROC.
	 */
	if(conf.nmach == 1 || idle_spin == 0 ||
	    idle_if_nproc && conf.nmach >= idle_if_nproc)
		halt();
}
