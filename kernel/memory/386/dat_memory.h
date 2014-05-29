/*s: dat_memory.h */

// define things used in Proc, for mmu.c to work on
/*
 *  MMU stuff in proc
 */
/*s: constant NCOLOR */
#define NCOLOR 1
/*e: constant NCOLOR */
/*s: struct ArchProcMMU */
//@Scheck: not dead, unnamed substructure
struct ArchProcMMU
{
  /*s: [[Proc]] [[Arch]] memory fields */
  Page* mmupdb;     /* page directory base */
  Page* mmufree;    /* unused page table pages */
  Page* mmuused;    /* used page table pages */
  /*x: [[Proc]] [[Arch]] memory fields */
  Page* kmaptable;    /* page table used by kmap */
  uint  lastkmap;   /* last entry used by kmap */
  int nkmap;      /* number of current kmaps */
  /*e: [[Proc]] [[Arch]] memory fields */
};
/*e: struct ArchProcMMU */

/*s: struct KMap */
/*
 * KMap the structure doesn't exist, but the functions do.
 */
typedef struct KMap   KMap;
/*e: struct KMap */
/*s: macro VA */
#define VA(k)   ((virt_addr3)(k))
/*e: macro VA */
/*e: dat_memory.h */
