/*s: dat_memory.h */

#define NCOLOR 1

// define things used in Proc, for mmu.c to work on
/*
 *  MMU stuff in proc
 */
/*s: struct ArchProcMMU(x86) */
struct Arch_ProcMMU
{
  /*s: [[Proc]] [[Arch]] memory fields(x86) */
  Page* mmupd;     /* page directory base */
  /*x: [[Proc]] [[Arch]] memory fields(x86) */
  // type? list<ref<?? > >
  Page* mmufree;    /* unused page table pages */
  Page* mmuused;    /* used page table pages */
  /*e: [[Proc]] [[Arch]] memory fields(x86) */
};
/*e: struct ArchProcMMU(x86) */

/*s: struct KMap(x86) */
/*
 * KMap the structure doesn't exist, but the functions do.
 */
typedef struct Arch_KMap   Arch_KMap;
/*e: struct KMap(x86) */
/*s: macro VA(x86) */
#define VA(k)   ((virt_addr3)(k))
/*e: macro VA(x86) */

/*e: dat_memory.h */
