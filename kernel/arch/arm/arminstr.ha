/*s: arch/arm/arminstr.ha */
/*
 * armv6/v7 machine assist, definitions
 *
 * loader uses R11 as scratch.
 */

/*s: macro PADDR bis(arm) */
#define PADDR(va)   (((va) & ~KSEGM))
/*e: macro PADDR bis(arm) */
/*s: macro L1X bis(arm) */
#define L1X(va)     (((((va))>>20) & 0x0fff)<<2)
/*e: macro L1X bis(arm) */

/*
 * new instructions
 */
/*s: instruction ISB(arm) */
#define ISB \
    MOVW    $0, R0; \
    MCR CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvi), CpCACHEwait
/*e: instruction ISB(arm) */
/*s: instruction DSB(arm) */
#define DSB \
    MOVW    $0, R0; \
    MCR CpSC, 0, R0, C(CpCACHE), C(CpCACHEwb), CpCACHEwait
/*e: instruction DSB(arm) */
/*s: instruction BARRIERS(arm) */
#define BARRIERS    ISB; DSB
/*e: instruction BARRIERS(arm) */

/*s: instruction MCRR(arm) */
#define MCRR(coproc, op, rd, rn, crm) \
    WORD $(0xec400000|(rn)<<16|(rd)<<12|(coproc)<<8|(op)<<4|(crm))
/*e: instruction MCRR(arm) */
/*s: instruction MRRC(arm) */
#define MRRC(coproc, op, rd, rn, crm) \
    WORD $(0xec500000|(rn)<<16|(rd)<<12|(coproc)<<8|(op)<<4|(crm))
/*e: instruction MRRC(arm) */

/*s: instruction LDREX(arm) */
#define LDREX(fp,t)   WORD $(0xe<<28|0x01900f9f | (fp)<<16 | (t)<<12)
/*e: instruction LDREX(arm) */
/*s: instruction STREX(arm) */
/* `The order of operands is from left to right in dataflow order' - asm man */
#define STREX(f,tp,r) WORD $(0xe<<28|0x01800f90 | (tp)<<16 | (r)<<12 | (f)<<0)
/*e: instruction STREX(arm) */
/*s: instruction CLREX(arm) */
#define CLREX   WORD    $0xf57ff01f
/*e: instruction CLREX(arm) */

/*s: instruction CPSID(arm) */
#define CPSID   WORD    $0xf10c0080 /* intr disable: sets I bit */
/*e: instruction CPSID(arm) */

/*s: instruction CPUID(arm) */
/*
 * get cpu id, or zero if armv6
 */
#define CPUID(r) \
    MRC CpSC, 0, r, C(CpID), C(CpIDfeat), 7; \
    CMP $0, r; \
    B.EQ    2(PC); \
    MRC CpSC, 0, r, C(CpID), C(CpIDidct), CpIDmpid; \
    AND.S   $(MAXCPUS-1), r
/*e: instruction CPUID(arm) */
/*e: arch/arm/arminstr.ha */
