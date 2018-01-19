/*s: i8253.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "io.h"

//*****************************************************************************
// Data
//*****************************************************************************

/*
 *  8253 timer
 */
enum
{
    T0cntr= 0x40,       /* counter ports */
    T1cntr= 0x41,       /* ... */
    T2cntr= 0x42,       /* ... */
    Tmode=  0x43,       /* mode port (control word register) */
    T2ctl=  0x61,       /* counter 2 control port */

    /* commands */
    Latch0= 0x00,       /* latch counter 0's value */
    Load0l= 0x10,       /* load counter 0's lsb */
    Load0m= 0x20,       /* load counter 0's msb */
    Load0=  0x30,       /* load counter 0 with 2 bytes */

    Latch1= 0x40,       /* latch counter 1's value */
    Load1l= 0x50,       /* load counter 1's lsb */
    Load1m= 0x60,       /* load counter 1's msb */
    Load1=  0x70,       /* load counter 1 with 2 bytes */

    Latch2= 0x80,       /* latch counter 2's value */
    Load2l= 0x90,       /* load counter 2's lsb */
    Load2m= 0xa0,       /* load counter 2's msb */
    Load2=  0xb0,       /* load counter 2 with 2 bytes */

    /* 8254 read-back command: everything > pc-at has an 8254 */
    Rdback= 0xc0,       /* readback counters & status */
    Rdnstat=0x10,       /* don't read status */
    Rdncnt= 0x20,       /* don't read counter value */
    Rd0cntr=0x02,       /* read back for which counter */
    Rd1cntr=0x04,
    Rd2cntr=0x08,

    /* modes */
    ModeMsk=0xe,
    Square= 0x6,        /* periodic square wave */
    Trigger=0x0,        /* interrupt on terminal count */
    Sstrobe=0x8,        /* software triggered strobe */

    /* T2ctl bits */
    T2gate= (1<<0),     /* enable T2 counting */
    T2spkr= (1<<1),     /* connect T2 out to speaker */
    T2out=  (1<<5),     /* output of T2 */
};

enum {
    /*s: constant [[Freq]](x86) */
    Freq=   1193182,    /* Real clock frequency */
    /*e: constant [[Freq]](x86) */
    Tickshift=8,        /* extra accuracy */
    MaxPeriod=Freq/Arch_HZ,
    MinPeriod=Freq/(100*Arch_HZ),

    Wdogms  = 200,      /* ms between strokes */
};

/*s: global [[i8253]](x86) */
I8253 i8253;
/*e: global [[i8253]](x86) */

/*s: struct [[Watchdog]](x86) */
struct Watchdog
{
    void    (*enable)(void);    /* watchdog enable */
    void    (*disable)(void);   /* watchdog disable */
    void    (*restart)(void);   /* watchdog restart */
    void    (*stat)(char*, char*);  /* watchdog statistics */
};
/*e: struct [[Watchdog]](x86) */

/*s: global [[watchdog]](x86) */
Watchdog* watchdog;
int watchdogon;
/*e: global [[watchdog]](x86) */


//*****************************************************************************
// Init
//*****************************************************************************

/*s: function [[i8253init]](x86) */
void
i8253init(void)
{
    int loops, x;

    ioalloc(T0cntr, 4, 0, "i8253");
    ioalloc(T2ctl, 1, 0, "i8253.cntr2ctl");

    i8253.period = Freq/Arch_HZ;

    /*
     *  enable a 1/HZ interrupt for providing scheduling interrupts
     */
    outb(Tmode, Load0|Square);
    outb(T0cntr, (Freq/Arch_HZ));    /* low byte */
    outb(T0cntr, (Freq/Arch_HZ)>>8); /* high byte */

    /*
     *  enable a longer period counter to use as a clock
     */
    outb(Tmode, Load2|Square);
    outb(T2cntr, 0);        /* low byte */
    outb(T2cntr, 0);        /* high byte */
    x = inb(T2ctl);
    x |= T2gate;
    outb(T2ctl, x);
    
    /*
     * Introduce a little delay to make sure the count is
     * latched and the timer is counting down; with a fast
     * enough processor this may not be the case.
     * The i8254 (which this probably is) has a read-back
     * command which can be used to make sure the counting
     * register has been written into the counting element.
     */
    x = (Freq/Arch_HZ);
    for(loops = 0; loops < 100000 && x >= (Freq/Arch_HZ); loops++){
        outb(Tmode, Latch0);
        x = inb(T0cntr);
        x |= inb(T0cntr)<<8;
    }
}
/*e: function [[i8253init]](x86) */

