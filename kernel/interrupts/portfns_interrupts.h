/*s: portfns_interrupts.h */

// <arch>/trap.c (used in arch specific)
void  arch_intrenable(int, void (*)(Ureg*, void*), void*, int, char*);

// <arch>/trap.c (called from main)
void  arch_trapinit(void);

/*e: portfns_interrupts.h */
