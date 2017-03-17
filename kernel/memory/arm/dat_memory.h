/*s: memory/arm/dat_memory.h */

/*
 *  MMU stuff in proc
 */
/*s: constant NCOLOR(arm) */
#define NCOLOR  1       /* 1 level cache, don't worry about VCE's */
/*e: constant NCOLOR(arm) */

/*s: struct Arch_Proc(arm) */
struct Arch_Proc
{
    /*s: [[Proc]] [[Arch]] memory fields(arm) */
    Page*   mmul2;
    /*x: [[Proc]] [[Arch]] memory fields(arm) */
    Page*   mmul2cache; /* free mmu pages */
    /*e: [[Proc]] [[Arch]] memory fields(arm) */
};
/*e: struct Arch_Proc(arm) */

/*
 * Fake kmap.
 */
typedef void        Arch_KMap;
/*s: macro VA(arm) */
#define VA(k)       ((uintptr)(k))
/*e: macro VA(arm) */

/*e: memory/arm/dat_memory.h */