//*****************************************************************************
// Misc
//*****************************************************************************

/*s: function [[wdogpause]](x86) */
/*
 * if the watchdog is running and we're on cpu 0 and ignoring (clock)
 * interrupts, disable the watchdog temporarily so that the (presumed)
 * long-running loop to follow will not trigger an NMI.
 * wdogresume restarts the watchdog if wdogpause stopped it.
 */
static int
wdogpause(void)
{
    int turndogoff;

    turndogoff = watchdogon && cpu->cpuno == 0 && !arch_islo();
    if (turndogoff) {
        watchdog->disable();
        watchdogon = 0;
    }
    return turndogoff;
}
/*e: function [[wdogpause]](x86) */

/*s: function [[wdogresume]](x86) */
static void
wdogresume(int resume)
{
    if (resume) {
        watchdog->enable();
        watchdogon = 1;
    }
}
/*e: function [[wdogresume]](x86) */

/*s: function [[guesscpuhz]](x86) */
void
guesscpuhz(int aalcycles)
{
    int loops, incr, x, y, dogwason;
    uvlong a, b, cpufreq;

    dogwason = wdogpause();     /* don't get NMI while busy looping */

    /* find biggest loop that doesn't wrap */
    incr = 16000000/(aalcycles*Arch_HZ*2);
    x = 2000;
    for(loops = incr; loops < 64*1024; loops += incr) {
    
        /*
         *  measure time for the loop
         *
         *          MOVL    loops,CX
         *  aaml1:      AAM
         *          LOOP    aaml1
         *
         *  the time for the loop should be independent of external
         *  cache and memory system since it fits in the execution
         *  prefetch buffer.
         *
         */
        outb(Tmode, Latch0);
        arch_cycles(&a);
        x = inb(T0cntr);
        x |= inb(T0cntr)<<8;
        aamloop(loops);
        outb(Tmode, Latch0);
        arch_cycles(&b);
        y = inb(T0cntr);
        y |= inb(T0cntr)<<8;
        x -= y;
    
        if(x < 0)
            x += Freq/Arch_HZ;

        if(x > Freq/(3*Arch_HZ))
            break;
    }
    wdogresume(dogwason);

    /*
     *  figure out clock frequency and a loop multiplier for delay().
     *  n.b. counter goes up by 2*Freq
     */
    if(x == 0)
        x = 1;          /* avoid division by zero on vmware 7 */
    cpufreq = (vlong)loops*((aalcycles*2*Freq)/x);
    cpu->loopconst = (cpufreq/1000)/aalcycles;    /* AAM+LOOP's for 1 ms */

    if(cpu->havetsc && a != b){  /* a == b means virtualbox has confused us */
        /* counter goes up by 2*Freq */
        b = (b-a)<<1;
        b *= Freq;
        b /= x;

        /*
         *  round to the nearest megahz
         */
        cpu->cpumhz = (b+500000)/1000000L;
        cpu->cpuhz = b;
        cpu->cyclefreq = b;
    } else {
        /*
         *  add in possible 0.5% error and convert to MHz
         */
        cpu->cpumhz = (cpufreq + cpufreq/200)/1000000;
        cpu->cpuhz = cpufreq;
    }

    /* don't divide by zero in trap.c */
    if (cpu->cpumhz == 0)
        panic("guesscpuhz: zero cpu->cpumhz");
    i8253.hz = Freq<<Tickshift;
}
/*e: function [[guesscpuhz]](x86) */

