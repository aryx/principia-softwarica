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
    // list<ref<Page> > pages used to store page tables
    Page*   mmul2;
    /*x: [[Proc]] [[Arch]] memory fields(arm) */
    // list<ref<Page> > (next = Page.next)
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
