/*s: portdat_time.h */

/*s: enum timermode */
/*
 * fasttick timer interrupts
 */
enum Timermode 
{
    Trelative,  /* timer programmed in ns from now */
    Tperiodic,  /* periodic timer, period in ns */
};
/*e: enum timermode */

/*s: type Txxx */
typedef vlong   Tval; // ticks
typedef vlong   Tnano; // nanoseconds
typedef vlong   Tmicro; // microseconds
typedef int     Tms; // milliseconds
typedef vlong   Tsec; // seconds
/*e: type Txxx */

/*s: struct Timer */
struct Timer
{
    /* Public interface */
    // enum<timermode>
    int tmode;    /* See above */
    Tnano tns;    /* meaning defined by mode */ //nanosecond
    void  (*tf)(Ureg*, Timer*);
    void  *ta;
  
    /* Internal */
    Lock;
    Tval  tticks;   /* tns converted to ticks */
    Tval  twhen;    /* ns represented in arch_fastticks */

    /*s: [[Timer extra fields */
    // list<Timer> of Timers.head
    Timer *tnext;
    // ref<list<Timer>> Timers.head
    Timers  *tt;    /* Timers queue this timer runs on */
    /*e: [[Timer extra fields */
    };
/*e: struct Timer */

// was in clock.c
/*s: struct Timers */
struct Timers
{
    // list<Timer> (next = Timer.tnext)
    Timer *head;
    // extra
    Lock;
};
/*e: struct Timers */

// used only in arm/ for now
enum {
 Mhz	= 1000 * 1000,
};

// <arch>/dat_time.h should defined Arch_HZ

#pragma varargck  type  "t"   long
#pragma varargck  type  "U"   uvlong
/*e: portdat_time.h */
