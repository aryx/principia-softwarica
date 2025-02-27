/*s: interrupts/386/trap.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    "io.h"

#include    <tos.h>
#include    <ureg.h>
#include    <trace.h>

//*****************************************************************************
// Globals
//*****************************************************************************

/*s: global [[trapinited]](x86) */
static bool trapinited;
/*e: global [[trapinited]](x86) */

/*s: global [[vctllock]](x86) */
static Lock vctllock;
/*e: global [[vctllock]](x86) */
/*s: global [[vctl]](x86) */
// array<list<ref_own<Vctl>>> (next = Vctl.next)
static Vctl *vctl[256];
/*e: global [[vctl]](x86) */

enum
{
   /*s: constant [[Ntimevec]](x86) */
   Ntimevec = 20       /* number of time buckets for each intr */
   /*e: constant [[Ntimevec]](x86) */
};
/*s: global [[intrtimes]](x86) */
ulong intrtimes[256][Ntimevec];
/*e: global [[intrtimes]](x86) */

//*****************************************************************************
// Forward decl
//*****************************************************************************
/*s: trap.c forward decl(x86) */
void    arch__noted(Ureg*, ulong);
int     notify(Ureg*);
void        dumpregs(Ureg*);

static void debugbpt(Ureg*, void*);
static void fault386(Ureg*, void*);
static void doublefault(Ureg*, void*);
static void unexpected(Ureg*, void*);
static void _dumpstack(Ureg*);

extern void checkpages(void);
/*e: trap.c forward decl(x86) */

//*****************************************************************************
// Interrupts enable/disable
//*****************************************************************************

/*s: function [[intrenable]](x86) */
void
arch_intrenable(int irq, void (*f)(Ureg*, void*), void* a, int tbdf, char *name)
{
    int vno;
    Vctl *v;

    if(f == nil){
        print("intrenable: nil handler for %d, tbdf 0x%uX for %s\n",
            irq, tbdf, name);
        return;
    }

    v = xalloc(sizeof(Vctl));
    v->isintr = true;
    v->irq = irq;
    v->tbdf = tbdf;
    v->f = f;
    v->a = a;
    strncpy(v->name, name, KNAMELEN-1);
    v->name[KNAMELEN-1] = 0;

    ilock(&vctllock);
    vno = arch->intrenable(v); // this should also set v->isr or v->eoi
    if(vno == -1){
        iunlock(&vctllock);
        print("intrenable: couldn't enable irq %d, tbdf 0x%uX for %s\n",
            irq, tbdf, v->name);
        xfree(v);
        return;
    }
    // add_list(vctl[vno], v) and extra check
    if(vctl[vno]){
        if(vctl[vno]->isr != v->isr || vctl[vno]->eoi != v->eoi)
            panic("intrenable: handler: %s %s %#p %#p %#p %#p",
                vctl[vno]->name, v->name,
                vctl[vno]->isr, v->isr, vctl[vno]->eoi, v->eoi);
        v->next = vctl[vno];
    }
    vctl[vno] = v;
    iunlock(&vctllock);
}
/*e: function [[intrenable]](x86) */

/*s: function [[intrdisable]](x86) */
int
intrdisable(int irq, void (*f)(Ureg *, void *), void *a, int tbdf, char *name)
{
    Vctl **pv, *v;
    int vno;

    /*
     * For now, none of this will work with the APIC code,
     * there is no mapping between irq and vector as the IRQ
     * is pretty meaningless.
     */
    if(arch->intrvecno == nil)
        return -1;
    vno = arch->intrvecno(irq);
    ilock(&vctllock);
    pv = &vctl[vno];
    while (*pv &&
          ((*pv)->irq != irq || (*pv)->tbdf != tbdf || (*pv)->f != f || (*pv)->a != a ||
           strcmp((*pv)->name, name)))
        pv = &((*pv)->next);
    assert(*pv);

    v = *pv;
    *pv = (*pv)->next;  /* Link out the entry */

    if(vctl[vno] == nil && arch->intrdisable != nil)
        arch->intrdisable(irq);
    iunlock(&vctllock);
    xfree(v);
    return 0;
}
/*e: function [[intrdisable]](x86) */

//*****************************************************************************
// Init
//*****************************************************************************

/*s: function [[irqallocread]](x86) */
static long
irqallocread(Chan*, void *vbuf, long n, vlong offset)
{
    char *buf, *p, str[2*(11+1)+KNAMELEN+1+1];
    int m, vno;
    long oldn;
    Vctl *v;

    if(n < 0 || offset < 0)
        error(Ebadarg);

    oldn = n;
    buf = vbuf;
    for(vno=0; vno<nelem(vctl); vno++){
        for(v=vctl[vno]; v; v=v->next){
            m = snprint(str, sizeof str, "%11d %11d %.*s\n", vno, v->irq, KNAMELEN, v->name);
            if(m <= offset) /* if do not want this, skip entry */
                offset -= m;
            else{
                /* skip offset bytes */
                m -= offset;
                p = str+offset;
                offset = 0;

                /* write at most max(n,m) bytes */
                if(m > n)
                    m = n;
                memmove(buf, p, m);
                n -= m;
                buf += m;

                if(n == 0)
                    return oldn;
            }
        }
    }
    return oldn - n;
}
/*e: function [[irqallocread]](x86) */

