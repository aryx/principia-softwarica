/*s: include/font.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"
// This file assumes you have included draw.h before.

typedef struct	Cachefont Cachefont;
typedef struct	Cacheinfo Cacheinfo;
typedef struct	Cachesubf Cachesubf;
typedef struct	Fontchar Fontchar;
typedef struct	Subfont Subfont;

// The main type, Font, is defined in draw.h.

/*s: struct Fontchar */
struct	Fontchar
{
    // character coordinates in Subfont.bits
    int		x;		/* left edge of bits */
    uchar	top;		/* first non-zero scan-line */
    uchar	bottom;		/* last non-zero scan-line + 1 */

    // adjustments to make on drawing point coordinates in destination
    schar	left;		/* offset of baseline */
    uchar	width;		/* width of baseline */
};
/*e: struct Fontchar */

/*s: struct Subfont */
/*
 * Subfonts
 *
 * given char c, Subfont *f, Fontchar *i, and Point p, one says
 *	i = f->info + c;
 *	draw(b, Rect(p.x + i->left, p.y + i->top,
 *		p.x + i->left + ((i+1)->x - i->x), p.y + i->bottom),
 *		color, f->bits, Pt(i->x, i->top));
 *	p.x += i->width;
 * to draw characters in the specified color (itself an Image) in Image b.
 */
struct	Subfont
{
    // ref_own<string>
    char		*name;
    // ref_own<Image> ?
    Image		*bits;	/* of font */

    // array<Fontchar> (size = Subfont.n + 1)
    Fontchar 	*info;	/* n+1 character descriptors */
    short		n;		/* number of chars in font */

    /*s: [[Subfont]] other fields */
    uchar		height;		/* height of image */
    char		ascent;		/* top of image to baseline */
    /*e: [[Subfont]] other fields */

    // Extra
    int		ref;
};
/*e: struct Subfont */

/*s: struct Cachefont */
struct Cachefont
{
    Rune		min;	/* lowest rune value to be taken from subfont */
    Rune		max;	/* highest rune value+1 to be taken from subfont */
    // option<int>, None = 0
    int		offset;	/* position in subfont of character at min */

    // ref_own<string>, relative filename
    char		*name;			/* stored in font */
    // option<ref_own<filename>>, absolute filename, computed by subfontname()
    char		*subfontname;		/* to access subfont */

};
/*e: struct Cachefont */

/*s: struct Cacheinfo */
struct Cacheinfo
{
    // the key
    Rune		value;	/* value of character at this slot in cache */

    // the values
    ushort		x;		/* left edge of bits */
    byte		width;		/* width of baseline */
    schar		left;		/* offset of baseline */

    ushort		age;
};
/*e: struct Cacheinfo */

/*s: struct Cachesubf */
struct Cachesubf
{
    // ref<Cachefont>, the key
    Cachefont	*cf;	/* font info that owns us */
    // ref_own<Subfont>, the value
    Subfont		*f;	/* attached subfont */

    ulong		age;	/* for replacement */
};
/*e: struct Cachesubf */

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


// internal to font code
extern Subfont*	allocsubfont(char*, int, int, int, Fontchar*, Image*);
extern void	    freesubfont(Subfont*);
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
extern Point	strsubfontwidth(Subfont*, char*);
extern int		loadchar(Font*, Rune, Cacheinfo*, int, int, char**);
extern Subfont*	getdefont(Display*);

// used also by libmemdraw/
extern void		_unpackinfo(Fontchar*, byte*, int);
extern	byte	defontdata[];
extern	int		sizeofdefont;


/*e: include/font.h */
