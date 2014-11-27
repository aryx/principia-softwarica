/*s: include/draw.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"

// forward decls
typedef struct	Cachefont Cachefont;
typedef struct	Cacheinfo Cacheinfo;
typedef struct	Cachesubf Cachesubf;
typedef struct	Display Display;
typedef struct	Font Font;
typedef struct	Fontchar Fontchar;
typedef struct	Image Image;
typedef struct	Mouse Mouse;
typedef struct	Point Point;
typedef struct	Rectangle Rectangle;
typedef struct	RGB RGB;
typedef struct	Screen Screen;
typedef struct	Subfont Subfont;

#pragma incomplete Mouse

// dumpers
#pragma varargck	type	"R"	Rectangle
#pragma varargck	type	"P"	Point
extern	int	Rfmt(Fmt*);
extern	int	Pfmt(Fmt*);

/*s: enum _anon_ */
enum
{
    DOpaque			= 0xFFFFFFFF,
    DTransparent	= 0x00000000,/* only useful for allocimage, memfillcolor */

    DBlack		= 0x000000FF,
    DWhite		= 0xFFFFFFFF,
    DRed		= 0xFF0000FF,
    DGreen		= 0x00FF00FF,
    DBlue		= 0x0000FFFF,
    DCyan		= 0x00FFFFFF,
    DMagenta	= 0xFF00FFFF,
    DYellow		= 0xFFFF00FF,

    DPaleyellow	= 0xFFFFAAFF,
    DDarkyellow	= 0xEEEE9EFF,
    DDarkgreen	= 0x448844FF,
    DPalegreen	= 0xAAFFAAFF,
    DMedgreen	= 0x88CC88FF,
    DDarkblue	= 0x000055FF,
    DPalebluegreen= 0xAAFFFFFF,
    DPaleblue	= 0x0000BBFF,
    DBluegreen	= 0x008888FF,
    DGreygreen	= 0x55AAAAFF,
    DPalegreygreen	= 0x9EEEEEFF,
    DYellowgreen	= 0x99994CFF,
    DMedblue		= 0x000099FF,
    DGreyblue		= 0x005DBBFF,
    DPalegreyblue	= 0x4993DDFF,
    DPurpleblue		= 0x8888CCFF,

    DNotacolor	= 0xFFFFFF00, // Alpha = 0
    DNofill	= DNotacolor,
    
};
/*e: enum _anon_ */

/*s: enum _anon_ (include/draw.h) */
enum
{
    /*s: constant ICOSSCALE */
    ICOSSCALE	= 1024,
    /*e: constant ICOSSCALE */
    /*s: constant Borderwidth */
    Borderwidth =	4,
    /*e: constant Borderwidth */
};
/*e: enum _anon_ (include/draw.h) */

/*s: enum _anon_ (include/draw.h)2 */
enum
{
    /* refresh methods */
    Refbackup	= 0,
    Refnone		= 1,
    Refmesg		= 2 // incomplete apparently
};
/*e: enum _anon_ (include/draw.h)2 */

/*s: enum _anon_ (include/draw.h)3 */
enum
{
    /* line ends */
    Endsquare	= 0,
    Enddisc		= 1,
    Endarrow	= 2,

    Endmask		= 0x1F
};
/*e: enum _anon_ (include/draw.h)3 */

/*s: function ARROW */
#define	ARROW(a, b, c)	(Endarrow|((a)<<5)|((b)<<14)|((c)<<23))
/*e: function ARROW */

/*s: enum drawop */
enum drawop
{
    /* Porter-Duff compositing operators */
    Clear	= 0,

    SinD	= 8,
    DinS	= 4,
    SoutD	= 2,
    DoutS	= 1,

    S		= SinD|SoutD,
    SoverD	= SinD|SoutD|DoutS,
    SatopD	= SinD|DoutS,
    SxorD	= SoutD|DoutS,

    D		= DinS|DoutS,
    DoverS	= DinS|DoutS|SoutD,
    DatopS	= DinS|SoutD,
    DxorS	= DoutS|SoutD,	/* == SxorD */