/*s: function [[trapenable]](x86) */
void
trapenable(int vno, void (*f)(Ureg*, void*), void* a, char *name)
{
    Vctl *v;

    if(vno < 0 || vno >= VectorPIC)
        panic("trapenable: vno %d", vno);

    v = xalloc(sizeof(Vctl));
    v->tbdf = BUSUNKNOWN;
    v->f = f;
    v->a = a;
    v->isintr = false;
    strncpy(v->name, name, KNAMELEN);
    v->name[KNAMELEN-1] = 0;

    ilock(&vctllock);
    //add_list(vctl[vno], v)
    v->next = vctl[vno];
    vctl[vno] = v;
    iunlock(&vctllock);
}
/*e: function [[trapenable]](x86) */

/*s: function [[nmienable]](x86) */
static void
nmienable(void)
{
    int x;

    /*
     * Hack: should be locked with NVRAM access.
     */
    outb(0x70, 0x80);       /* NMI latch clear */
    outb(0x70, 0);

    x = inb(0x61) & 0x07;       /* Enable NMI */
    outb(0x61, 0x08|x);
    outb(0x61, x);
}
/*e: function [[nmienable]](x86) */

/*s: function [[trapinit0]](x86) */
/*
 * Minimal trap setup.  Just enough so that we can panic
 * on traps (bugs) during kernel initialization.
 * Called very early - malloc is not yet available.
 */
void
trapinit0(void)
{
    int d1, v;
    kern_addr vaddr;
    Segdesc *idt;

    idt = (Segdesc*)IDTADDR;
    vaddr = (kern_addr)vectortable;
    for(v = 0; v < 256; v++){
        d1 = (vaddr & 0xFFFF0000)|SEGP;
        switch(v){
        case VectorBPT:
            d1 |= SEGPL(3)|SEGIG;
            break;
        case VectorSYSCALL:
            d1 |= SEGPL(3)|SEGIG;
            break;
        default:
            d1 |= SEGPL(0)|SEGIG;
            break;
        }
        idt[v].d0 = (vaddr & 0xFFFF)|(KESEL<<16);
        idt[v].d1 = d1;
        vaddr += 6;
    }
}
/*e: function [[trapinit0]](x86) */

/*s: function [[trapinit]](x86) */
void
arch__trapinit(void)
{
    /*
     * Special traps.
     * Syscall() is called directly without going through trap().
     */
    trapenable(VectorBPT, debugbpt, 0, "debugpt");
    trapenable(VectorPF, fault386, 0, "fault386");

    trapenable(Vector2F, doublefault, 0, "doublefault");
    trapenable(Vector15, unexpected, 0, "unexpected");
    nmienable();

    addarchfile("irqalloc", 0444, irqallocread, nil);
    trapinited = true;
}
/*e: function [[trapinit]](x86) */

//*****************************************************************************
// Misc
//*****************************************************************************

/*s: global [[excname]](x86) */
static char* excname[32] = {
    "divide error",
    "debug exception",
    "nonmaskable interrupt",
    "breakpoint",
    "overflow",
    "bounds check",
    "invalid opcode",
    "coprocessor not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid TSS",
    "segment not present",
    "stack exception",
    "general protection violation",
    "page fault",
    "15 (reserved)",
    "coprocessor error",
    "alignment check",
    "machine check",
    "19 (reserved)",
    "20 (reserved)",
    "21 (reserved)",
    "22 (reserved)",
    "23 (reserved)",
    "24 (reserved)",
    "25 (reserved)",
    "26 (reserved)",
    "27 (reserved)",
    "28 (reserved)",
    "29 (reserved)",
    "30 (reserved)",
    "31 (reserved)",
};
/*e: global [[excname]](x86) */

/*s: function [[intrtime]](x86) */
/*
 *  keep histogram of interrupt service times
 */
void
intrtime(Cpu*, int vno)
{
    ulong diff;
    ulong x;

    x = arch_perfticks();
    diff = x - cpu->perf.intrts;
    cpu->perf.intrts = x;

    cpu->perf.inintr += diff;
    if(up == nil && cpu->perf.inidle > diff)
        cpu->perf.inidle -= diff;

    diff /= cpu->cpumhz*100;      /* quantum = 100microsec */
    if(diff >= Ntimevec)
        diff = Ntimevec-1;
    intrtimes[vno][diff]++;
}
/*e: function [[intrtime]](x86) */

