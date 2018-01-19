/*s: interrupts/arm/trap.c */
/*
 * traps, exceptions, interrupts, system calls.
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "io.h"
#include "ureg.h"
#include "arm.h"

/*s: constant [[INTREGS]](arm) */
#define INTREGS     (VIRTIO+0xB200)
/*e: constant [[INTREGS]](arm) */
/*s: constant [[LOCALREGS]](arm) */
#define LOCALREGS   (VIRTIO+IOSIZE)
/*e: constant [[LOCALREGS]](arm) */

typedef struct Intregs Intregs;
typedef struct Vctl Vctl;
typedef struct Vpage0 Vpage0;

/*s: enum [[_anon_]]([[(interrupts/arm/trap.c)(arm)]]) */
enum {
    Debug = 0,

    /*s: constant [[Nvec]](arm) */
    Nvec = 8,       /* # of vectors at start of lexception.s */
    /*e: constant [[Nvec]](arm) */
    /*s: constant [[Fiqenable]](arm) */
    Fiqenable = 1<<7,
    /*e: constant [[Fiqenable]](arm) */

    /*s: constant [[Localtimerint]](arm) */
    Localtimerint   = 0x40,
    /*e: constant [[Localtimerint]](arm) */
    /*s: constant [[Localmboxint]](arm) */
    Localmboxint    = 0x50,
    /*e: constant [[Localmboxint]](arm) */
    /*s: constant [[Localintpending]](arm) */
    Localintpending = 0x60,
    /*e: constant [[Localintpending]](arm) */
};
/*e: enum [[_anon_]]([[(interrupts/arm/trap.c)(arm)]]) */

/*s: struct [[Vpage0]](arm) */
/*
 *   Layout at virtual address KZERO (double mapped at HVECTORS).
 */
struct Vpage0 {
    void    (*vectors[Nvec])(void);
    u32int  vtable[Nvec];
};
/*e: struct [[Vpage0]](arm) */

/*s: struct [[Intregs]](arm) */
/*
 * interrupt control registers
 */
// The order matters! the fields match the memory-mapped external registers.
struct Intregs {
    u32int  ARMpending;
    u32int  GPUpending[2];

    u32int  FIQctl;

    u32int  GPUenable[2];
    u32int  ARMenable;

    u32int  GPUdisable[2];
    u32int  ARMdisable;
};
/*e: struct [[Intregs]](arm) */

/*s: struct [[Vctl]](arm) */
struct Vctl {
    // enum<IRQ>
    int irq;

    u32int  *reg; // VIRTIO address
    u32int  mask;

    void    (*f)(Ureg*, void*); /* handler to call */
    void    *a; /* argument to call it with */

    /*s: [[Vctl]] other fields(arm) */
    int cpu;
    /*e: [[Vctl]] other fields(arm) */

    // Extra
    /*s: [[Vctl]] extra fields(arm) */
    Vctl    *next;
    /*e: [[Vctl]] extra fields(arm) */
};
/*e: struct [[Vctl]](arm) */

/*s: global [[vctllock]](arm) */
static Lock vctllock;
/*e: global [[vctllock]](arm) */

/*s: global [[vctl]](arm) */
// list<ref_own<Vctl>>> (next = Vctl.next)
static Vctl *vctl;
/*e: global [[vctl]](arm) */

/*s: global [[vfiq]](arm) */
static Vctl *vfiq;
/*e: global [[vfiq]](arm) */

/*s: global [[trapnames]](arm) */
static char *trapnames[PsrMask+1] = {
    [ PsrMusr ] "user mode",
    [ PsrMsvc ] "svc/swi exception",
    [ PsrMirq ] "irq interrupt",
    [ PsrMabt ] "prefetch abort/data abort",
    [ PsrMabt+1 ] "data abort",
    [ PsrMund ] "undefined instruction",
    /*s: [[trapnames()]] other entries(arm) */
    [ PsrMfiq ] "fiq interrupt",
    /*e: [[trapnames()]] other entries(arm) */
};
/*e: global [[trapnames]](arm) */

