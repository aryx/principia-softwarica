
/*
 *  MMU stuff in proc
 */
#define NCOLOR	1		/* 1 level cache, don't worry about VCE's */

struct Arch_ProcMMU
{
	Page*	mmul2;
	Page*	mmul2cache;	/* free mmu pages */
};

/*
 * Fake kmap.
 */
typedef void		Arch_KMap;
#define	VA(k)		((uintptr)(k))

