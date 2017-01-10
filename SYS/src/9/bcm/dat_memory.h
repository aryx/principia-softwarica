
/*
 *  MMU stuff in proc
 */
#define NCOLOR	1		/* 1 level cache, don't worry about VCE's */

struct ArchProcMMU
{
	Page*	mmul2;
	Page*	mmul2cache;	/* free mmu pages */
};

/*
 * Fake kmap.
 */
typedef void		KMap;
#define	VA(k)		((uintptr)(k))

#define	kmap(p)		(KMap*)((p)->pa|kseg0)
#define	kunmap(k)