/*s: function [[arch__trapinit]](arm) */
/*
 *  set up for exceptions
 */
void
arch__trapinit(void)
{
    Vpage0 *vpage0;

    if (cpu->cpuno == 0) {
        /* disable everything */
        intrsoff();
        /* set up the exception vectors */
        vpage0 = (Vpage0*)HVECTORS;
        memmove(vpage0->vectors, vectors, sizeof(vpage0->vectors));
        memmove(vpage0->vtable, vtable, sizeof(vpage0->vtable));
        /*s: [[arch__trapinit()]] invalidate cache after adjust vectors(arm) */
        cacheuwbinv();
        l2cacheuwbinv();
        /*e: [[arch__trapinit()]] invalidate cache after adjust vectors(arm) */
    }

    /* set up the stacks for the interrupt modes */
    /*s: [[arch__trapinit()]] set stack for other exception/processor-modes */
    setr13(PsrMirq, cpu->sirq);
    setr13(PsrMabt, cpu->sabt);
    setr13(PsrMund, cpu->sund);
    /*x: [[arch__trapinit()]] set stack for other exception/processor-modes */
    setr13(PsrMfiq, (u32int*)(FIQSTKTOP));
    /*e: [[arch__trapinit()]] set stack for other exception/processor-modes */
    /*s: [[arch__trapinit()]] coherence(arm) */
    arch_coherence();
    /*e: [[arch__trapinit()]] coherence(arm) */
}
/*e: function [[arch__trapinit]](arm) */

/*s: function [[intrcpushutdown]](arm) */
void
intrcpushutdown(void)
{
    u32int *enable;

    if(soc.armlocal == 0)
        return;
    enable = (u32int*)(LOCALREGS + Localtimerint) + cpu->cpuno;
    *enable = 0;
    if(cpu->cpuno){
        enable = (u32int*)(LOCALREGS + Localmboxint) + cpu->cpuno;
        *enable = 1;
    }
}
/*e: function [[intrcpushutdown]](arm) */

/*s: function [[intrsoff]](arm) */
void
intrsoff(void)
{
    Intregs *ip;
    int disable;

    ip = (Intregs*)INTREGS;
    disable = ~0;
    ip->GPUdisable[0] = disable;
    ip->GPUdisable[1] = disable;
    ip->ARMdisable = disable;
    ip->FIQctl = 0;
}
/*e: function [[intrsoff]](arm) */

/*s: function [[intrshutdown]](arm) */
/* called from cpu0 after other cpus are shutdown */
void
intrshutdown(void)
{
    intrsoff();
    intrcpushutdown();
}
/*e: function [[intrshutdown]](arm) */

/*s: function [[intrtime]](arm) */
static void
intrtime(void)
{
    ulong diff;
    ulong x;

    x = arch_perfticks();
    diff = x - cpu->perf.intrts;
    cpu->perf.intrts = x;

    cpu->perf.inintr += diff;
    if(up == nil && cpu->perf.inidle > diff)
        cpu->perf.inidle -= diff;
}
/*e: function [[intrtime]](arm) */


/*s: function [[irq]](arm) */
/*
 *  called by trap to handle irq interrupts.
 *  returns true iff a clock interrupt, thus maybe reschedule.
 */
static bool
irq(Ureg* ureg)
{
    Vctl *v;
    bool clockintr;
    bool found;

    cpu->perf.intrts = arch_perfticks();
    clockintr = false;
    found = false;
    for(v = vctl; v; v = v->next)
        if(v->cpu == cpu->cpuno && (*v->reg & v->mask) != 0){
            found = true;
            /*s: [[irq()]] before dispatch(arm) */
            arch_coherence();
            /*e: [[irq()]] before dispatch(arm) */
            // Dispatch
            v->f(ureg, v->a);
            /*s: [[irq()]] after dispatch(arm) */
            arch_coherence();
            /*e: [[irq()]] after dispatch(arm) */
            if(v->irq == IRQclock || v->irq == IRQcntps || v->irq == IRQcntpns)
                clockintr = true;
        }
    if(!found)
        cpu->spuriousintr++;
    intrtime();
    return clockintr;
}
/*e: function [[irq]](arm) */

