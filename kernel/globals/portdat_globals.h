
extern Conf conf;

// ref<Mach>, the actual Mach is where??
extern Mach *m;
/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// array<ref<Mach>>, MAXMACH is defined in 386/mem.h
extern Mach* machp[MAXMACH];

// array<Dev>, it looks like an allocated array<ref<dev>> but
// it is really a static array put here to avoid backward deps on conf_devtab,
// and it is not really a <ref<dev>> because it's pointer to static
// structures (e.g. mousedevtab, vgadevtab, etc).
extern Dev** devtab;

extern char* eve;
int iseve(void);

#define MACHP(n)  (machp[n])
// MACHADDR is defined in 386/mem.h
// TODO: why not m->externup? m is not valid?
#define up  (((Mach*)MACHADDR)->externup)
#define poperror()    up->nerrlab--
#define waserror()  (up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