    Ncomp = 12,
};
/*e: enum drawop */
typedef enum drawop Drawop;

/*s: enum _anon_ (include/draw.h)4 */
/*
 * image channel descriptors 
 */
enum {
    CRed = 0,
    CGreen,
    CBlue,

    CGrey,
    CAlpha,
    CMap,
    CIgnore,

    NChan,
};
/*e: enum _anon_ (include/draw.h)4 */

/*s: function __DC */
#define __DC(type, nbits)	((((type)&15)<<4)|((nbits)&15))
/*e: function __DC */
/*s: function CHAN1 */
#define CHAN1(a,b)	__DC(a,b)
/*e: function CHAN1 */
/*s: function CHAN2 */
#define CHAN2(a,b,c,d)	(CHAN1((a),(b))<<8|__DC((c),(d)))
/*e: function CHAN2 */
/*s: function CHAN3 */
#define CHAN3(a,b,c,d,e,f)	(CHAN2((a),(b),(c),(d))<<8|__DC((e),(f)))
/*e: function CHAN3 */
/*s: function CHAN4 */
#define CHAN4(a,b,c,d,e,f,g,h)	(CHAN3((a),(b),(c),(d),(e),(f))<<8|__DC((g),(h)))
/*e: function CHAN4 */

/*s: function NBITS */
#define NBITS(c) ((c)&15)
/*e: function NBITS */
/*s: function TYPE */
#define TYPE(c) (((c)>>4)&15)
/*e: function TYPE */

/*s: enum _anon_ (include/draw.h)5 */
enum {
    GREY1	= CHAN1(CGrey, 1),
    GREY2	= CHAN1(CGrey, 2),
    GREY4	= CHAN1(CGrey, 4),
    GREY8	= CHAN1(CGrey, 8),

    CMAP8	= CHAN1(CMap, 8),

    RGB15	= CHAN4(CIgnore, 1, CRed, 5, CGreen, 5, CBlue, 5),
    RGB16	= CHAN3(CRed, 5, CGreen, 6, CBlue, 5),
    RGB24	= CHAN3(CRed, 8, CGreen, 8, CBlue, 8),
    BGR24	= CHAN3(CBlue, 8, CGreen, 8, CRed, 8),

    RGBA32	= CHAN4(CRed, 8, CGreen, 8, CBlue, 8, CAlpha, 8), // classic one?
    ARGB32	= CHAN4(CAlpha, 8, CRed, 8, CGreen, 8, CBlue, 8),/* stupid VGAs */
    ABGR32	= CHAN4(CAlpha, 8, CBlue, 8, CGreen, 8, CRed, 8),

    XRGB32	= CHAN4(CIgnore, 8, CRed, 8, CGreen, 8, CBlue, 8),
    XBGR32	= CHAN4(CIgnore, 8, CBlue, 8, CGreen, 8, CRed, 8),
};
/*e: enum _anon_ (include/draw.h)5 */

extern	int		chantodepth(ulong);

extern	char*	chantostr(char*, ulong);
extern	ulong	strtochan(char*);

/*s: struct Point */
struct	Point
{
    int	x;
    int	y;
};
/*e: struct Point */

/*s: struct Rectangle */
struct Rectangle
{
    Point	min;
    Point	max;
};
/*e: struct Rectangle */

/*s: struct Screen */
struct Screen
{
    Display	*display;	/* display holding data */
    int		id;			/* id of system-held Screen */
    Image	*image;		/* unused; for reference only */
    Image	*fill;		/* color to paint behind windows */
};
/*e: struct Screen */

/*s: struct Display */
struct Display
{
    // ref_own<Image>
    Image	*image;