/*s: function [[fiq]](arm) */
/*
 * called direct from lexception.s to handle fiq interrupt.
 */
void
fiq(Ureg *ureg)
{
    Vctl *v;

    cpu->perf.intrts = arch_perfticks();
    v = vfiq;
    if(v == nil)
        panic("cpu%d: unexpected item in bagging area", cpu->cpuno);
    cpu->intr++;
    ureg->pc -= 4;
    arch_coherence();

    // Dispatch!
    v->f(ureg, v->a);

    arch_coherence();
    intrtime();
}
/*e: function [[fiq]](arm) */

/*s: function [[irqenable]](arm) */
void
irqenable(int irq, void (*f)(Ureg*, void*), void* a)
{
    Vctl *v;
    Intregs *ip;
    u32int *enable;

    ip = (Intregs*)INTREGS;
    v = (Vctl*)malloc(sizeof(Vctl));
    /*s: [[irqenable()]] sanity check v(arm) */
    if(v == nil)
        panic("irqenable: no mem");
    /*e: [[irqenable()]] sanity check v(arm) */
    v->irq = irq;
    v->cpu = 0;
    /*s: [[irqenable()]] if IRQlocal(arm) */
    if(irq >= IRQlocal){
        enable = (u32int*)(LOCALREGS + Localtimerint) + cpu->cpuno;
        v->reg = (u32int*)(LOCALREGS + Localintpending) + cpu->cpuno;
        v->mask = 1 << (irq - IRQlocal);
        v->cpu = cpu->cpuno;
    }
    /*e: [[irqenable()]] if IRQlocal(arm) */
    /*s: [[irqenable()]] if IRQbasic(arm) */
    else if(irq >= IRQbasic){
        enable = &ip->ARMenable;
        v->reg = &ip->ARMpending;
        v->mask = 1 << (irq - IRQbasic);
    }
    /*e: [[irqenable()]] if IRQbasic(arm) */
    else{
        enable = &ip->GPUenable[irq/32];
        v->reg = &ip->GPUpending[irq/32];
        v->mask = 1 << (irq % 32);
    }
    v->f = f;
    v->a = a;
    lock(&vctllock);
    /*s: [[irqenable()]] if IRQfiq(arm) */
    if(irq == IRQfiq){
        /*s: [[irqenable()]] sanity check no previous FIQ(arm) */
        assert((ip->FIQctl & Fiqenable) == 0);
        assert((*enable & v->mask) == 0);
        /*e: [[irqenable()]] sanity check no previous FIQ(arm) */
        vfiq = v;
        ip->FIQctl = Fiqenable | irq;
    }
    /*e: [[irqenable()]] if IRQfiq(arm) */
    else{
        v->next = vctl;
        vctl = v;
        /*s: [[irqenable()]] if IRQlocal adjust enable(arm) */
        if(irq >= IRQlocal)
            *enable |= 1 << (irq - IRQlocal);
        /*e: [[irqenable()]] if IRQlocal adjust enable(arm) */
        else
            *enable = v->mask;
    }
    unlock(&vctllock);
}
/*e: function [[irqenable]](arm) */

/*s: function [[trapname]](arm) */
static char *
trapname(int psr)
{
    char *s;

    s = trapnames[psr & PsrMask];
    if(s == nil)
        s = "unknown trap number in psr";
    return s;
}
/*e: function [[trapname]](arm) */