/*s: function [[kexit]](x86) */
/* go to user space */
void
kexit(Ureg*)
{
    /*s: [[arch__kexit()]] tos adjustments */
    uvlong t;
    Tos *tos;

    /* precise time accounting, kernel exit */
    tos = (Tos*)(USTKTOP-sizeof(Tos));
    arch_cycles(&t);
    tos->kcycles += t - up->kentry;
    tos->pcycles = up->pcycles;
    tos->pid = up->pid;
    /*e: [[arch__kexit()]] tos adjustments */
}
/*e: function [[kexit]](x86) */

/*s: function [[trap]](x86) */
/*
 *  All traps come here.  It is slower to have all traps call trap()
 *  rather than directly vectoring the handler. However, this avoids a
 *  lot of code duplication and possible bugs. The only exception is
 *  VectorSYSCALL.
 *  Trap is called with interrupts disabled via interrupt-gates.
 */
//@Scheck: not dead, called from assembly by _strayintr
void trap(Ureg* ureg)
{
    bool clockintr;
    bool user;
    int i, vno;
    char buf[ERRMAX];
    Vctl *ctl, *v;
    Cpu *mach;

    if(!trapinited){
        /* fault386 can give a better error message */
        if(ureg->trap == VectorPF)
            fault386(ureg, nil);
        panic("trap %lud: not ready", ureg->trap);
    }

    cpu->perf.intrts = arch_perfticks();

    user = (ureg->cs & 0xFFFF) == UESEL; // TODO: arch_userureg

    if(user){
        up->dbgreg = ureg;
        /*s: [[trap()]] adjust kentry when interrupt user */
        arch_cycles(&up->kentry);
        /*e: [[trap()]] adjust kentry when interrupt user */
    }
    // else if !user, then that means we interrupted a syscall() which should
    // already have done those things, so no need for redundancy

    clockintr = false;

    vno = ureg->trap;
    if(ctl = vctl[vno]){
        if(ctl->isintr){
            cpu->intr++;
            if(vno >= VectorPIC && vno != VectorSYSCALL)
                cpu->lastintr = ctl->irq;
        }

        if(ctl->isr)
            ctl->isr(vno);
        for(v = ctl; v != nil; v = v->next){
            if(v->f) // this can be null?
                v->f(ureg, v->a);
        }
        if(ctl->eoi)
            ctl->eoi(vno);

        if(ctl->isintr){
            intrtime(cpu, vno);

            if(ctl->irq == IrqCLOCK || ctl->irq == IrqTIMER)
                clockintr = true;

            if(up && !clockintr)
                preempt();
        }
    } // no Vctl?
    else if(vno < nelem(excname) && user){
        arch_spllo();
        snprint(buf, sizeof buf, "sys: trap: %s", excname[vno]);
        postnote(up, 1, buf, NDebug);
    }
    else if(vno >= VectorPIC && vno != VectorSYSCALL){
        /*
         * An unknown interrupt.
         * Check for a default IRQ7. This can happen when
         * the IRQ input goes away before the acknowledge.
         * In this case, a 'default IRQ7' is generated, but
         * the corresponding bit in the ISR isn't set.
         * In fact, just ignore all such interrupts.
         */

        /* call all interrupt routines, just in case */
        for(i = VectorPIC; i <= MaxIrqLAPIC; i++){
            ctl = vctl[i];
            if(ctl == nil)
                continue;
            if(!ctl->isintr)
                continue;
            for(v = ctl; v != nil; v = v->next){
                if(v->f)
                    v->f(ureg, v->a);
            }
            /* should we do this? */
            if(ctl->eoi)
                ctl->eoi(i);
        } // remove? ugly?

        /* clear the interrupt */
        i8259isr(vno);
        /*s: [[trap()]] debugging(x86) */
                if(0)print("cpu%d: spurious interrupt %d, last %d\n",
                    cpu->cpuno, vno, cpu->lastintr);
                if(0)if(conf.ncpu > 1){
                    for(i = 0; i < MAXCPUS; i++){
                        if(!(active.cpus & (1<<i)))
                            continue;
                        mach = CPUS(i);
                        if(cpu->cpuno == mach->cpuno)
                            continue;
                        print(" cpu%d: last %d",
                            mach->cpuno, mach->lastintr);
                    }
                    print("\n");
                }
        /*e: [[trap()]] debugging(x86) */
        cpu->spuriousintr++;
        if(user)
            kexit(ureg);
        return;
    }else{
        if(vno == VectorNMI){
            /*
             * Don't re-enable, it confuses the crash dumps.
            nmienable();
             */
            iprint("cpu%d: NMI PC %#8.8lux\n", cpu->cpuno, ureg->pc);
            while(cpu->cpuno != 0)
                ;
        }
        dumpregs(ureg);
        if(!user){
            ureg->sp = (ulong)&ureg->sp;
            _dumpstack(ureg);
        }
        if(vno < nelem(excname))
            panic("%s", excname[vno]);
        panic("unknown trap/intr: %d", vno);
    }
    arch_splhi(); // possible arch_spllo() done above

    /*s: [[trap()]] if delaysched and clockintr */
    /* delaysched set because we held a lock or because our quantum ended */
    if(up && up->delaysched && clockintr){
        sched();
        arch_splhi();
    }
    /*e: [[trap()]] if delaysched and clockintr */

    if(user){
        if(up->procctl || up->nnote)
            notify(ureg);
        kexit(ureg);
    }
}
/*e: function [[trap]](x86) */

