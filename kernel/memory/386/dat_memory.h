
// define things used in Proc, for mmu.c to work on
/*
 *  MMU stuff in proc
 */
#define NCOLOR 1
//@Scheck: not dead, unnamed substructure
struct ArchProcMMU
{
  Page* mmupdb;     /* page directory base */
  Page* mmufree;    /* unused page table pages */
  Page* mmuused;    /* used page table pages */

  Page* kmaptable;    /* page table used by kmap */
  uint  lastkmap;   /* last entry used by kmap */
  int nkmap;      /* number of current kmaps */
};

/*
 * KMap the structure doesn't exist, but the functions do.
 */
typedef struct KMap   KMap;
#define VA(k)   ((void*)(k))
