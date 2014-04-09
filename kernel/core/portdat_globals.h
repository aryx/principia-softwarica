
extern Conf conf;

// hash<string, string>
#define MAXCONF         64
extern char *confname[];
extern char *confval[];
// Hashtbl.length(confname)
extern int nconf;
extern bool cpuserver; // defined in conf/pcf.c

char* getconf(char *name);

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
#define MACHP(n)  (machp[n])


extern char* eve;
int iseve(void);

extern ulong	kerndate; // defined in ???

// used to be in devcons.c, but used also by edf.c
extern int panicking;

//TODO: mv in 386/
// MACHADDR is defined in 386/mem.h
// TODO: why not m->externup? m is not valid?
#define up  (((Mach*)MACHADDR)->externup)
#define poperror()    up->nerrlab--
#define waserror()  (up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