/*s: function [[ckfaultstuck]](arm) */
/* this is quite helpful during mmu and cache debugging */
static void
ckfaultstuck(uintptr va)
{
    static int cnt, lastpid;
    static uintptr lastva;

    if (va == lastva && up->pid == lastpid) {
        ++cnt;
        if (cnt >= 2)
            /* fault() isn't fixing the underlying cause */
            panic("fault: %d consecutive faults for va %#p",
                cnt+1, va);
    } else {
        cnt = 0;
        lastva = va;
        lastpid = up->pid;
    }
}
/*e: function [[ckfaultstuck]](arm) */

/*s: function [[faultarm]](arm) */
/*
 *  called by trap to handle access faults
 */
static void
faultarm(Ureg *ureg, virt_addr va, bool user, bool read)
{
    bool insyscall;
    int n;
    char buf[ERRMAX];

    if(up == nil) {
        //dumpregs(ureg);
        panic("fault: nil up in faultarm, pc %#p accessing %#p", ureg->pc, va);
    }
    insyscall = up->insyscall;
    up->insyscall = true; // ???
    /*s: [[faultarm()]] if debug */
    if (Debug)
        ckfaultstuck(va);
    /*e: [[faultarm()]] if debug */
    n = fault(va, read); // portable code

    if(n < 0){
        if(!user){
            dumpregs(ureg);
            panic("fault: kernel accessing %#p", va);
        }
        /* don't dump registers; programs suicide all the time */
        snprint(buf, sizeof buf, "sys: trap: fault %s va=%#p",
            read? "read": "write", va);
        postnote(up, 1, buf, NDebug);
    }
    up->insyscall = insyscall;
}
/*e: function [[faultarm]](arm) */

/*s: function [[writetomem]](arm) */
/*
 *  returns true if the instruction writes memory, false otherwise
 */
bool
writetomem(ulong inst)
{
    /* swap always write memory */
    if((inst & 0x0FC00000) == 0x01000000)
        return true;

    /* loads and stores are distinguished by bit 20 */
    if(inst & (1<<20))
        return false;

    return true;
}
/*e: function [[writetomem]](arm) */

/*s: function [[arch__trap]](arm) */
/*
 *  here on all exceptions other than syscall (SWI) and fiq
 */
