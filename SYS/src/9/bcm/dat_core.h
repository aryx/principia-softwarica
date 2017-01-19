
// was vlong in x86
//typedef uvlong		Tval;

typedef u32int		PTE;

/*
 *  MMU stuff in Mach.
 */
struct MMMU
{
	PTE*	mmul1;		/* l1 for this processor */
	int	mmul1lo;
	int	mmul1hi;

	int	mmupid;
};


struct ArchCpu {
	MMMU;

	Lock	alarmlock;		/* access to alarm list */
	void*	alarm;			/* alarms bound to this clock */

	int	cputype;
	ulong	delayloop;

	/* stats */

	uvlong	fastclock;		/* last sampled value */

	int	lastintr;

	/* vfp2 or vfp3 fpu */
	int	havefp;
	int	havefpvalid;
	int	fpon;
	int	fpconfiged;
	int	fpnregs;
	ulong	fpscr;			/* sw copy */
	int	fppid;			/* pid of last fault */
	uintptr	fppc;			/* addr of last fault */
	int	fpcnt;			/* how many consecutive at that addr */

	/* save areas for exceptions, hold R0-R4 */
	u32int	sfiq[5];
	u32int	sirq[5];
	u32int	sund[5];
	u32int	sabt[5];
	u32int	smon[5];		/* probably not needed */
	u32int	ssys[5];

};
