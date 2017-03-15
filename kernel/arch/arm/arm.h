/*s: arch/arm/arm.h */
/*
 * ARM-specific definitions for armv6 (arm11), armv7 (cortex-a7 and -a8)
 * these are used in C and assembler
 */

/*
 * Program Status Registers
 */
/*s: type PsrMode(arm) */
// Last 4 bits of PSR
#define PsrMusr     0x00000010  /* user mode */
#define PsrMsvc     0x00000013  /* `protected mode for OS' */
/*s: [[PsrMode]] other cases(arm) */
#define PsrMirq     0x00000012
#define PsrMabt     0x00000017
#define PsrMund     0x0000001B
/*x: [[PsrMode]] other cases(arm) */
#define PsrMfiq     0x00000011
/*e: [[PsrMode]] other cases(arm) */

#define PsrMask     0x0000001F
/*e: type PsrMode(arm) */

/*s: type PsrDisable(arm) */
#define PsrDfiq     0x00000040      /* disable FIQ interrupts */
#define PsrDirq     0x00000080      /* disable IRQ interrupts */
/*e: type PsrDisable(arm) */

/*
 * Coprocessors
 */
/*s: type Coprocessors(arm) */
#define CpSC        15          /* System Control */
/*s: [[Coprocessors]] other cases(arm) */
#define CpFP        10          /* float FP, VFP cfg. */
#define CpDFP       11          /* double FP */
/*e: [[Coprocessors]] other cases(arm) */
/*e: type Coprocessors(arm) */

/*
 * Primary (CRn) CpSC registers.
 */
/*s: type CpSC primary registers(arm) */
#define CpCONTROL   1           /* miscellaneous control */
#define CpTTB       2           /* Translation Table Base(s) */
#define CpDAC       3           /* Domain Access Control */
#define CpFSR       5           /* Fault Status */
#define CpFAR       6           /* Fault Address */
#define CpCACHE     7           /* cache/write buffer control */
#define CpTLB       8           /* TLB control */
#define CpSPM       15          /* system performance monitor (arm1176) */
/*s: [[CpSC primary registers]] other cases(arm) */
#define CpID        0           /* ID and cache type */
#define CpTIMER     14          /* Generic timer (cortex-a7) */
/*x: [[CpSC primary registers]] other cases(arm) */
#define CpCLD       9        /* L2 Cache Lockdown, op1==1 */
/*e: [[CpSC primary registers]] other cases(arm) */
/*e: type CpSC primary registers(arm) */


/*s: type CpID secondary registers(arm) */
/*
 * CpID Secondary (CRm) registers.
 */
#define CpIDidct    0
#define CpIDfeat    1
/*e: type CpID secondary registers(arm) */
/*s: type CpID opcode2(arm) */
/*
 * CpID op1==0 opcode2 fields.
 * the cortex has more op1 codes for cache size, etc.
 */
#define CpIDid      0           /* main ID */
#define CpIDct      1           /* cache type */
#define CpIDtlb     3           /* tlb type (cortex) */
#define CpIDmpid    5           /* multiprocessor id (cortex) */
#define CpIDrevid   6           /* extra revision ID */

/* CpIDid op1 values */
#define CpIDcsize   1           /* cache size (cortex) */
#define CpIDcssel   2           /* cache size select (cortex) */
/*e: type CpID opcode2(arm) */

/*s: type CpCONTROL opcode2(arm) */
/*
 * CpCONTROL op2 codes, op1==0, Crm==0.
 */
#define CpMainctl   0
/*s: [[CpCONTROL.opcode2]] other cases(arm) */
#define CpAuxctl    1
#define CpCPaccess  2
/*e: [[CpCONTROL.opcode2]] other cases(arm) */
/*e: type CpCONTROL opcode2(arm) */
/*s: type CpCONTROL CpMainctl(arm) */
/*
 * CpCONTROL: op1==0, CRm==0, op2==CpMainctl.
 * main control register.
 * cortex/armv7 has more ops and CRm values.
 */
#define CpCmmu      0x00000001  /* M: MMU enable */
#define CpCdcache   0x00000004  /* C: data cache on */
#define CpCicache   0x00001000  /* I: instruction cache on */
#define CpChv       0x00002000  /* V: high vectors */
/*s: [[CpCONTROL.CpMainctl]] other cases(arm) */
#define CpCsbo (3<<22|1<<18|1<<16|017<<3)   /* must be 1 (armv7) */
#define CpCsbz (CpCtre|1<<26|CpCve|1<<15|7<<7)  /* must be 0 (armv7) */
#define CpCsw       (1<<10)     /* SW: SWP(B) enable (deprecated in v7) */
#define CpCpredict  0x00000800  /* Z: branch prediction (armv7) */
/*x: [[CpCONTROL.CpMainctl]] other cases(arm) */
#define CpCve       (1<<24)     /* VE: intr vectors enable */
#define CpCtre      (1<<28)     /* TRE: TEX remap enable */
/*e: [[CpCONTROL.CpMainctl]] other cases(arm) */
/*e: type CpCONTROL CpMainctl(arm) */
/*s: type CpCONTROL CpAuxctl(arm) */
/*
 * CpCONTROL: op1==0, CRm==0, op2==CpAuxctl.
 * Auxiliary control register on cortex at least.
 */
