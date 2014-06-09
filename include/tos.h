/*s: tos.h */
typedef struct Plink Plink;
#pragma incomplete Plink

/*s: struct Tos */
struct Tos {
    /*s: [[Tos]] prof field */
    struct			/* Per process profiling */
    {
        Plink	*pp;	/* known to be 0(ptr) */
        Plink	*next;	/* known to be 4(ptr) */
        Plink	*last;
        Plink	*first;
        ulong	pid;
        ulong	what;
    } prof;
    /*e: [[Tos]] prof field */
    uvlong cyclefreq;	/* cycle clock frequency if there is one, 0 otherwise */

    vlong kcycles;	/* cycles spent in kernel */
    vlong pcycles;	/* cycles spent in process (kernel + user) */
    ulong clock; // in ms

    ulong pid;		/* might as well put the pid here */
    /*s: [[Tos]] other fields */
    /* scratch space for kernel use (e.g., mips fp delay-slot execution) */
    ulong	kscr[4];
    /*e: [[Tos]] other fields */
    /* top of stack is here */
};
/*e: struct Tos */
typedef struct Tos Tos;

extern Tos *_tos;
/*e: tos.h */
