/*s: time/arm/clock.c */
/*
 * bcm283[56] timers
 *  System timers run at 1MHz (timers 1 and 2 are used by GPU)
 *  ARM timer usually runs at 250MHz (may be slower in low power modes)
 *  Cycle counter runs at 700MHz (unless overclocked)
 *    All are free-running up-counters
 *  Cortex-a7 has local generic timers per cpu (which we run at 1MHz)
 *
 * Use system timer 3 (64 bits) for hzclock interrupts and fastticks
 *   For smp on bcm2836, use local generic timer for interrupts on cpu1-3
 * Use ARM timer (32 bits) for perfticks
 * Use ARM timer to force immediate interrupt
 * Use cycle counter for cycles()
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

/*s: constant SYSTIMERS(arm) */
#define SYSTIMERS (VIRTIO+0x3000)
/*e: constant SYSTIMERS(arm) */
/*s: constant ARMTIMER(arm) */
#define ARMTIMER (VIRTIO+0xB400)
/*e: constant ARMTIMER(arm) */
//TODO factorize in mem.h
/*s: constant ARMLOCAL(arm) */
#define ARMLOCAL    (VIRTIO+IOSIZE)
/*e: constant ARMLOCAL(arm) */

/*s: enum _anon_ (time/arm/clock.c)(arm) */
enum {
    /*s: constant Localctl(arm) */
    Localctl    = 0x00,
    /*e: constant Localctl(arm) */
    /*s: constant Prescaler(arm) */
    Prescaler   = 0x08,
    /*e: constant Prescaler(arm) */

    /*s: constant SystimerFreq(arm) */
    SystimerFreq    = 1*Mhz,
    /*e: constant SystimerFreq(arm) */

    /*s: constant MaxPeriod(arm) */
    MaxPeriod   = SystimerFreq / Arch_HZ,
    /*e: constant MaxPeriod(arm) */
    /*s: constant MinPeriod(arm) */
    MinPeriod   = SystimerFreq / (100*Arch_HZ),
    /*e: constant MinPeriod(arm) */
};
/*e: enum _anon_ (time/arm/clock.c)(arm) */

typedef struct Systimers Systimers;
typedef struct Armtimer Armtimer;

/*s: struct Systimers(arm) */
// The order matters! the fields match the memory-mapped external registers.
struct Systimers {
    u32int  cs;

    u32int  clo;
    u32int  chi;

    u32int  c0;
    u32int  c1;
    u32int  c2;
    u32int  c3;
};
/*e: struct Systimers(arm) */

/*s: struct Armtimer(arm) */
struct Armtimer {
    u32int  load;
    u32int  val;
    u32int  ctl;
    u32int  irqack;
    u32int  irq;
    u32int  maskedirq;
    u32int  reload;
    u32int  predivider;
    u32int  count;
};
/*e: struct Armtimer(arm) */

/*s: enum _anon_ (time/arm/clock.c)2(arm) */
enum {
    CntPrescaleShift= 16,   /* freq is sys_clk/(prescale+1) */
    CntPrescaleMask = 0xFF,
    CntEnable   = 1<<9,
    TmrDbgHalt  = 1<<8,
    TmrEnable   = 1<<7,
    TmrIntEnable    = 1<<5,
    TmrPrescale1    = 0x00<<2,
    TmrPrescale16   = 0x01<<2,
    TmrPrescale256  = 0x02<<2,
    CntWidth16  = 0<<1,
    CntWidth32  = 1<<1,

    /* generic timer (cortex-a7) */
    Enable  = 1<<0,
    Imask   = 1<<1,
    Istatus = 1<<2,
};
/*e: enum _anon_ (time/arm/clock.c)2(arm) */

/*s: function clockintr(arm) */
static void
clockintr(Ureg *ureg, void*)
{
    Systimers *tn;

    tn = (Systimers*)SYSTIMERS;
    /* dismiss interrupt */
    tn->cs = 1<<3;
    timerintr(ureg, 0);
}
/*e: function clockintr(arm) */

/*s: function localclockintr(arm) */
static void
localclockintr(Ureg *ureg, void *)
{
    if(cpu->cpuno == 0)
        panic("cpu0: Unexpected local generic timer interrupt");
    cpwrsc(0, CpTIMER, CpTIMERphys, CpTIMERphysctl, Imask|Enable);
    timerintr(ureg, 0);
}
/*e: function localclockintr(arm) */

/*s: function clockshutdown(arm) */
void
clockshutdown(void)
{
    Armtimer *tm;

    tm = (Armtimer*)ARMTIMER;
    tm->ctl = 0;
    //wdogoff();
}
/*e: function clockshutdown(arm) */

