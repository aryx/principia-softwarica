/*s: memory/arm/dat_memory.h */

/*s: constant NCOLOR(arm) */
/*
 *  MMU stuff in proc
 */
#define NCOLOR  1       /* 1 level cache, don't worry about VCE's */
/*e: constant NCOLOR(arm) */

/*s: struct Arch_ProcMMU(arm) */
struct Arch_ProcMMU
{
    /*s: [[Proc]] [[Arch]] memory fields(arm) */
    Page*   mmul2;
    Page*   mmul2cache; /* free mmu pages */
    /*e: [[Proc]] [[Arch]] memory fields(arm) */
};
/*e: struct Arch_ProcMMU(arm) */

/*
 * Fake kmap.
 */
typedef void        Arch_KMap;
/*s: macro VA(arm) */
#define VA(k)       ((uintptr)(k))
/*e: macro VA(arm) */

/*e: memory/arm/dat_memory.h */