/*s: function [[dumpregs2]](x86) */
/*
 *  dump registers
 */
void
dumpregs2(Ureg* ureg)
{
    if(up)
        iprint("cpu%d: registers for %s %lud\n",
            cpu->cpuno, up->text, up->pid);
    else
        iprint("cpu%d: registers for kernel\n", cpu->cpuno);
    iprint("FLAGS=%luX TRAP=%luX ECODE=%luX PC=%luX",
        ureg->flags, ureg->trap, ureg->ecode, ureg->pc);
    iprint(" SS=%4.4luX USP=%luX\n", ureg->ss & 0xFFFF, ureg->usp);
    iprint("  AX %8.8luX  BX %8.8luX  CX %8.8luX  DX %8.8luX\n",
        ureg->ax, ureg->bx, ureg->cx, ureg->dx);
    iprint("  SI %8.8luX  DI %8.8luX  BP %8.8luX\n",
        ureg->si, ureg->di, ureg->bp);
    iprint("  CS %4.4luX  DS %4.4luX  ES %4.4luX  FS %4.4luX  GS %4.4luX\n",
        ureg->cs & 0xFFFF, ureg->ds & 0xFFFF, ureg->es & 0xFFFF,
        ureg->fs & 0xFFFF, ureg->gs & 0xFFFF);
}
/*e: function [[dumpregs2]](x86) */

/*s: function [[dumpregs]](x86) */
void
dumpregs(Ureg* ureg)
{
    vlong mca, mct;

    dumpregs2(ureg);

    /*
     * Processor control registers.
     * If machine check exception, time stamp counter, page size extensions
     * or enhanced virtual 8086 mode extensions are supported, there is a
     * CR4. If there is a CR4 and machine check extensions, read the machine
     * check address and machine check type registers if RDMSR supported.
     */
    iprint("  CR0 %8.8lux CR2 %8.8lux CR3 %8.8lux",
        getcr0(), getcr2(), getcr3());
    if(cpu->cpuiddx & (Mce|Tsc|Pse|Vmex)){
        iprint(" CR4 %8.8lux", getcr4());
        if((cpu->cpuiddx & (Mce|Cpumsr)) == (Mce|Cpumsr)){
            rdmsr(0x00, &mca);
            rdmsr(0x01, &mct);
            iprint("\n  MCA %8.8llux MCT %8.8llux", mca, mct);
        }
    }
    iprint("\n  ur %#p up %#p\n", ureg, up);
}
/*e: function [[dumpregs]](x86) */

/*s: function [[callwithureg]](x86) */
/*
 * Fill in enough of Ureg to get a stack trace, and call a function.
 * Used by debugging interface rdb.
 */
void
arch_callwithureg(void (*fn)(Ureg*))
{
    Ureg ureg;
    ureg.pc = getcallerpc(&fn);
    ureg.sp = (ulong)&fn;
    fn(&ureg);
}
/*e: function [[callwithureg]](x86) */

// was in fns.h before as a macro
// #define userureg(ur) (((ur)->cs & 0xFFFF) == UESEL)
/*s: function [[userureg]](x86) */
//#define userureg(ur) (((ur)->cs & 0xFFFF) == UESEL)
int
arch_userureg(Ureg* ur)
{
  return (((ur)->cs & 0xFFFF) == UESEL);
}
/*e: function [[userureg]](x86) */

/*s: function [[_dumpstack]](x86) */
static void
_dumpstack(Ureg *ureg)
{
    uintptr l, v, i, estack;
    extern ulong etext;
    int x;
    char *s;

    if((s = getconf("*nodumpstack")) != nil && strcmp(s, "0") != 0){
        iprint("dumpstack disabled\n");
        return;
    }
    iprint("dumpstack\n");

    x = 0;
    x += iprint("ktrace /kernel/path %.8lux %.8lux <<EOF\n", ureg->pc, ureg->sp);
    i = 0;
    if(up
    && (uintptr)&l >= (uintptr)up->kstack
    && (uintptr)&l <= (uintptr)up->kstack+KSTACK)
        estack = (uintptr)up->kstack+KSTACK;
    else if((uintptr)(&l) >= (uintptr)(cpu->stack)
    && (uintptr)(&l) <= (uintptr)(cpu+CPUSIZE))
        estack = (uintptr)(cpu+CPUSIZE);
    else
        return;
    x += iprint("estackx %p\n", estack);

    for(l = (uintptr)(&l); l < estack; l += sizeof(uintptr)){
        v = *(uintptr*)l;
        if((KTZERO < v && v < (uintptr)(&etext)) || estack-l < 32){
            /*
             * Could Pick off general CALL (((byte*)v)[-5] == 0xE8)
             * and CALL indirect through AX
             * (((byte*)v)[-2] == 0xFF && ((byte*)v)[-2] == 0xD0),
             * but this is too clever and misses faulting address.
             */
            x += iprint("%.8p=%.8p ", l, v);
            i++;
        }
        if(i == 4){
            i = 0;
            x += iprint("\n");
        }
    }
    if(i)
        iprint("\n");
    iprint("EOF\n");

    if(ureg->trap != VectorNMI)
        return;

    i = 0;
    for(l = (uintptr)(&l); l < estack; l += sizeof(uintptr)){
        iprint("%.8p ", *(uintptr*)l);
        if(++i == 8){
            i = 0;
            iprint("\n");
        }
    }
    if(i)
        iprint("\n");
}
/*e: function [[_dumpstack]](x86) */