    /*s: [[Display]] devdraw connection fields */
    int		dirno; // /dev/draw/x
    fdt		ctlfd; // /dev/draw/new
    fdt		fd;    // /dev/draw/x/data
    /*x: [[Display]] devdraw connection fields */
    char	*devdir; // /dev in general
    char	*windir; // /dev in general
    /*x: [[Display]] devdraw connection fields */
    fdt		reffd; // /dev/draw/x/refresh
    /*e: [[Display]] devdraw connection fields */
    /*s: [[Display]] buf fields */
    // drawing operatings to write in /dev/draw/x/data until flush
    // array<byte>
    byte	*buf;
    int		bufsize;
    // index in Display.buf array
    byte	*bufp;
    /*e: [[Display]] buf fields */
    /*s: [[Display]] basic images fields */
    Image	*white;
    Image	*black;

    Image	*opaque;
    Image	*transparent;
    /*e: [[Display]] basic images fields */

    /*s: [[Display]] other fields */
    QLock	qlock;
    int		locking;	/*program is using lockdisplay */
    /*x: [[Display]] other fields */
    int		local;
    char	oldlabel[64];
    ulong	dataqid;
    /*x: [[Display]] other fields */
    bool	_isnewdisplay;
    /*x: [[Display]] other fields */
    int		imageid;
    /*x: [[Display]] other fields */
    Font	*defaultfont;
    Subfont	*defaultsubfont;
    /*x: [[Display]] other fields */
    // list<ref<Window>> (next = Image.next)
    Image	*windows;
    /*x: [[Display]] other fields */
    Image	*screenimage; // ???
    /*x: [[Display]] other fields */
    void	(*error)(Display*, char*);
    /*e: [[Display]] other fields */
};
/*e: struct Display */

/*s: struct Image */
struct Image
{
    // ref<Display>
    Display		*display;	/* display holding data */
    int			id;		/* id of system-held Image */

    Rectangle	r;		/* rectangle in data area, local coords */
    Rectangle 	clipr;	/* clipping region */
    // bitset<enum<fxxx>>    
    int			repl;		/* flag: data replicates to tile clipr */

    ulong		chan;
    int			depth;		/* number of bits per pixel */

    /*s: [[Image]] other fields */
    Screen		*screen;	/* 0 if not a window */
    /*e: [[Image]] other fields */
    // Extra
    /*s: [[Image]] extra fields */
    Image		*next;	/* next in list of windows */
    /*e: [[Image]] extra fields */
};
/*e: struct Image */

/*s: struct RGB */
struct RGB
{
    ulong	red;
    ulong	green;
    ulong	blue;
};
/*e: struct RGB */

/*s: struct Fontchar */
struct	Fontchar
{
    int		x;		/* left edge of bits */
    uchar		top;		/* first non-zero scan-line */
    uchar		bottom;		/* last non-zero scan-line + 1 */
    char		left;		/* offset of baseline */
    uchar		width;		/* width of baseline */
};
/*e: struct Fontchar */

/*s: struct Subfont */
/*
 * Subfonts
 *
 * given char c, Subfont *f, Fontchar *i, and Point p, one says
 *	i = f->info+c;
 *	draw(b, Rect(p.x+i->left, p.y+i->top,
 *		p.x+i->left+((i+1)->x-i->x), p.y+i->bottom),
 *		color, f->bits, Pt(i->x, i->top));
 *	p.x += i->width;
 * to draw characters in the specified color (itself an Image) in Image b.
 */
struct	Subfont
{
    char		*name;
    short		n;		/* number of chars in font */
    uchar		height;		/* height of image */
    char		ascent;		/* top of image to baseline */

    Fontchar 	*info;		/* n+1 character descriptors */
    Image		*bits;		/* of font */
    int		ref;
};
/*e: struct Subfont */

/*s: enum _anon_ (include/draw.h)6 */
enum
{
    /* starting values */
    LOG2NFCACHE =	6,
    NFCACHE =	(1<<LOG2NFCACHE),	/* #chars cached */
    NFLOOK =	5,			/* #chars to scan in cache */
    NFSUBF =	2,			/* #subfonts to cache */
    /* max value */
    MAXFCACHE =	1024+NFLOOK,		/* upper limit */
    MAXSUBF =	50,			/* generous upper limit */
    /* deltas */
    DSUBF = 	4,
    /* expiry ages */
    SUBFAGE	=	10000,
    CACHEAGE =	10000
};
/*e: enum _anon_ (include/draw.h)6 */

