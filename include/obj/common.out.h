/*s: include/obj/common.out.h */

// The entities below were originally in some xxx/y.out.h, but were always
// the same in all architecture, hence the factorization below.

/*s: constant [[NSYM]] */
#define	NSYM	50
/*e: constant [[NSYM]] */

/*s: struct [[ieee]] */
/*
 * this is the simulated IEEE floating point
 * claude: must be int32, not long: the linkers copy the raw bytes of
 * this struct into the data segment (datblk() D_FCONST), so 64-bit
 * longs corrupt the emitted doubles (caught by tests/s/variants)
 */
struct	ieee
{
    int32	l;	/* contains ls-man	0xffffffff */
    int32	h;	/* contains sign	0x80000000
                    exp		0x7ff00000
                    ms-man	0x000fffff */
};
/*e: struct [[ieee]] */
typedef	struct ieee Ieee;
/*e: include/obj/common.out.h */
