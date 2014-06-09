/*s: tos.h */
typedef struct Plink Plink;
#pragma incomplete Plink

struct Tos {
	struct			/* Per process profiling */
	{
		Plink	*pp;	/* known to be 0(ptr) */
		Plink	*next;	/* known to be 4(ptr) */
		Plink	*last;
		Plink	*first;
		ulong	pid;
		ulong	what;
	} prof;

	uvlong	cyclefreq;	/* cycle clock frequency if there is one, 0 otherwise */
	vlong	kcycles;	/* cycles spent in kernel */
	vlong	pcycles;	/* cycles spent in process (kernel + user) */
	ulong	clock;

	ulong	pid;		/* might as well put the pid here */
	/* scratch space for kernel use (e.g., mips fp delay-slot execution) */
	ulong	kscr[4];
	/* top of stack is here */
};
typedef struct Tos Tos;

extern Tos *_tos;
/*e: tos.h */