/*s: function [[dumpstack]](x86) */
void
trap_dumpstack(void)
{
    arch_callwithureg(_dumpstack);
}
/*e: function [[dumpstack]](x86) */

/*s: function [[debugbpt]](x86) */
static void
debugbpt(Ureg* ureg, void*)
{
    char buf[ERRMAX];

    if(up == nil)
        panic("kernel bpt");
    /* restore pc to instruction that caused the trap */
    ureg->pc--;
    snprint(buf, sizeof buf, "sys: breakpoint");
    postnote(up, 1, buf, NDebug);
}
/*e: function [[debugbpt]](x86) */

/*s: function [[doublefault]](x86) */
static void
doublefault(Ureg*, void*)
{
    panic("double fault");
}
/*e: function [[doublefault]](x86) */

/*s: function [[unexpected]](x86) */
static void
unexpected(Ureg* ureg, void*)
{
    print("unexpected trap %lud; ignoring\n", ureg->trap);
}
/*e: function [[unexpected]](x86) */

/*s: function [[fault386]] */
static void
fault386(Ureg* ureg, void*)
{
    virt_addr addr; 
    bool read, insyscall, user;
    int n; // ret_code
    char buf[ERRMAX];

    addr = getcr2(); // faulting va
    read = !(ureg->ecode & 2);
    user = (ureg->cs & 0xFFFF) == UESEL;

    if(!user){
        if(vmapsync(addr))
            return;
        if(addr >= USTKTOP)
            panic("kernel fault: bad address pc=0x%.8lux addr=0x%.8lux", ureg->pc, addr);
        if(up == nil)
            panic("kernel fault: no user process pc=0x%.8lux addr=0x%.8lux", ureg->pc, addr);
    }
    if(up == nil)
        panic("user fault: up=0 pc=0x%.8lux addr=0x%.8lux", ureg->pc, addr);

    insyscall = up->insyscall;
    up->insyscall = true; // really?

    n = fault(addr, read); // portable code

    if(n < 0){
        if(!user){
            dumpregs(ureg);
            panic("fault: 0x%lux", addr);
        }
        checkpages();
        //checkfault(addr, ureg->pc);
        snprint(buf, sizeof buf, "sys: trap: fault %s addr=0x%lux",
            read ? "read" : "write", addr);
        postnote(up, 1, buf, NDebug);
    }
    up->insyscall = insyscall;
}
/*e: function [[fault386]] */

//*****************************************************************************
// Syscall
//*****************************************************************************

/*
 *  system calls
 */
#include "../port/systab.h"

/*s: function [[syscall]](x86) */
/*
 *  Syscall is called directly from assembler without going through trap().
 */
