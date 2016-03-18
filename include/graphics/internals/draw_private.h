/*s: include/draw_private.h */

/*s: enum _anon_ (include/draw.h)6 */
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
/*e: enum _anon_ (include/draw.h)6 */

// internal to font code
extern Subfont*	allocsubfont(char*, int, int, int, Fontchar*, Image*);
extern void	freesubfont(Subfont*);
extern Subfont*	lookupsubfont(Display*, char*);
extern void	installsubfont(char*, Subfont*);
extern void	uninstallsubfont(Subfont*);
extern Subfont*	readsubfont(Display*, char*, int, int);
extern char*	subfontname(char*, char*, int);
extern Subfont*	_getsubfont(Display*, char*);
// for subfont designers
extern int	writesubfont(int, Subfont*);

extern int		cachechars(Font*, char**, Rune**, ushort*, int, int*, char**);
extern void		agefont(Font*);
extern void		_unpackinfo(Fontchar*, byte*, int);
extern Point	strsubfontwidth(Subfont*, char*);
extern int		loadchar(Font*, Rune, Cacheinfo*, int, int, char**);
extern Subfont*	getdefont(Display*);

extern int		drawlsetrefresh(ulong, int, void*, void*);

extern	byte	defontdata[];
extern	int		sizeofdefont;


// dead?
extern	int		_cursorfd;

extern	bool	_drawdebug;	/* set to true to see errors from flushimage */

// used also by window.c
extern Image*	_allocimage(Image*, Display*, Rectangle, ulong, int, ulong, int, int);
extern int	    _freeimage1(Image*);
extern Image*	_allocwindow(Image*, Screen*, Rectangle, int, ulong);

extern Point	_string(Image*, Point, Image*, Point, Font*, char*, Rune*, int, Rectangle, Image*, Point, Drawop);

extern	void	_setdrawop(Display*, Drawop);

// internals
extern byte*	bufimage(Display*, int);

/*s: function BGSHORT */
#define	BGSHORT(p)		(((p)[0]<<0) | ((p)[1]<<8))
/*e: function BGSHORT */
/*s: function BGLONG */
#define	BGLONG(p)		((BGSHORT(p)<<0) | (BGSHORT(p+2)<<16))
/*e: function BGLONG */
/*s: function BPSHORT */
#define	BPSHORT(p, v)		((p)[0]=(v), (p)[1]=((v)>>8))
/*e: function BPSHORT */
/*s: function BPLONG */
#define	BPLONG(p, v)		(BPSHORT(p, (v)), BPSHORT(p+2, (v)>>16))
/*e: function BPLONG */

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


extern	void _twiddlecompressed(byte*, int);
extern	int	_compblocksize(Rectangle, int);

/* XXX backwards helps; should go */
extern	ulong	drawld2chan[];
extern	void	drawsetdebug(bool);

/*e: include/draw_private.h */
