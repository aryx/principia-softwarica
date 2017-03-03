/*s: memory/arm/cache_raspi1.s */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

#define CACHELINESZ 32
                
/*
 * drain write buffer
 * writeback data cache
 */
TEXT cachedwb(SB), 1, $-4
	DSB
	MOVW	$0, R0
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEwb), CpCACHEall
	RET

/*
 * drain write buffer
 * writeback and invalidate data cache
 */
TEXT cachedwbinv(SB), 1, $-4
	DSB
	MOVW	$0, R0
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEwbi), CpCACHEall
	RET

/*
 * cachedwbinvse(va, n)
 *   drain write buffer
 *   writeback and invalidate data cache range [va, va+n)
 */
TEXT cachedwbinvse(SB), 1, $-4
	MOVW	R0, R1		/* DSB clears R0 */
	DSB
	MOVW	n+4(FP), R2
	ADD	R1, R2
	SUB	$1, R2
	BIC	$(CACHELINESZ-1), R1
	BIC	$(CACHELINESZ-1), R2
	MCRR(CpSC, 0, 2, 1, CpCACHERANGEdwbi)
	RET

/*
 * cachedwbse(va, n)
 *   drain write buffer
 *   writeback data cache range [va, va+n)
 */
TEXT cachedwbse(SB), 1, $-4
	MOVW	R0, R1		/* DSB clears R0 */
	DSB
	MOVW	n+4(FP), R2
	ADD	R1, R2
	BIC	$(CACHELINESZ-1), R1
	BIC	$(CACHELINESZ-1), R2
	MCRR(CpSC, 0, 2, 1, CpCACHERANGEdwb)
	RET

/*
 * cachedinvse(va, n)
 *   drain write buffer
 *   invalidate data cache range [va, va+n)
 */
TEXT cachedinvse(SB), 1, $-4
	MOVW	R0, R1		/* DSB clears R0 */
	DSB
	MOVW	n+4(FP), R2
	ADD	R1, R2
	SUB	$1, R2
	BIC	$(CACHELINESZ-1), R1
	BIC	$(CACHELINESZ-1), R2
	MCRR(CpSC, 0, 2, 1, CpCACHERANGEinvd)
	RET

/*
 * drain write buffer and prefetch buffer
 * writeback and invalidate data cache
 * invalidate instruction cache
 */
TEXT cacheuwbinv(SB), 1, $-4
	BARRIERS
	MOVW	$0, R0
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEwbi), CpCACHEall
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvi), CpCACHEall
	RET

/*
 * L2 cache is not enabled
 */
TEXT l2cacheuwbinv(SB), 1, $-4
	RET

/*
 * invalidate instruction cache
 */
TEXT cacheiinv(SB), 1, $-4
	MOVW	$0, R0
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvi), CpCACHEall
	RET
/*e: memory/arm/cache_raspi1.s */
