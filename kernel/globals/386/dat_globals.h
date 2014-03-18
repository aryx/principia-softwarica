
/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// MAXMACH is defined in 386/mem.h
extern Mach* machp[MAXMACH];

#define	MACHP(n)	(machp[n])

// MACHADDR is defined in 386/mem.h
#define up	(((Mach*)MACHADDR)->externup)

extern struct Active active;

