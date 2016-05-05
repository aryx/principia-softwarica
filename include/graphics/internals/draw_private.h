/*s: include/draw_private.h */

/*s: enum misc */
enum
{
    /* starting values */
    /*s: constant NFCACHE */
    LOG2NFCACHE =	6,
    NFCACHE =	(1<<LOG2NFCACHE),	/* #chars cached */
    /*e: constant NFCACHE */
    /*s: constant NFLOOK */
    NFLOOK =	5,			/* #chars to scan in cache */
    /*e: constant NFLOOK */
    /*s: constant NFSUBF */
    NFSUBF =	2,			/* #subfonts to cache */
    /*e: constant NFSUBF */

    /* max value */
    /*s: constant MAXFCACHE */
    MAXFCACHE =	1024+NFLOOK,		/* upper limit */
    /*e: constant MAXFCACHE */
    /*s: constant MAXSUBF */
    MAXSUBF =	50,			/* generous upper limit */
    /*e: constant MAXSUBF */

    /* deltas */
    /*s: constant DSUBF */
    DSUBF = 	4,
    /*e: constant DSUBF */

    /* expiry ages */
    SUBFAGE	=	10000,
    CACHEAGE =	10000
};
/*e: enum misc */

extern int		drawlsetrefresh(ulong, int, void*, void*);

// dead?
extern	int		_cursorfd;

extern	bool	_drawdebug;	/* set to true to see errors from flushimage */

// used also by window.c
extern Image*	_allocimage(Image*, Display*, Rectangle, ulong, int, ulong, int, int);
extern int	    _freeimage1(Image*);

extern	void	_setdrawop(Display*, Drawop);

/*s: constant NMATCH */
/*
 * Compressed image file parameters and helper routines
 */
#define	NMATCH	3		/* shortest match possible */
/*e: constant NMATCH */
/*s: constant NRUN */
#define	NRUN	(NMATCH+31)	/* longest match possible */
/*e: constant NRUN */
/*s: constant NMEM */
#define	NMEM	1024		/* window size */
/*e: constant NMEM */
/*s: constant NDUMP */
#define	NDUMP	128		/* maximum length of dump */
/*e: constant NDUMP */
/*s: constant NCBLOCK */
#define	NCBLOCK	6000		/* size of compressed blocks */
/*e: constant NCBLOCK */

/* XXX backwards helps; should go */
extern	ulong	drawld2chan[];
extern	void	drawsetdebug(bool);

#include <marshal.h>

/*e: include/draw_private.h */