void
arch__trap(Ureg *ureg)
{
    bool user;
    bool clockintr;
    /*s: [[trap()]] other locals(arm) */
    int rem;
    /*x: [[trap()]] other locals(arm) */
    char buf[ERRMAX];
    /*x: [[trap()]] other locals(arm) */
    int x;
    ulong fsr;
    /*x: [[trap()]] other locals(arm) */
    virt_addr va; // can be a kern_addr?
    ulong inst;
    /*x: [[trap()]] other locals(arm) */
    int rv;
    /*e: [[trap()]] other locals(arm) */

    /*s: [[trap()]] sanity check interrupts are disabled(arm) */
    assert(!arch_islo());
    /*e: [[trap()]] sanity check interrupts are disabled(arm) */
    /*s: [[trap()]] sanity check enough space in kernel stack(arm) */
    if(up != nil)
        rem = ((char*)ureg)-up->kstack;
    else
        rem = ((char*)ureg)-((char*)cpu + sizeof(Cpu));
    if(rem < 256) {
        iprint("trap: %d stack bytes left, up %#p ureg %#p at pc %#lux\n",
            rem, up, ureg, ureg->pc);
        arch_delay(1000);
        arch_dumpstack();
        panic("trap: %d stack bytes left, up %#p ureg %#p at pc %#lux",
            rem, up, ureg, ureg->pc);
    }
    /*e: [[trap()]] sanity check enough space in kernel stack(arm) */

    user = (ureg->psr & PsrMask) == PsrMusr; // TODO: arch_userureg
    if(user){
        up->dbgreg = ureg;
        /*s: [[trap()]] adjust kentry when interrupt user */
        arch_cycles(&up->kentry);
        /*e: [[trap()]] adjust kentry when interrupt user */
    }

    /*
     * All interrupts/exceptions should be resumed at ureg->pc-4,
     * except for Data Abort which resumes at ureg->pc-8.
     */
    if(ureg->type == (PsrMabt+1))
        ureg->pc -= 8;
    else
       ureg->pc -= 4;

    clockintr = false;      /* if set, may call sched() before return */

    switch(ureg->type){
    /*s: [[trap()]] switch exception type cases(arm) */
    case PsrMund:           /* undefined instruction */
        if(user){
            /*s: [[trap()]] when undefined instruction, if breakpoint(arm) */
            if(seg(up, ureg->pc, false) != nil &&
               *(u32int*)ureg->pc == 0xD1200070)
                postnote(up, 1, "sys: breakpoint", NDebug);
            /*e: [[trap()]] when undefined instruction, if breakpoint(arm) */
            else{
                /*s: [[trap()]] when undefined instruction, if float instr(arm) */
                /* look for floating point instructions to interpret */
                rv = fpuemu(ureg);
                /*e: [[trap()]] when undefined instruction, if float instr(arm) */
                if(rv == 0){
                    snprint(buf, sizeof buf,
                        "undefined instruction: pc %#lux\n",
                        ureg->pc);
                    postnote(up, 1, buf, NDebug);
                }
            }
        }else{
            /*s: [[trap()]] when undefined instruction, round pc(arm) */
            if (ureg->pc & 3) {
                iprint("rounding fault pc %#lux down to word\n",
                    ureg->pc);
                ureg->pc &= ~3;
            }
            /*e: [[trap()]] when undefined instruction, round pc(arm) */
            iprint("undefined instruction: pc %#lux inst %#ux\n",
                ureg->pc, *(u32int*)ureg->pc);
            panic("undefined instruction");
        }
        break;
    /*x: [[trap()]] switch exception type cases(arm) */
    case PsrMabt:           /* prefetch fault */
        x = ifsrget();
        fsr = (x>>7) & 0x8 | x & 0x7;
        switch(fsr){
        /*s: [[trap()]] when prefetch fault, switch fst cases(arm) */
        case 0x02:      /* instruction debug event (BKPT) */
            if(user){
                snprint(buf, sizeof buf, "sys: breakpoint");
                postnote(up, 1, buf, NDebug);
            }else{
                iprint("kernel bkpt: pc %#lux inst %#ux\n",
                    ureg->pc, *(u32int*)ureg->pc);
                panic("kernel bkpt");
            }
            break;
        /*e: [[trap()]] when prefetch fault, switch fst cases(arm) */
        default:
            // Page fault on code!!
            faultarm(ureg, ureg->pc, user, true);
            break;
        }
        break;
    /*x: [[trap()]] switch exception type cases(arm) */
    case PsrMabt+1:         /* data fault */
        va = farget();
        inst = *(ulong*)(ureg->pc);
        /* bits 12 and 10 have to be concatenated with status */
        x = fsrget();
        fsr = (x>>7) & 0x20 | (x>>6) & 0x10 | x & 0xf;
        switch(fsr){
        /*s: [[trap()]] when data fault, switch fst cases(arm) */
        case 0xd:
        case 0xf:
            /* permission error, copy on write or real permission error */
            faultarm(ureg, va, user, !writetomem(inst));
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x5:       /* translation fault, no section entry */
        case 0x7:       /* translation fault, no page entry */
            // Page fault on data!!
            faultarm(ureg, va, user, !writetomem(inst));
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x9:
        case 0xb:
            /* domain fault, accessing something we shouldn't */
            if(user){
                snprint(buf, sizeof buf,
                    "sys: access violation: pc %#lux va %#p\n",
                    ureg->pc, va);
                postnote(up, 1, buf, NDebug);
            } else
                panic("kernel access violation: pc %#lux va %#p",
                    ureg->pc, va);
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x0:
            panic("vector exception at %#lux", ureg->pc);
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x1:       /* alignment fault */
        case 0x3:       /* access flag fault (section) */
            if(user){
                snprint(buf, sizeof buf,
                    "sys: alignment: pc %#lux va %#p\n",
                    ureg->pc, va);
                postnote(up, 1, buf, NDebug);
            } else
                panic("kernel alignment: pc %#lux va %#p", ureg->pc, va);
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x2:
            panic("terminal exception at %#lux", ureg->pc);
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x4:       /* icache maint fault */
        case 0x6:       /* access flag fault (page) */
        case 0x8:       /* precise external abort, non-xlat'n */
        case 0x28:
        case 0xc:       /* l1 translation, precise ext. abort */
        case 0x2c:
        case 0xe:       /* l2 translation, precise ext. abort */
        case 0x2e:
        case 0x16:      /* imprecise ext. abort, non-xlt'n */
        case 0x36:
            panic("external abort %#lux pc %#lux addr %#p",
                fsr, ureg->pc, va);
            break;
        /*x: [[trap()]] when data fault, switch fst cases(arm) */
        case 0x1c:      /* l1 translation, precise parity err */
        case 0x1e:      /* l2 translation, precise parity err */
        case 0x18:      /* imprecise parity or ecc err */
            panic("translation parity error %#lux pc %#lux addr %#p",
                fsr, ureg->pc, va);
            break;
        /*e: [[trap()]] when data fault, switch fst cases(arm) */
        default:
        case 0xa:       /* ? was under external abort */
            panic("unknown data fault, 6b fsr %#lux", fsr);
            break;
        }
        break;
    /*x: [[trap()]] switch exception type cases(arm) */
    case PsrMirq:
        clockintr = irq(ureg);
        cpu->intr++;
        break;
    /*e: [[trap()]] switch exception type cases(arm) */
    default:
        panic("unknown trap; type %#lux, psr mode %#lux pc %lux", ureg->type,
            ureg->psr & PsrMask, ureg->pc);
        break;
    }
    arch_splhi();

    /*s: [[trap()]] if delaysched and clockintr */
    /* delaysched set because we held a lock or because our quantum ended */
    if(up && up->delaysched && clockintr){
        sched();
        arch_splhi();
    }
    /*e: [[trap()]] if delaysched and clockintr */

    if(user){
        /*s: [[trap()]] call possibly notify(arm) */
        if(up->procctl || up->nnote)
            arch__notify(ureg);
        /*e: [[trap()]] call possibly notify(arm) */
        arch__kexit(ureg);
    }
}
/*e: function [[arch__trap]](arm) */