#define CpACcachenopipe     (1<<20) /* don't pipeline cache maint. */
#define CpACcp15serial      (1<<18) /* serialise CP1[45] ops. */
#define CpACcp15waitidle    (1<<17) /* CP1[45] wait-on-idle */
#define CpACcp15pipeflush   (1<<16) /* CP1[45] flush pipeline */
#define CpACneonissue1      (1<<12) /* neon single issue */
#define CpACldstissue1      (1<<11) /* force single issue ld, st */
#define CpACissue1      (1<<10) /* force single issue */
#define CpACnobsm       (1<<7)  /* no branch size mispredicts */
#define CpACibe         (1<<6)  /* cp15 invalidate & btb enable */
#define CpACl1neon      (1<<5)  /* cache neon (FP) data in L1 cache */
#define CpACasa         (1<<4)  /* enable speculative accesses */
#define CpACl1pe        (1<<3)  /* l1 cache parity enable */
#define CpACl2en        (1<<1)  /* l2 cache enable; default 1 */

/* cortex-a7 and cortex-a9 */
#define CpACsmp         (1<<6)  /* SMP l1 caches coherence; needed for ldrex/strex */
#define CpACl1pctl      (3<<13) /* l1 prefetch control */
/*e: type CpCONTROL CpAuxctl(arm) */
/*s: type CpCONTROL secondary registers(arm) */
/*
 * CpCONTROL Secondary (CRm) registers and opcode2 fields.
 */
#define CpCONTROLscr    1
/*e: type CpCONTROL secondary registers(arm) */
/*s: type CpCONTROL opcode2 bis(arm) */
#define CpSCRscr    0
/*e: type CpCONTROL opcode2 bis(arm) */

/*s: type CpTTB(arm) */
/*
 * CpTTB op1==0, Crm==0 opcode2 values.
 */
#define CpTTB0      0
#define CpTTB1      1           /* cortex */
#define CpTTBctl    2           /* cortex */
/*e: type CpTTB(arm) */

/*s: type CpFSR(arm) */
/*
 * CpFSR opcode2 values.
 */
#define CpFSRdata   0           /* armv6, armv7 */
#define CpFSRinst   1           /* armv6, armv7 */
/*e: type CpFSR(arm) */

/*s: type CpCACHE secondary registers(arm) */
/*
 * CpCACHE Secondary (CRm) registers and opcode2 fields.  op1==0.
 * In ARM-speak, 'flush' means invalidate and 'clean' means writeback.
 */
#define CpCACHEintr 0           /* interrupt (op2==4) */
#define CpCACHEisi  1           /* inner-sharable I cache (v7) */
#define CpCACHEpaddr    4           /* 0: phys. addr (cortex) */
#define CpCACHEinvi 5           /* instruction, branch table */
#define CpCACHEinvd 6           /* data or unified */
#define CpCACHEinvu 7           /* unified (not on cortex) */
#define CpCACHEva2pa    8           /* va -> pa translation (cortex) */
#define CpCACHEwb   10          /* writeback to PoC */
#define CpCACHEwbu  11          /* writeback to PoU */
#define CpCACHEwbi  14          /* writeback+invalidate (to PoC) */
/*e: type CpCACHE secondary registers(arm) */
/*s: type CpCACHE opcode2(arm) */
#define CpCACHEall  0           /* entire (not for invd nor wb(i) on cortex) */
#define CpCACHEse   1           /* single entry */
#define CpCACHEsi   2           /* set/index (set/way) */
#define CpCACHEtest 3           /* test loop */
#define CpCACHEwait 4           /* wait (prefetch flush on cortex) */
#define CpCACHEdmbarr   5           /* wb only (cortex) */
#define CpCACHEflushbtc 6           /* flush branch-target cache (cortex) */
#define CpCACHEflushbtse 7          /* ⋯ or just one entry in it (cortex) */
/*e: type CpCACHE opcode2(arm) */

/*s: type CpTLB secondary registers(arm) */
/*
 * CpTLB Secondary (CRm) registers and opcode2 fields.
 */
#define CpTLBinvi   5           /* instruction */
#define CpTLBinvd   6           /* data */
#define CpTLBinvu   7           /* unified */
/*e: type CpTLB secondary registers(arm) */
/*s: type CpTLB opcode2(arm) */
#define CpTLBinv    0           /* invalidate all */
#define CpTLBinvse  1           /* invalidate single entry */
#define CpTBLasid   2           /* by ASID (cortex) */
/*e: type CpTLB opcode2(arm) */

/*s: type CpCLD secondary registers(arm) */
/*
 * CpCLD Secondary (CRm) registers and opcode2 fields for op1==0. (cortex)
 */
