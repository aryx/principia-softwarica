/*s: dat_memory.h */

// define things used in Proc, for mmu.c to work on
/*
 *  MMU stuff in proc
 */
/*s: struct ArchProcMMU */
//@Scheck: not dead, unnamed substructure
struct ArchProcMMU
{
  /*s: [[Proc]] [[Arch]] memory fields */
  Page* mmupd;     /* page directory base */
  /*x: [[Proc]] [[Arch]] memory fields */
  // type? list<ref<?? > >
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