/*s: function [[isvalidaddr]](arm) */
int
isvalidaddr(void *v)
{
    return (uintptr)v >= KZERO;
}
/*e: function [[isvalidaddr]](arm) */

/*s: function [[dumplongs]](arm) */
static void
dumplongs(char *msg, ulong *v, int n)
{
    int i, l;

    l = 0;
    iprint("%s at %.8p: ", msg, v);
    for(i=0; i<n; i++){
        if(l >= 4){
            iprint("\n    %.8p: ", v);
            l = 0;
        }
        if(isvalidaddr(v)){
            iprint(" %.8lux", *v++);
            l++;
        }else{
            iprint(" invalid");
            break;
        }
    }
    iprint("\n");
}
/*e: function [[dumplongs]](arm) */

/*s: function [[dumpstackwithureg]](arm) */
static void
dumpstackwithureg(Ureg *ureg)
{
    uintptr l, i, v, estack;
    u32int *p;
    char *s;

    if((s = getconf("*nodumpstack")) != nil && strcmp(s, "0") != 0){
        iprint("dumpstack disabled\n");
        return;
    }
    iprint("ktrace /kernel/path %#.8lux %#.8lux %#.8lux # pc, sp, link\n",
        ureg->pc, ureg->sp, ureg->r14);
    arch_delay(2000);
    i = 0;
    if(up != nil && (uintptr)&l <= (uintptr)up->kstack+KSTACK)
        estack = (uintptr)up->kstack+KSTACK;
    else if((uintptr)&l >= (uintptr)cpu->stack
         && (uintptr)&l <= (uintptr)cpu + CPUSIZE)
        estack = (uintptr)cpu + CPUSIZE;
    else{
        if(up != nil)
            iprint("&up->kstack %#p &l %#p\n", up->kstack, &l);
        else
            iprint("&m %#p &l %#p\n", cpu, &l);
        return;
    }
    for(l = (uintptr)&l; l < estack; l += sizeof(uintptr)){
        v = *(uintptr*)l;
        if(KTZERO < v && v < (uintptr)etext && !(v & 3)){
            v -= sizeof(u32int);        /* back up an instr */
            p = (u32int*)v;
            if((*p & 0x0f000000) == 0x0b000000){    /* BL instr? */
                iprint("%#8.8lux=%#8.8lux ", l, v);
                i++;
            }
        }
        if(i == 4){
            i = 0;
            iprint("\n");
        }
    }
    if(i)
        iprint("\n");
}
/*e: function [[dumpstackwithureg]](arm) */