#define CpCLDena    12          /* enables */
#define CpCLDcyc    13          /* cycle counter */
#define CpCLDuser   14          /* user enable */
/*e: type CpCLD secondary registers(arm) */
/*s: type CpCLD opcode2(arm) */
#define CpCLDenapmnc    0
#define CpCLDenacyc 1
/*e: type CpCLD opcode2(arm) */

/*s: type CpTIMER(arm) */
/*
 * CpTIMER op1==0 Crm and opcode2 registers (cortex-a7)
 */
#define CpTIMERcntfrq   0
#define CpTIMERphys     2

#define CpTIMERphysval  0
#define CpTIMERphysctl  1
/*e: type CpTIMER(arm) */

/*s: type CpSPM secondary registers(arm) */
/*
 * CpSPM Secondary (CRm) registers and opcode2 fields (armv6)
 */
#define CpSPMperf   12          /* various counters */
/*e: type CpSPM secondary registers(arm) */
/*s: type CpSPM opcode2(arm) */
#define CpSPMctl    0           /* performance monitor control */
#define CpSPMcyc    1           /* cycle counter register */
/*e: type CpSPM opcode2(arm) */


/*
 * CpCACHERANGE opcode2 fields for MCRR instruction (armv6)
 */
#define CpCACHERANGEinvi    5       /* invalidate instruction  */
#define CpCACHERANGEinvd    6       /* invalidate data */
#define CpCACHERANGEdwb     12      /* writeback */
#define CpCACHERANGEdwbi    14      /* writeback+invalidate */

/*
 * CpTTB cache control bits
 */
#define CpTTBnos    (1<<5)  /* only Inner cache shareable */
#define CpTTBinc    (0<<0|0<<6) /* inner non-cacheable */
#define CpTTBiwba   (0<<0|1<<6) /* inner write-back write-allocate */
#define CpTTBiwt    (1<<0|0<<6) /* inner write-through */
#define CpTTBiwb    (1<<0|1<<6) /* inner write-back no write-allocate */
#define CpTTBonc    (0<<3)  /* outer non-cacheable */
#define CpTTBowba   (1<<3)  /* outer write-back write-allocate */
#define CpTTBowt    (2<<3)  /* outer write-through */
#define CpTTBowb    (3<<3)  /* outer write-back no write-allocate */
#define CpTTBs  (1<<1)  /* page table in shareable memory */
#define CpTTBbase   ~0x7F       /* mask off control bits */

/*
 * MMU page table entries.
 */
/*s: constant Mbz(arm) */
/* Mbz (0x10) bit is implementation-defined and must be 0 on the cortex. */
#define Mbz     (0<<4)
/*e: constant Mbz(arm) */

#define Fault       0x00000000      /* L[12] pte: unmapped */

/*s: type PageDirGranularity(arm) */
#define Coarse      (Mbz|1)         /* L1 */
#define Section     (Mbz|2)         /* L1 1MB */
#define Fine        (Mbz|3)         /* L1 */
/*e: type PageDirGranularity(arm) */
/*s: type PageTableGranularity(arm) */
#define Large       0x00000001      /* L2 64KB */
#define Small       0x00000002      /* L2 4KB */
#define Tiny        0x00000003      /* L2 1KB: not in v7 */
/*e: type PageTableGranularity(arm) */

/*s: type PageTableEntryAttribute1(arm) */
#define Buffered    0x00000004      /* L[12]: write-back not -thru */
#define Cached      0x00000008      /* L[12] */
/*e: type PageTableEntryAttribute1(arm) */

#define Dom0        0

/*s: type PageTableEntryAttribute2(arm) */
#define L1wralloc   (1<<12)         /* L1 TEX */
#define L1sharable  (1<<16)
#define L2wralloc   (1<<6)          /* L2 TEX (small pages) */
#define L2sharable  (1<<10)
/*e: type PageTableEntryAttribute2(arm) */

/*s: type PageTableEntryAttribute3(arm) */
#define Noaccess    0       /* AP, DAC */
#define Krw     1           /* AP */
/* armv7 deprecates AP[2] == 1 & AP[1:0] == 2 (Uro), prefers 3 (new in v7) */
#define Uro     2           /* AP */
#define Urw     3           /* AP */
/*e: type PageTableEntryAttribute3(arm) */

/*s: type DAC(arm) */
#define Client      1           /* DAC */
#define Manager     3           /* DAC */
/*e: type DAC(arm) */

#define F(v, o, w)  (((v) & ((1<<(w))-1))<<(o))

#define AP(n, v)    F((v), ((n)*2)+4, 2)
#define L1AP(ap)    (AP(3, (ap)))
/* L2AP differs between armv6 and armv7 -- see l2ap in arch*.c */
#define DAC(n, v)   F((v), (n)*2, 2)

/*s: constant HVECTORS(arm) */
#define HVECTORS    0xffff0000
/*e: constant HVECTORS(arm) */

/*e: arch/arm/arm.h */