/*s: struct Cachefont */
struct Cachefont
{
    Rune		min;	/* lowest rune value to be taken from subfont */
    Rune		max;	/* highest rune value+1 to be taken from subfont */
    int		offset;	/* position in subfont of character at min */
    char		*name;			/* stored in font */
    char		*subfontname;		/* to access subfont */
};
/*e: struct Cachefont */

/*s: struct Cacheinfo */
struct Cacheinfo
{
    ushort		x;		/* left edge of bits */
    uchar		width;		/* width of baseline */
    schar		left;		/* offset of baseline */
    Rune		value;	/* value of character at this slot in cache */
    ushort		age;
};
/*e: struct Cacheinfo */

/*s: struct Cachesubf */
struct Cachesubf
{
    ulong		age;	/* for replacement */
    Cachefont	*cf;	/* font info that owns us */
    Subfont		*f;	/* attached subfont */
};
/*e: struct Cachesubf */

/*s: struct Font */
struct Font
{
    char		*name;
    Display		*display;

    short		height;	/* max height of image, interline spacing */
    short		ascent;	/* top of image to baseline */
    short		width;	/* widest so far; used in caching only */	

    short		nsub;	/* number of subfonts */
    ulong		age;	/* increasing counter; used for LRU */
    int		maxdepth;	/* maximum depth of all loaded subfonts */
    int		ncache;	/* size of cache */
    int		nsubf;	/* size of subfont list */

    Cacheinfo	*cache;
    Cachesubf	*subf;
    Cachefont	**sub;	/* as read from file */
    Image		*cacheimage;
};
/*e: struct Font */

/*s: function Dx */
#define	Dx(r)	((r).max.x-(r).min.x)
/*e: function Dx */
/*s: function Dy */
#define	Dy(r)	((r).max.y-(r).min.y)
/*e: function Dy */

/*
 * Image management
 */
extern int	initdraw(void(*)(Display*, char*), char*, char*);
extern int	geninitdraw(char*, void(*)(Display*, char*), char*, char*, char*, int);
extern Display*	initdisplay(char*, char*, void(*)(Display*, char*));
extern void		closedisplay(Display*);

extern int		flushimage(Display*, bool);

extern Image*	allocimage(Display*, Rectangle, ulong, int, ulong);
extern int		freeimage(Image*);
extern Image* 	allocimagemix(Display*, ulong, ulong);

extern int		loadimage(Image*, Rectangle, uchar*, int);
extern int		unloadimage(Image*, Rectangle, uchar*, int);
extern Image* 	readimage(Display*, int, int);
extern int		writeimage(int, Image*, int);

// compressed variants
extern int		cloadimage(Image*, Rectangle, uchar*, int);
extern Image* 	creadimage(Display*, int, int);

extern Image*	namedimage(Display*, char*);
extern int		nameimage(Image*, char*, int);

extern void	drawerror(Display*, char*);

extern int	bytesperline(Rectangle, int);
extern int	wordsperline(Rectangle, int);

// why not in Windows section?
extern int	newwindow(char*);
extern int	getwindow(Display*, int);
extern int	gengetwindow(Display*, char*, Image**, Screen**, int);


/*
 * Colors
 */
extern	void	readcolmap(Display*, RGB*);
extern	void	writecolmap(Display*, RGB*);
extern	ulong	setalpha(ulong, uchar);

/*
 * Windows
 */
extern Screen*	allocscreen(Image*, Image*, int);
extern int		freescreen(Screen*);
extern Screen*	publicscreen(Display*, int, ulong);

extern Image*	allocwindow(Screen*, Rectangle, int, ulong);
extern int	originwindow(Image*, Point, Point);
extern void	bottomnwindows(Image**, int);
extern void	bottomwindow(Image*);
extern void	topnwindows(Image**, int);
extern void	topwindow(Image*);

/*
 * Geometry
 */