//@Scheck: not dead, called from assembly by _syscallintr
void syscall(Ureg* ureg)
{
    char *e;
    ulong   sp;
    long    ret;
    int i, s;
    ulong scallnr;
    vlong startns, stopns;

    if((ureg->cs & 0xFFFF) != UESEL) // TODO: use arch_userureg
        panic("syscall: cs 0x%4.4luX", ureg->cs);

    /*s: [[syscall()]] adjust kentry */
    arch_cycles(&up->kentry);
    /*e: [[syscall()]] adjust kentry */

    cpu->syscall++;
    up->insyscall = true;

    up->pc = ureg->pc;
    up->dbgreg = ureg;
    sp = ureg->usp;

    // syscall number!
    scallnr = ureg->ax;

    /*s: [[syscall()]] Proc_tracesyscall if, syscall entry(x86) */
    if(up->procctl == Proc_tracesyscall){
        /*
         * Redundant validaddr.  Do we care?
         * Tracing syscalls is not exactly a fast path...
         * Beware, validaddr currently does a pexit rather
         * than an error if there's a problem; that might
         * change in the future.
         */
        if(sp < (USTKTOP-BY2PG) || sp > (USTKTOP-sizeof(Sargs)-BY2WD))
            validaddr(sp, sizeof(Sargs)+BY2WD, false);

        syscallfmt(scallnr, ureg->pc, (va_list)(sp+BY2WD));
        up->procctl = Proc_stopme;
        // this will call sched() and wakeup the tracer process
        procctl(up); 
        // back here when the tracer process readied us back and
        // should have set procctl back to Proc_tracesyscall
        if(up->syscalltrace)
            free(up->syscalltrace);
        up->syscalltrace = nil;
        startns = todget(nil);
    }
    /*e: [[syscall()]] Proc_tracesyscall if, syscall entry(x86) */

    /*s: [[syscall()]] fp adjustments if fork(x86) */
        if(scallnr == RFORK && up->fpstate == FPactive){
            fpsave(&up->fpsave);
            up->fpstate = FPinactive;
        }
    /*e: [[syscall()]] fp adjustments if fork(x86) */
    arch_spllo();

    up->nerrlab = 0;
    ret = -1;

    if(!waserror()){
        if(scallnr >= nsyscall || systab[scallnr] == nil){
            pprint("bad sys call number %lud pc %lux\n",
                scallnr, ureg->pc);
            postnote(up, 1, "sys: bad sys call", NDebug);
            error(Ebadarg);
        }

        if(sp<(USTKTOP-BY2PG) || sp>(USTKTOP-sizeof(Sargs)-BY2WD)) // adjust Tos ?
            validaddr(sp, sizeof(Sargs)+BY2WD, false);

        // copy syscall arguments from user stack to up->sargs
        up->sargs = *((Sargs*)(sp+BY2WD)); // 1 word for? return pc?
        up->psstate = sysctab[scallnr];

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //IMPORTANT: The actual system call
        ret = systab[scallnr](up->sargs.args);
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        poperror();
    }else{
        /* failure: save the error buffer for errstr */
        e = up->syserrstr;
        up->syserrstr = up->errstr;
        up->errstr = e;
    }

    if(up->nerrlab){
        print("bad errstack [%lud]: %d extra\n", scallnr, up->nerrlab);
        for(i = 0; i < NERR; i++)
            print("sp=%lux pc=%lux\n",
                up->errlab[i].sp, up->errlab[i].pc);
        panic("error stack");
    }

    /*
     *  Put return value in frame.  On the x86 the syscall is
     *  just another trap and the return value from syscall is
     *  ignored.  On other machines the return value is put into
     *  the results register by caller of syscall.
     */
    ureg->ax = ret;

    /*s: [[syscall()]] Proc_tracesyscall if, syscall exit(x86) */
    if(up->procctl == Proc_tracesyscall){
        stopns = todget(nil);
        up->procctl = Proc_stopme;
        sysretfmt(scallnr, (va_list)(sp+BY2WD), ret, startns, stopns);
        s = arch_splhi();
        procctl(up); // again, will call sched() and wakeup tracer process
        arch_splx(s);
        if(up->syscalltrace)
            free(up->syscalltrace);
        up->syscalltrace = nil;
    }
    /*e: [[syscall()]] Proc_tracesyscall if, syscall exit(x86) */

    up->insyscall = false;
    up->psstate = nil;

    /*s: [[syscall()]] call noted() */
    if(scallnr == NOTED)
        arch__noted(ureg, *(ulong*)(sp+BY2WD));
    /*e: [[syscall()]] call noted() */
    /*s: [[syscall()]] call notify()(x86) */
    if(scallnr!=RFORK && (up->procctl || up->nnote)){
        arch_splhi();
        notify(ureg);
    }
    /*e: [[syscall()]] call notify()(x86) */

    /*s: [[syscall()]] if delaysched(x86) */
    /* if we delayed sched because we held a lock, sched now */
    if(up->delaysched)
        sched();
    /*e: [[syscall()]] if delaysched(x86) */
    kexit(ureg);
}
/*e: function [[syscall]](x86) */

/*s: function [[notify]](x86) */
/*
 *  Call user, if necessary, with note.
 *  Pass user the Ureg struct and the note on his stack.
 */
