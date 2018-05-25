/*s: lib_gui/libpanel/rtext.h */

/*s: constant [[PL_NOPBIT]] */
/*
 * Rtext definitions
 */
#define	PL_NOPBIT	4
/*e: constant [[PL_NOPBIT]] */
/*s: constant [[PL_NARGBIT]] */
#define	PL_NARGBIT	12
/*e: constant [[PL_NARGBIT]] */
/*s: constant [[PL_ARGMASK]] */
#define	PL_ARGMASK	((1<<PL_NARGBIT)-1)
/*e: constant [[PL_ARGMASK]] */
/*s: function [[PL_SPECIAL]] */
#define	PL_SPECIAL(op)	(((-1<<PL_NOPBIT)|op)<<PL_NARGBIT)
/*e: function [[PL_SPECIAL]] */
/*s: function [[PL_OP]] */
#define	PL_OP(t)	((t)&~PL_ARGMASK)
/*e: function [[PL_OP]] */
/*s: function [[PL_ARG]] */
#define	PL_ARG(t)	((t)&PL_ARGMASK)
/*e: function [[PL_ARG]] */
/*s: constant [[PL_TAB]] */
#define	PL_TAB		PL_SPECIAL(0)		/* # of tab stops before text */
/*e: constant [[PL_TAB]] */

void pltabsize(int, int);			/* set min tab and tab size */
/*e: lib_gui/libpanel/rtext.h */
