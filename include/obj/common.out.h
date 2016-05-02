/*s: include/common.out.h */

/*s: constant NSYM */
#define	NSYM	50
/*e: constant NSYM */

// It was originally in some xxx/y.out.h, but was always the same in 
// all architecture, hence the factorization here.
/*s: struct ieee */
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
/*e: struct ieee */
typedef	struct	ieee	Ieee;
/*e: include/common.out.h */
