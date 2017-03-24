/*s: memory/arm/mem.h */
/*
 * Memory and machine-specific definitions.  Used in C and assembler.
 */

/*s: constant KiB(arm) */
#define KiB     1024u           /* Kibi 0x0000000000000400 */
/*e: constant KiB(arm) */
/*s: constant MiB(arm) */
#define MiB     1048576u        /* Mebi 0x0000000000100000 */
/*e: constant MiB(arm) */
/*s: constant GiB(arm) */
#define GiB     1073741824u     /* Gibi 000000000040000000 */
/*e: constant GiB(arm) */

/*
 * Sizes
 */
/*s: constant BY2PG(arm) */
#define BY2PG       (4*KiB)         /* bytes per page */
/*e: constant BY2PG(arm) */
/*s: constant PGSHIFT(arm) */
#define PGSHIFT     12          /* log(BY2PG) */
/*e: constant PGSHIFT(arm) */

/*s: constant MAXCPUS(arm) */
#define MAXCPUS     4           /* max # cpus system can run */
/*e: constant MAXCPUS(arm) */
/*s: constant CPUSIZE(arm) */
#define CPUSIZE BY2PG
/*e: constant CPUSIZE(arm) */
/*s: constant L1SIZE(arm) */
#define L1SIZE      (4 * BY2PG)
/*e: constant L1SIZE(arm) */

/*s: macro STACKALIGN(arm) */
#define STACKALIGN(sp)  ((sp) & ~3)     /* bug: assure with alloc */
/*e: macro STACKALIGN(arm) */

/*
 * Magic registers
 */
/*s: constant UP(arm) */
#define UP      9       /* R9 is up-> */
/*e: constant UP(arm) */
/*s: constant CPU(arm) */
#define CPU     10      /* R10 is cpu-> */
/*e: constant CPU(arm) */

/*
 * Address spaces.
 * KTZERO is used by kprof and dumpstack (if any).
 *
 * KZERO is mapped to physical 0 (start of ram).
 *
 * vectors are at 0, plan9.ini is at KZERO+256 and is limited to 16K by
 * devenv.
 */
/*s: constant KSEG0(arm) */
#define KSEG0       0x80000000      /* kernel segment */
/*e: constant KSEG0(arm) */
/*s: constant KSEGM(arm) */
/* mask to check segment; good for 1GB dram */
#define KSEGM       0xC0000000
/*e: constant KSEGM(arm) */

/*s: constant KZERO(arm) */
#define KZERO       KSEG0           /* kernel address space */
/*e: constant KZERO(arm) */

/*s: constant CONFADDR(arm) */
#define CONFADDR    (KZERO+0x100)       /* unparsed plan9.ini */
/*e: constant CONFADDR(arm) */

// L1 must be 16KiB aligned so this dictates the organization below
/*s: constant CPUADDR(arm) */
#define CPUADDR     (KZERO+0x2000)      /* Cpu structure */
/*e: constant CPUADDR(arm) */
/*s: constant L2(arm) */
#define L2          (KZERO+0x3000)      /* L2 ptes for vectors etc */
/*e: constant L2(arm) */
/*s: constant VCBUFFER(arm) */
#define VCBUFFER    (KZERO+0x3400)      /* videocore mailbox buffer */
/*e: constant VCBUFFER(arm) */
/*s: constant FIQSTKTOP(arm) */
#define FIQSTKTOP   (KZERO+0x4000)      /* FIQ stack */
/*e: constant FIQSTKTOP(arm) */
/*s: constant L1(arm) */
#define L1          (KZERO+0x4000)      /* ttb ptes: 16KiB aligned */
/*e: constant L1(arm) */

/*s: constant KTZERO(arm) */
#define KTZERO      (KZERO+0x8000)      /* kernel text start */
/*e: constant KTZERO(arm) */

