/*s: portfns_time.h */

// portclock.c
ulong   tk2ms(ulong);
#define   TK2MS(x) ((x)*(1000/HZ))
#define   MS2NS(n) (((vlong)(n))*1000000LL)
ulong   ms2tk(ulong);
void    timerdel(Timer*);
void    timeradd(Timer*);
Timer*  addclock0link(void (*)(void), int);
void    timerintr(Ureg*, Tval);
void    timersinit(void);

// alarm.c
ulong   procalarm(ulong);
void    alarmkproc(void*);
void    checkalarms(void);

// tod.c
// initialize the hidden global tod.c#tod
void    todinit(void);
void    todsetfreq(vlong);
void    todset(vlong, vlong, int);
vlong   todget(vlong*);
uvlong  fastticks2us(uvlong);
uvlong  ns2fastticks(uvlong);
long    seconds(void);

// <arch>/clock.c (called from port)
void   arch_timerset(Tval x); // called from portclock.c (e.g., addclock0link)
ulong  arch_us(void);         // called from edf.c
uvlong arch_fastticks(uvlong *hz); // called from portclock.c
long   arch_lcycles(void); // called from edf.c, sysproc.c, taslock.c
ulong  arch_perfticks(void);
// <arch>/clock.c (called from port but signature not portable across <arch>)
//void arch_cycles(uvlong*);

/*e: portfns_time.h */
