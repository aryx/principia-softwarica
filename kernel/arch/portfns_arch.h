
// <arch>/arch.c (called from port)
void   arch_timerset(Tval x); // called from portclock.c (e.g., addclock0link)
ulong  arch_us(void);         // called from edf.c
uvlong arch_fastticks(uvlong *hz); // called from portclock.c
long   arch_lcycles(void); // called from edf.c, sysproc.c, taslock.c
// <arch>/arch.c (called from main)
void   arch_cpuidprint(void);
// <arch>/arch.c (called from port but signature not portable across <arch>)
//void arch_cycles(uvlong*);