extern Point	Pt(int, int);
extern Point	addpt(Point, Point);
extern Point	subpt(Point, Point);
extern Point	divpt(Point, int);
extern Point	mulpt(Point, int);
extern int		eqpt(Point, Point);

extern Rectangle	Rect(int, int, int, int);
extern Rectangle	Rpt(Point, Point);
extern int			eqrect(Rectangle, Rectangle);
extern Rectangle	insetrect(Rectangle, int);
extern Rectangle	rectaddpt(Rectangle, Point);
extern Rectangle	rectsubpt(Rectangle, Point);
extern Rectangle	canonrect(Rectangle);
extern int		rectXrect(Rectangle, Rectangle);
extern int		rectinrect(Rectangle, Rectangle);
extern void		combinerect(Rectangle*, Rectangle);
extern int		rectclip(Rectangle*, Rectangle);
extern int		ptinrect(Point, Rectangle);

extern void		replclipr(Image*, int, Rectangle);
extern int		drawreplxy(int, int, int);	/* used to be drawsetxy */
extern Point	drawrepl(Rectangle, Point);
extern int		rgb2cmap(int, int, int);
extern int		cmap2rgb(int);
extern int		cmap2rgba(int);

extern void		icossin(int, int*, int*);
extern void		icossin2(int, int, int*, int*);

/*
 * Graphics
 */
extern void	draw(Image*, Rectangle, Image*, Image*, Point);
extern void	drawop(Image*, Rectangle, Image*, Image*, Point, Drawop);
extern void	gendraw(Image*, Rectangle, Image*, Point, Image*, Point);
extern void	gendrawop(Image*, Rectangle, Image*, Point, Image*, Point, Drawop);

extern void	border(Image*, Rectangle, int, Image*, Point);
extern void	borderop(Image*, Rectangle, int, Image*, Point, Drawop);

extern void	line(Image*, Point, Point, int, int, int, Image*, Point);
extern void	lineop(Image*, Point, Point, int, int, int, Image*, Point, Drawop);

extern void	poly(Image*, Point*, int, int, int, int, Image*, Point);
extern void	polyop(Image*, Point*, int, int, int, int, Image*, Point, Drawop);
extern void	fillpoly(Image*, Point*, int, int, Image*, Point);
extern void	fillpolyop(Image*, Point*, int, int, Image*, Point, Drawop);

extern void	ellipse(Image*, Point, int, int, int, Image*, Point);
extern void	ellipseop(Image*, Point, int, int, int, Image*, Point, Drawop);
extern void	fillellipse(Image*, Point, int, int, Image*, Point);
extern void	fillellipseop(Image*, Point, int, int, Image*, Point, Drawop);

extern void	arc(Image*, Point, int, int, int, Image*, Point, int, int);
extern void	arcop(Image*, Point, int, int, int, Image*, Point, int, int, Drawop);
extern void	fillarc(Image*, Point, int, int, Image*, Point, int, int);
extern void	fillarcop(Image*, Point, int, int, Image*, Point, int, int, Drawop);

extern int	bezier(Image*, Point, Point, Point, Point, int, int, int, Image*, Point);
extern int	bezierop(Image*, Point, Point, Point, Point, int, int, int, Image*, Point, Drawop);
extern int	bezspline(Image*, Point*, int, int, int, int, Image*, Point);
extern int	bezsplineop(Image*, Point*, int, int, int, int, Image*, Point, Drawop);
extern int	bezsplinepts(Point*, int, Point**);
extern int	fillbezier(Image*, Point, Point, Point, Point, int, Image*, Point);
extern int	fillbezierop(Image*, Point, Point, Point, Point, int, Image*, Point, Drawop);
extern int	fillbezspline(Image*, Point*, int, int, Image*, Point);
extern int	fillbezsplineop(Image*, Point*, int, int, Image*, Point, Drawop);


