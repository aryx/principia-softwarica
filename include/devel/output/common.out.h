/*s: include/common.out.h */

/*s: constant NSYM */
#define	NSYM	50
/*e: constant NSYM */

// was originally in a xxx/y.out.h (but was always the same in all archi)
// hence the factorization here
/*
 * this is the simulated IEEE floating point
 */
typedef	struct	ieee	Ieee;
/*s: struct ieee */
struct	ieee
{
    long	l;	/* contains ls-man	0xffffffff */
    long	h;	/* contains sign	0x80000000
                    exp		0x7ff00000
                    ms-man	0x000fffff */
};
/*e: struct ieee */
/*e: include/common.out.h */
