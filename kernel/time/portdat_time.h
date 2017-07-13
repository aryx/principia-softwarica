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
typedef vlong   Tsec; // seconds
typedef int     Tms; // milliseconds
typedef vlong   Tmicro; // microseconds
typedef vlong   Tnano; // nanoseconds
/*x: type Txxx */
typedef vlong   Tval; // ticks
/*x: type Txxx */
typedef vlong   Tfast; // fastticks
typedef uvlong   Tufast; // fastticks
/*e: type Txxx */

/*s: struct Timer */
struct Timer
{
    /* Public interface */
    // enum<Timermode>
    int tmode;    /* See above */
    Tnano tns;    /* meaning defined by mode */ //nanosecond

    void  (*tf)(Ureg*, Timer*);
    void  *ta; // extra argument (generic pointer)
  
    /* Internal */
    Tfast  twhen;    /* ns represented in fastticks */
    ILock; // LOCK ORDERING: ilock(timer); ilock(timers)

    /*s: [[Timer]] extra fields */
    // list<Timer> of Timers.head
    Timer *tnext;
    /*x: [[Timer]] extra fields */
    // option<ref<list<Timer>> Timers.head (of one of global timers[MAXCPUS])
    Timers  *tt;    /* Timers queue this timer runs on */
    /*e: [[Timer]] extra fields */
    };
/*e: struct Timer */

// was in clock.c
/*s: struct Timers */
struct Timers
{
    // list<Timer> (next = Timer.tnext)
    Timer *head;
    // extra
    ILock;
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