extern Point	string(Image*, Point, Image*, Point, Font*, char*);
extern Point	stringop(Image*, Point, Image*, Point, Font*, char*, Drawop);
extern Point	stringn(Image*, Point, Image*, Point, Font*, char*, int);
extern Point	stringnop(Image*, Point, Image*, Point, Font*, char*, int, Drawop);
extern Point	runestring(Image*, Point, Image*, Point, Font*, Rune*);
extern Point	runestringop(Image*, Point, Image*, Point, Font*, Rune*, Drawop);
extern Point	runestringn(Image*, Point, Image*, Point, Font*, Rune*, int);
extern Point	runestringnop(Image*, Point, Image*, Point, Font*, Rune*, int, Drawop);
extern Point	stringbg(Image*, Point, Image*, Point, Font*, char*, Image*, Point);
extern Point	stringbgop(Image*, Point, Image*, Point, Font*, char*, Image*, Point, Drawop);
extern Point	stringnbg(Image*, Point, Image*, Point, Font*, char*, int, Image*, Point);
extern Point	stringnbgop(Image*, Point, Image*, Point, Font*, char*, int, Image*, Point, Drawop);
extern Point	runestringbg(Image*, Point, Image*, Point, Font*, Rune*, Image*, Point);
extern Point	runestringbgop(Image*, Point, Image*, Point, Font*, Rune*, Image*, Point, Drawop);
extern Point	runestringnbg(Image*, Point, Image*, Point, Font*, Rune*, int, Image*, Point);
extern Point	runestringnbgop(Image*, Point, Image*, Point, Font*, Rune*, int, Image*, Point, Drawop);


extern Point	stringsubfont(Image*, Point, Image*, Subfont*, char*);


/*
 * Font management
 */
extern Font*	openfont(Display*, char*);
extern void		freefont(Font*);

extern Font*	buildfont(Display*, char*, char*);
extern Font*	mkfont(Subfont*, Rune);

extern Subfont*	allocsubfont(char*, int, int, int, Fontchar*, Image*);
extern Subfont*	lookupsubfont(Display*, char*);
extern void	installsubfont(char*, Subfont*);
extern void	freesubfont(Subfont*);
extern Subfont*	readsubfont(Display*, char*, int, int);
extern Subfont*	readsubfonti(Display*, char*, int, Image*, int);
extern int	writesubfont(int, Subfont*);
extern void	uninstallsubfont(Subfont*);
extern char*	subfontname(char*, char*, int);
extern Subfont*	_getsubfont(Display*, char*);

extern Point	stringsize(Font*, char*);
extern int		stringwidth(Font*, char*);
extern int		stringnwidth(Font*, char*, int);
extern Point	runestringsize(Font*, Rune*);
extern int		runestringwidth(Font*, Rune*);
extern int		runestringnwidth(Font*, Rune*, int);

extern int		cachechars(Font*, char**, Rune**, ushort*, int, int*, char**);
extern void		agefont(Font*);
extern void		_unpackinfo(Fontchar*, uchar*, int);
extern Point	strsubfontwidth(Subfont*, char*);
extern int		loadchar(Font*, Rune, Cacheinfo*, int, int, char**);
extern Subfont*	getdefont(Display*);
extern int		drawlsetrefresh(ulong, int, void*, void*);

// seems related to font
extern void		lockdisplay(Display*);
extern void		unlockdisplay(Display*);

/*
 * One of a kind
 */
extern int		mousescrollsize(int);


/*
 * Predefined 
 */
extern	Point		ZP;
extern	Rectangle	ZR;
extern	uchar	defontdata[];
extern	int		sizeofdefont;

/*
 * Set up by initdraw()
 */
extern	Display	*display;
extern	Image	*screen;
extern	Font	*font;

extern	Screen	*_screen;
extern	int		_cursorfd;
extern	bool	_drawdebug;	/* set to true to see errors from flushimage */

// forward decl, could be move to individual files or in a drawimpl.h
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
extern	void	_twiddlecompressed(uchar*, int);
extern	int	_compblocksize(Rectangle, int);

/* XXX backwards helps; should go */
extern	ulong	drawld2chan[];
extern	void	drawsetdebug(int);
/*e: include/draw.h */