/*s: function [[i8253timerset]](x86) */
void
i8253timerset(uvlong next)
{
    long period;
    ulong want;
    ulong now;

    period = MaxPeriod;
    if(next != 0){
        want = next>>Tickshift;
        now = i8253.ticks;  /* assuming whomever called us just did arch_fastticks() */

        period = want - now;
        if(period < MinPeriod)
            period = MinPeriod;
        else if(period > MaxPeriod)
            period = MaxPeriod;
    }

    /* hysteresis */
    if(i8253.period != period){
        ilock(&i8253);
        /* load new value */
        outb(Tmode, Load0|Square);
        outb(T0cntr, period);       /* low byte */
        outb(T0cntr, period >> 8);      /* high byte */

        /* remember period */
        i8253.period = period;
        i8253.periodset++;
        iunlock(&i8253);
    }
}
/*e: function [[i8253timerset]](x86) */

/*s: interrupt callback i8253clock(x86) */
static void
i8253clock(Ureg* ureg, void*)
{
    timerintr(ureg, 0);
}
/*e: interrupt callback i8253clock(x86) */

/*s: function [[i8253enable]](x86) */
void
i8253enable(void)
{
    i8253.enabled = true;
    i8253.period = Freq/Arch_HZ;
    arch_intrenable(IrqCLOCK, i8253clock, 0, BUSUNKNOWN, "clock");
}
/*e: function [[i8253enable]](x86) */

/*s: function [[i8253read]](x86) */
/*
 *  return the total ticks of counter 2.  We shift by
 *  8 to give timesync more wriggle room for interpretation
 *  of the frequency
 */
uvlong
i8253read(uvlong *hz)
{
    ushort y, x;
    uvlong ticks;

    if(hz)
        *hz = i8253.hz;

    ilock(&i8253);
    outb(Tmode, Latch2);
    y = inb(T2cntr);
    y |= inb(T2cntr)<<8;

    if(y < i8253.last)
        x = i8253.last - y;
    else {
        x = i8253.last + (0x10000 - y);
        if (x > 3*MaxPeriod) {
            outb(Tmode, Load2|Square);
            outb(T2cntr, 0);        /* low byte */
            outb(T2cntr, 0);        /* high byte */
            y = 0xFFFF;
            x = i8253.period;
        }
    }
    i8253.last = y;
    i8253.ticks += x>>1;
    ticks = i8253.ticks;
    iunlock(&i8253);

    return ticks<<Tickshift;
}
/*e: function [[i8253read]](x86) */

/*s: function [[delay]](x86) */
void
i8253_delay(int millisecs)
{
    if (millisecs > 10*1000)
        iprint("delay(%d) from %#p\n", millisecs,
            getcallerpc(&millisecs));
    if (watchdogon && cpu->cpuno == 0 && !arch_islo())
        for (; millisecs > Wdogms; millisecs -= Wdogms) {
            arch_delay(Wdogms);
            watchdog->restart();
        }
    millisecs *= cpu->loopconst;
    if(millisecs <= 0)
        millisecs = 1;
    aamloop(millisecs);
}
/*e: function [[delay]](x86) */

/*s: function [[microdelay]](x86) */
void
i8253_microdelay(int microsecs)
{
    if (watchdogon && cpu->cpuno == 0 && !arch_islo())
        for (; microsecs > Wdogms*1000; microsecs -= Wdogms*1000) {
            arch_delay(Wdogms);
            watchdog->restart();
        }
    microsecs *= cpu->loopconst;
    microsecs /= 1000;
    if(microsecs <= 0)
        microsecs = 1;
    aamloop(microsecs);
}
/*e: function [[microdelay]](x86) */

/*s: function [[perfticks]](x86) */
/*  
 *  performance measurement ticks.  must be low overhead.
 *  doesn't have to count over a second.
 */
ulong
arch_perfticks(void)
{
    uvlong x;

    if(cpu->havetsc)
        arch_cycles(&x);
    else
        x = 0;
    return x;
}
/*e: function [[perfticks]](x86) */
/*e: i8253.c */
