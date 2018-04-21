/*s: include/obj/common.out.h */

// The entities below were originally in some xxx/y.out.h, but were always
// the same in all architecture, hence the factorization below.

/*s: constant [[NSYM]] */
#define	NSYM	50
/*e: constant [[NSYM]] */

/*s: struct [[ieee]] */
/*
 * this is the simulated IEEE floating point
 */
struct	ieee
{
    long	l;	/* contains ls-man	0xffffffff */
    long	h;	/* contains sign	0x80000000
                    exp		0x7ff00000
                    ms-man	0x000fffff */
};
/*e: struct [[ieee]] */
typedef	struct ieee Ieee;
/*e: include/obj/common.out.h */