int
notify(Ureg* ureg)
{
    int l;
    ulong s, sp;
    Note *n;

    if(up->procctl)
        procctl(up); // a bit ugly to group procctl handling and note handling
    if(up->nnote == 0)
        return 0;

    /*s: [[notify()]] fp adjustments(x86) */
        if(up->fpstate == FPactive){
            fpsave(&up->fpsave);
            up->fpstate = FPinactive;
        }
        up->fpstate |= FPillegal;
    /*e: [[notify()]] fp adjustments(x86) */

    s = arch_spllo();
    qlock(&up->debug);
    up->notepending = false;
    n = &up->note[0];
    if(strncmp(n->msg, "sys:", 4) == 0){
        l = strlen(n->msg);
        if(l > ERRMAX-15)   /* " pc=0x12345678\0" */
            l = ERRMAX-15;
        seprint(n->msg+l, &n->msg[sizeof n->msg], " pc=0x%.8lux",
            ureg->pc);
    }

    if(n->flag!=NUser && (up->notified || up->notify==0)){
        if(n->flag == NDebug)
            pprint("suicide: %s\n", n->msg);
        qunlock(&up->debug);
        pexit(n->msg, n->flag!=NDebug);
    }

    if(up->notified){
        qunlock(&up->debug);
        arch_splhi();
        return 0;
    }

    if(!up->notify){
        qunlock(&up->debug);
        pexit(n->msg, n->flag!=NDebug);
    }
    sp = ureg->usp;
    sp -= 256;  /* debugging: preserve context causing problem */
    sp -= sizeof(Ureg);

    if(!okaddr((ulong)up->notify, 1, 0)
    || !okaddr(sp-ERRMAX-4*BY2WD, sizeof(Ureg)+ERRMAX+4*BY2WD, 1)){
        qunlock(&up->debug);
        pprint("suicide: bad address in notify\n");
        pexit("Suicide", false);
    }

    memmove((Ureg*)sp, ureg, sizeof(Ureg));
    *(Ureg**)(sp-BY2WD) = up->ureg; /* word under Ureg is old up->ureg */
    up->ureg = (void*)sp;
    sp -= BY2WD+ERRMAX;
    memmove((char*)sp, up->note[0].msg, ERRMAX);
    sp -= 3*BY2WD;
    *(ulong*)(sp+2*BY2WD) = sp+3*BY2WD;     /* arg 2 is string */
    *(ulong*)(sp+1*BY2WD) = (ulong)up->ureg;    /* arg 1 is ureg* */
    *(ulong*)(sp+0*BY2WD) = 0;          /* arg 0 is pc */
    ureg->usp = sp;
    ureg->pc = (ulong)up->notify;
    up->notified = true;
    up->nnote--;
    memmove(&up->lastnote, &up->note[0], sizeof(Note));
    memmove(&up->note[0], &up->note[1], up->nnote*sizeof(Note));

    qunlock(&up->debug);
    arch_splx(s);
    return 1;
}
/*e: function [[notify]](x86) */

/*s: function [[noted]](x86) */
/*
 *   Return user to state before notify()
 */
void
arch__noted(Ureg* ureg, ulong arg0)
{
    Ureg *nureg;
    ulong oureg, sp;

    qlock(&up->debug);
    if(arg0!=NRSTR && !up->notified) {
        qunlock(&up->debug);
        pprint("call to noted() when not notified\n");
        pexit("Suicide", /*freemem*/false);
    }
    up->notified = false;

    nureg = up->ureg;   /* pointer to user returned Ureg struct */

    /*s: [[noted()]] fp adjustments(x86) */
        up->fpstate &= ~FPillegal;
    /*e: [[noted()]] fp adjustments(x86) */

    /* sanity clause */
    oureg = (ulong)nureg;
    if(!okaddr((ulong)oureg-BY2WD, BY2WD+sizeof(Ureg), 0)){
        qunlock(&up->debug);
        pprint("bad ureg in noted or call to noted when not notified\n");
        pexit("Suicide", false);
    }

    /*
     * Check the segment selectors are all valid, otherwise
     * a fault will be taken on attempting to return to the
     * user process.
     * Take care with the comparisons as different processor
     * generations push segment descriptors in different ways.
     */
    if((nureg->cs & 0xFFFF) != UESEL || (nureg->ss & 0xFFFF) != UDSEL
      || (nureg->ds & 0xFFFF) != UDSEL || (nureg->es & 0xFFFF) != UDSEL
      || (nureg->fs & 0xFFFF) != UDSEL || (nureg->gs & 0xFFFF) != UDSEL){
        qunlock(&up->debug);
        pprint("bad segment selector in noted\n");
        pexit("Suicide", false);
    }

    /* don't let user change system flags */
    nureg->flags = (ureg->flags & ~0xCD5) | (nureg->flags & 0xCD5);

    memmove(ureg, nureg, sizeof(Ureg));

    switch(arg0){
    case NCONT:
    case NRSTR:
        if(!okaddr(nureg->pc, 1, 0) || !okaddr(nureg->usp, BY2WD, 0)){
            qunlock(&up->debug);
            pprint("suicide: trap in noted\n");
            pexit("Suicide", false);
        }
        up->ureg = (Ureg*)(*(ulong*)(oureg-BY2WD));
        qunlock(&up->debug);
        break;

    case NSAVE:
        if(!okaddr(nureg->pc, BY2WD, 0)
        || !okaddr(nureg->usp, BY2WD, 0)){
            qunlock(&up->debug);
            pprint("suicide: trap in noted\n");
            pexit("Suicide", false);
        }
        qunlock(&up->debug);
        sp = oureg-4*BY2WD-ERRMAX;
        arch_splhi();
        ureg->sp = sp;
        ((ulong*)sp)[1] = oureg;    /* arg 1 0(FP) is ureg* */
        ((ulong*)sp)[0] = 0;        /* arg 0 is pc */
        break;

    default:
        pprint("unknown noted arg 0x%lux\n", arg0);
        up->lastnote.flag = NDebug;
        /* fall through */

    case NDFLT:
        if(up->lastnote.flag == NDebug){
            qunlock(&up->debug);
            pprint("suicide: %s\n", up->lastnote.msg);
        } else
            qunlock(&up->debug);
        pexit(up->lastnote.msg, up->lastnote.flag!=NDebug);
    }
}
/*e: function [[noted]](x86) */
//old: #define evenaddr(x)       /* x86 doesn't care */
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