/*s: function clockinit(arm) */
void
clockinit(void)
{
    Systimers *tn;
    Armtimer *tm;
    u32int t0, t1, tstart, tend;

    /*s: [[clockinit()]] if many processors */
    if(((cprdsc(0, CpID, CpIDfeat, 1) >> 16) & 0xF) != 0) {
        /* generic timer supported */
        if(cpu->cpuno == 0){
            *(ulong*)(ARMLOCAL + Localctl) = 0;             /* magic */
            *(ulong*)(ARMLOCAL + Prescaler) = 0x06aaaaab;   /* magic for 1 Mhz */
        }
        cpwrsc(0, CpTIMER, CpTIMERphys, CpTIMERphysctl, Imask);
    }
    /*e: [[clockinit()]] if many processors */

    tn = (Systimers*)SYSTIMERS;
    tstart = tn->clo;
    do{
        t0 = arch_lcycles();
    }while(tn->clo == tstart); // PB QEMU
    tend = tstart + 10000;
    do{
        t1 = arch_lcycles();
    }while(tn->clo != tend); // PB QEMU
    t1 -= t0;

    cpu->cpuhz = 100 * t1;
    cpu->cpumhz = (cpu->cpuhz + Mhz/2 - 1) / Mhz;

    cpu->cyclefreq = cpu->cpuhz;

    if(cpu->cpuno == 0){
        tn->c3 = tn->clo - 1;
        tm = (Armtimer*)ARMTIMER;
        tm->load = 0;
        tm->ctl = TmrPrescale1|CntEnable|CntWidth32;
        arch_intrenable(IRQtimer3, clockintr, nil, 0, "clock");
    }
    /*s: [[clockinit()]] if not cpu0 */
    else
        arch_intrenable(IRQcntpns, localclockintr, nil, 0, "clock");
    /*e: [[clockinit()]] if not cpu0 */
}
/*e: function clockinit(arm) */

/*s: function arch_timerset(arm) */
void
arch_timerset(Tval next)
{
    Systimers *tn;
    uvlong now;
    long period;

    now = arch_fastticks(nil);
    period = next - now;
    if(period < MinPeriod)
        period = MinPeriod;
    else if(period > MaxPeriod)
        period = MaxPeriod;
    /*s: [[arch_timerset()]] if not cpu0 */
    if(cpu->cpuno > 0){
        cpwrsc(0, CpTIMER, CpTIMERphys, CpTIMERphysval, period);
        cpwrsc(0, CpTIMER, CpTIMERphys, CpTIMERphysctl, Enable);
    }
    /*e: [[arch_timerset()]] if not cpu0 */
    else{
        tn = (Systimers*)SYSTIMERS;
        tn->c3 = (ulong)(now + period);
    }
}
/*e: function arch_timerset(arm) */

/*s: function clock_arch_fastticks(arm) */
uvlong
clock_arch_fastticks(uvlong *hz)
{
    Systimers *tn;
    ulong lo, hi;
    uvlong now;
    int s;

    if(hz)
        *hz = SystimerFreq;
    tn = (Systimers*)SYSTIMERS;
    s = arch_splhi();
    do{
        hi = tn->chi;
        lo = tn->clo;
    }while(tn->chi != hi);
    now = (uvlong)hi<<32 | lo;
    cpu->fastclock = now;
    arch_splx(s);
    return cpu->fastclock;
}
/*e: function clock_arch_fastticks(arm) */

/*s: function arch_perfticks(arm) */
ulong
arch_perfticks(void)
{
    Armtimer *tm;

    tm = (Armtimer*)ARMTIMER;
    return tm->count;
}
/*e: function arch_perfticks(arm) */

/*s: function armtimerset(arm) */
void
armtimerset(int n)
{
    Armtimer *tm;

    tm = (Armtimer*)ARMTIMER;
    if(n > 0){
        tm->ctl |= TmrEnable|TmrIntEnable;
        tm->load = n;
    }else{
        tm->load = 0;
        tm->ctl &= ~(TmrEnable|TmrIntEnable);
        tm->irq = 1;
    }
}
/*e: function armtimerset(arm) */

/*s: function arch_us(arm) */
ulong
arch_us(void)
{
    /*s: [[arch_us()]] if non-standard systimer frequency */
    if(SystimerFreq != 1*Mhz)
        return fastticks2us(arch_fastticks(nil));
    /*e: [[arch_us()]] if non-standard systimer frequency */
    return arch_fastticks(nil);
}
/*e: function arch_us(arm) */

/*s: function clock_arch_microdelay(arm) */
void
clock_arch_microdelay(int n)
{
    Systimers *tn;
    u32int now, diff;

    diff = n + 1;
    tn = (Systimers*)SYSTIMERS;
    now = tn->clo;
    while(tn->clo - now < diff) // PB QEMU
        ;
}
/*e: function clock_arch_microdelay(arm) */

/*s: function clock_arch_delay(arm) */
void
clock_arch_delay(int n)
{
    while(--n >= 0)
        arch_microdelay(1000);
}
/*e: function clock_arch_delay(arm) */
/*e: time/arm/clock.c */