/*s: function [[getpcsp]](arm) */
/*
 * Fill in enough of Ureg to get a stack trace, and call a function.
 * Used by debugging interface rdb.
 */
static void
getpcsp(ulong *pc, ulong *sp)
{
    *pc = getcallerpc(&pc);
    *sp = (ulong)&pc-4;
}
/*e: function [[getpcsp]](arm) */

/*s: function [[arch_callwithureg]](arm) */
void
arch_callwithureg(void (*fn)(Ureg*))
{
    Ureg ureg;

    getpcsp((ulong*)&ureg.pc, (ulong*)&ureg.sp);
    ureg.r14 = getcallerpc(&fn);
    fn(&ureg);
}
/*e: function [[arch_callwithureg]](arm) */

/*s: function [[trap_arch_dumpstack]](arm) */
void
trap_arch_dumpstack(void)
{
    arch_callwithureg(dumpstackwithureg);
}
/*e: function [[trap_arch_dumpstack]](arm) */

/*s: function [[dumpregs]](arm) */
void
dumpregs(Ureg* ureg)
{
    int s;

    if (ureg == nil) {
        iprint("trap: no user process\n");
        return;
    }
    s = arch_splhi();
    iprint("trap: %s", trapname(ureg->type));
    if(ureg != nil && (ureg->psr & PsrMask) != PsrMsvc)
        iprint(" in %s", trapname(ureg->psr));
    iprint("\n");
    iprint("psr %8.8lux type %2.2lux pc %8.8lux link %8.8lux\n",
        ureg->psr, ureg->type, ureg->pc, ureg->link);
    iprint("R14 %8.8lux R13 %8.8lux R12 %8.8lux R11 %8.8lux R10 %8.8lux\n",
        ureg->r14, ureg->r13, ureg->r12, ureg->r11, ureg->r10);
    iprint("R9  %8.8lux R8  %8.8lux R7  %8.8lux R6  %8.8lux R5  %8.8lux\n",
        ureg->r9, ureg->r8, ureg->r7, ureg->r6, ureg->r5);
    iprint("R4  %8.8lux R3  %8.8lux R2  %8.8lux R1  %8.8lux R0  %8.8lux\n",
        ureg->r4, ureg->r3, ureg->r2, ureg->r1, ureg->r0);
    iprint("stack is at %#p\n", ureg);
    iprint("pc %#lux link %#lux\n", ureg->pc, ureg->link);

    if(up)
        iprint("user stack: %#p-%#p\n", up->kstack, up->kstack+KSTACK-4);
    else
        iprint("kernel stack: %8.8lux-%8.8lux\n",
            (ulong)(cpu+1), (ulong)cpu+BY2PG-4);
    dumplongs("stack", (ulong *)(ureg + 1), 16);
    arch_delay(2000);
    arch_dumpstack();
    arch_splx(s);
}
/*e: function [[dumpregs]](arm) */
/*e: interrupts/arm/trap.c */
