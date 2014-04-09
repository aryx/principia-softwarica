
// define things used in Mach
struct Tss {
  ulong link;     /* link (old TSS selector) */
  ulong esp0;     /* privilege level 0 stack pointer */
  ulong ss0;      /* privilege level 0 stack selector */
  ulong esp1;     /* privilege level 1 stack pointer */
  ulong ss1;      /* privilege level 1 stack selector */
  ulong esp2;     /* privilege level 2 stack pointer */
  ulong ss2;      /* privilege level 2 stack selector */
  ulong xcr3;     /* page directory base register - not used because we don't use trap gates */
  ulong eip;      /* instruction pointer */
  ulong eflags;     /* flags register */
  ulong eax;      /* general registers */
  ulong   ecx;
  ulong edx;
  ulong ebx;
  ulong esp;
  ulong ebp;
  ulong esi;
  ulong edi;
  ulong es;     /* segment selectors */
  ulong cs;
  ulong ss;
  ulong ds;
  ulong fs;
  ulong gs;
  ulong ldt;      /* selector for task's LDT */
  ulong iomap;      /* I/O map base address + T-bit */
};

struct Segdesc
{
  ulong d0;
  ulong d1;
};


// define things used in Proc
/*
 *  MMU stuff in proc
 */
#define NCOLOR 1
//@Scheck: not dead, unnamed substructure
struct ArchMMU
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


// used to be in devrtc.c, but to remove some backward deps had to be here
// it's really used only by nvram.c and devrtc.c
/*
 *  real time clock and non-volatile ram
 */

enum {
	Paddr=		0x70,	/* address port */
	PdataPort=		0x71,	/* data port */
};
extern Lock nvrtlock;