/*s: function [[execregs]](x86) */
long
arch_execregs(ulong entry, ulong ssize, ulong nargs)
{
    ulong *sp;
    Ureg *ureg;

    /*s: [[execregs()]] fp adjustments(x86) */
        //arch_procsetup(up), redundant?
        up->fpstate = FPinit;
        fpoff();
    /*e: [[execregs()]] fp adjustments(x86) */

    sp = (ulong*)(USTKTOP - ssize);
    *--sp = nargs;

    ureg = up->dbgreg;
    ureg->usp = (ulong)sp;
    ureg->pc = entry;
    return USTKTOP
     /*s: [[execregs()]] return adjustments(x86) */
           -sizeof(Tos)
     /*e: [[execregs()]] return adjustments(x86) */
     ;     /* address of kernel/user shared data */
}
/*e: function [[execregs]](x86) */

/*s: function [[userpc]](x86) */
/*
 *  return the userpc the last exception happened at
 */
ulong
arch_userpc(void)
{
    Ureg *ureg;

    ureg = (Ureg*)up->dbgreg;
    return ureg->pc;
}
/*e: function [[userpc]](x86) */

/*s: function [[setregisters]](x86) */
/* This routine must save the values of registers the user is not permitted
 * to write from devproc and then restore the saved values before returning.
 */
void
arch_setregisters(Ureg* ureg, char* pureg, char* uva, int n)
{
    ulong cs, ds, es, flags, fs, gs, ss;

    ss = ureg->ss;
    flags = ureg->flags;
    cs = ureg->cs;
    ds = ureg->ds;
    es = ureg->es;
    fs = ureg->fs;
    gs = ureg->gs;
    memmove(pureg, uva, n);
    ureg->gs = gs;
    ureg->fs = fs;
    ureg->es = es;
    ureg->ds = ds;
    ureg->cs = cs;
    ureg->flags = (ureg->flags & 0x00FF) | (flags & 0xFF00);
    ureg->ss = ss;
}
/*e: function [[setregisters]](x86) */

/*s: function [[linkproc]](x86) */
static void
linkproc(void)
{
    arch_spllo();
    up->kpfun(up->kparg);
    // should never reach this place?? kernel processes are supposed
    // to run forever??
    pexit("kproc dying", /*freemem*/false); 
}
/*e: function [[linkproc]](x86) */

/*s: function [[kprocchild]](x86) */
void
arch_kprocchild(Proc* p, void (*func)(void*), void* arg)
{
    p->kpfun = func;
    p->kparg = arg;

    /*
     * arch_gotolabel() needs a word on the stack in
     * which to place the return PC used to jump
     * to linkproc().
     */
    p->sched.pc = (ulong)linkproc;
    p->sched.sp = (ulong)p->kstack+KSTACK-BY2WD;
}
/*e: function [[kprocchild]](x86) */

/*s: function [[forkchild]](x86) */
void
arch_forkchild(Proc *p, Ureg *ureg)
{
    Ureg *cureg;

    /*
     * Add 2*BY2WD to the stack to account for
     *  - the return PC
     *  - trap's argument (ur)
     */
    p->sched.sp = (ulong)p->kstack+KSTACK-(sizeof(Ureg)+2*BY2WD);
    p->sched.pc = (ulong)arch__forkret;

    cureg = (Ureg*)(p->sched.sp+2*BY2WD);
    memmove(cureg, ureg, sizeof(Ureg));
    /* return value of syscall in child */
    cureg->ax = 0;

    /* Things from bottom of syscall which were never executed */
    p->psstate = nil;
    p->insyscall = false;
}
/*e: function [[forkchild]](x86) */

/*s: function [[setkernur]](x86) */
/* Give enough context in the ureg to produce a kernel stack for
 * a sleeping process
 */
void
arch_setkernur(Ureg* ureg, Proc* p)
{
    ureg->pc = p->sched.pc;
    ureg->sp = p->sched.sp+4;
}
/*e: function [[setkernur]](x86) */

/*s: function [[dbgpc]](x86) */
ulong
arch_dbgpc(Proc *p)
{
    Ureg *ureg;

    ureg = p->dbgreg;
    if(ureg == nil)
        return nilptr;

    return ureg->pc;
}
/*e: function [[dbgpc]](x86) */
/*e: interrupts/386/trap.c */