/*s: constant VIRTIO(arm) */
#define VIRTIO      0x7E000000      /* i/o registers */
/*e: constant VIRTIO(arm) */
/*s: constant FRAMEBUFFER(arm) */
#define FRAMEBUFFER 0xC0000000      /* video framebuffer */
/*e: constant FRAMEBUFFER(arm) */

/*s: constant UZERO(arm) */
#define UZERO       0           /* user segment */
/*e: constant UZERO(arm) */
/*s: constant UTZERO(arm) */
#define UTZERO      (UZERO+BY2PG)       /* user text start */
/*e: constant UTZERO(arm) */
/*s: macro UTROUND(arm) */
#define UTROUND(t)  ROUNDUP((t), BY2PG)
/*e: macro UTROUND(arm) */
/*s: constant USTKTOP(arm) */
#define USTKTOP     0x20000000      /* user segment end +1 */
/*e: constant USTKTOP(arm) */
/*s: constant USTKSIZE(arm) */
#define USTKSIZE    (8*1024*1024)       /* user stack size */
/*e: constant USTKSIZE(arm) */
/*s: constant TSTKTOP(arm) */
// end of new stack in sysexec
#define TSTKTOP     (USTKTOP-USTKSIZE)  /* sysexec temporary stack */
/*e: constant TSTKTOP(arm) */
/*s: constant TSTKSIZ(arm) */
#define TSTKSIZ     256 // pages in new stack; limits exec args
/*e: constant TSTKSIZ(arm) */

/*s: constant REBOOTADDR(arm) */
/* address at which to copy and execute rebootcode */
#define REBOOTADDR  (KZERO+0x1800)
/*e: constant REBOOTADDR(arm) */

/*
 * Legacy...
 */
//old: was 64 in bcm-latest, but then get panic in _allocb
/*s: constant BLOCKALIGN(arm) */
// possible alternative fix: git show 2342e706e42d1e6653c51cc1433f44816cf53fe6
#define BLOCKALIGN  32          /* only used in allocb.c */
/*e: constant BLOCKALIGN(arm) */
/*s: constant KSTACK(arm) */
#define KSTACK      (8*KiB)
/*e: constant KSTACK(arm) */

/*
 * Sizes
 */
/*s: constant BI2BY(arm) */
#define BI2BY       8           /* bits per byte */
/*e: constant BI2BY(arm) */
/*s: constant BY2WD(arm) */
#define BY2WD       4
/*e: constant BY2WD(arm) */
/*s: constant BY2V(arm) */
#define BY2V        8           /* only used in xalloc.c */
/*e: constant BY2V(arm) */

/*s: constant PAGETABMAPMEM(arm) */
#define PAGETABMAPMEM   (1024*1024) // 1MB
/*e: constant PAGETABMAPMEM(arm) */
/*s: constant PAGETABSIZE(arm) */
#define PAGETABSIZE 256 // (PAGETABMAPMEM/BY2PG)
/*e: constant PAGETABSIZE(arm) */
/*s: constant PAGEDIRSIZE(arm) */
#define PAGEDIRSIZE 1984
/*e: constant PAGEDIRSIZE(arm) */
/*s: constant SMALLPAGEDIRSIZE(arm) */
#define SMALLPAGEDIRSIZE    16
/*e: constant SMALLPAGEDIRSIZE(arm) */
/*s: macro PPN(arm) */
#define PPN(x)      ((x)&~(BY2PG-1))
/*e: macro PPN(arm) */

/*
 * Physical machine information from here on.
 *  PHYS addresses as seen from the arm cpu.
 *  BUS  addresses as seen from the videocore gpu.
 */
/*s: constant PHYSDRAM(arm) */
#define PHYSDRAM    0
/*e: constant PHYSDRAM(arm) */
/*s: constant IOSIZE(arm) */
#define IOSIZE      (16*MiB)
/*e: constant IOSIZE(arm) */
/*e: memory/arm/mem.h */
