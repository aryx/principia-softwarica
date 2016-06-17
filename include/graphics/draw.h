/*s: include/draw.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"

typedef struct	Point Point;
typedef struct	Rectangle Rectangle;
typedef struct	Display Display;
typedef struct	Image Image;
typedef struct	Font Font;
typedef struct	RGB RGB;

// defined in other header files
typedef struct	Screen Screen;
typedef struct	Cachefont Cachefont;
typedef struct	Cacheinfo Cacheinfo;
typedef struct	Cachesubf Cachesubf;
typedef struct	Subfont Subfont;
#pragma incomplete Screen
#pragma incomplete Cachefont
#pragma incomplete Cacheinfo
#pragma incomplete Cachesubf
#pragma incomplete Subfont

// temporary hack, because does not have a memfont.h so need to
// disable safe-linking for Fontchar everywhere
typedef struct	Fontchar Fontchar;
#pragma incomplete Fontchar

// TODO why need that?
typedef struct	Mouse Mouse;
#pragma incomplete Mouse

//----------------------------------------------------------------------------
// Data structures and constants
//----------------------------------------------------------------------------

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


/*s: enum Colors */
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

    DNotacolor	= 0xFFFFFF00, // Alpha = 0, transparent white
    DNofill	= DNotacolor,
    
};
/*e: enum Colors */
/*s: type rgba */
typedef ulong rgba;
/*e: type rgba */

/*s: struct RGB */
struct RGB
{
    ulong	red;
    ulong	green;
    ulong	blue;
};
/*e: struct RGB */


/*s: enum ImageChan */
/*
 * image channel descriptors 
 */
// coupling: chantostr and channames
enum ImageChan {
    CRed = 0,
    CGreen,
    CBlue,

    CGrey,
    CAlpha,

    CMap,
    /*s: [[ImageChan]] cases */
    CIgnore,
    /*e: [[ImageChan]] cases */

    NChan,
};
/*e: enum ImageChan */
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
/*s: enum ImageType */
enum ImageType {
    GREY1	= CHAN1(CGrey, 1), // used for masks, black and white
    CMAP8	= CHAN1(CMap, 8), // PC graphics mode by default, 1 byte per pixel
    RGB16	= CHAN3(CRed, 5, CGreen, 6, CBlue, 5), // Raspberry mode by default
    RGBA32	= CHAN4(CRed, 8, CGreen, 8, CBlue, 8, CAlpha, 8), // flexible
    ARGB32	= CHAN4(CAlpha, 8, CRed, 8, CGreen, 8, CBlue, 8),/* stupid VGAs */
    /*s: [[ImageType] cases */
    GREY2	= CHAN1(CGrey, 2),
    GREY4	= CHAN1(CGrey, 4),
    GREY8	= CHAN1(CGrey, 8),

    RGB15	= CHAN4(CIgnore, 1, CRed, 5, CGreen, 5, CBlue, 5),
    RGB24	= CHAN3(CRed, 8, CGreen, 8, CBlue, 8),
    BGR24	= CHAN3(CBlue, 8, CGreen, 8, CRed, 8),
    /*x: [[ImageType] cases */
    ABGR32	= CHAN4(CAlpha, 8, CBlue, 8, CGreen, 8, CRed, 8),

    XRGB32	= CHAN4(CIgnore, 8, CRed, 8, CGreen, 8, CBlue, 8),
    XBGR32	= CHAN4(CIgnore, 8, CBlue, 8, CGreen, 8, CRed, 8),
    /*e: [[ImageType] cases */
};
/*e: enum ImageType */
/*s: type channels */
typedef ulong channels;
/*e: type channels */


/*s: type Errorfn */
typedef void (*Errorfn)(Display*, char*);
/*e: type Errorfn */

/*s: struct Display */
struct Display
{
    // ref_own<Image>, the full screen
    Image	*image;

    /*s: [[Display]] devdraw connection fields */
    int		dirno; // /dev/draw/x
    fdt		ctlfd; // /dev/draw/x/ctl (opened via /dev/draw/new)
    fdt		fd;    // /dev/draw/x/data
    /*x: [[Display]] devdraw connection fields */
    char	*devdir; // /dev in general
    char	*windir; // /dev in general
    /*x: [[Display]] devdraw connection fields */
    fdt		reffd; // /dev/draw/x/refresh
    /*e: [[Display]] devdraw connection fields */
    /*s: [[Display]] buffer fields */
    // drawing operations to write in /dev/draw/x/data until flush
    // array<byte> (length = Display.bufsize)
    byte	*buf;
    int		bufsize;
    // index in Display.buf array
    byte	*bufp;
    /*e: [[Display]] buffer fields */
    /*s: [[Display]] basic images fields */
    // ref_own<Image>
    Image	*white;
    // ref_own<Image>
    Image	*black;

    // ref_own<Image>
    Image	*opaque;
    // ref_own<Image>
    Image	*transparent;
    /*e: [[Display]] basic images fields */
    /*s: [[Display]] font fields */
    Font	*defaultfont;
    /*x: [[Display]] font fields */
    Subfont	*defaultsubfont;
    /*e: [[Display]] font fields */
    /*s: [[Display]] other fields */
    // gensym
    int		imageid;
    /*x: [[Display]] other fields */
    // list<ref<Window>> (next = Image.next)
    Image	*windows;
    /*x: [[Display]] other fields */
    // option<Image>, same than view, the window (or desktop image)
    Image	*screenimage;
    /*x: [[Display]] other fields */
    char	oldlabel[64];
    /*x: [[Display]] other fields */
    Errorfn error;
    /*e: [[Display]] other fields */

    // Extra
    /*s: [[Display]] concurrency fields */
    QLock	qlock;
    /*x: [[Display]] concurrency fields */
    bool		locking;	/*program is using lockdisplay */
    /*e: [[Display]] concurrency fields */
};
/*e: struct Display */

/*s: struct Image */
struct Image
{
    int			id;		/* id of system-held Image */

    Rectangle	r;		/* rectangle in data area, local coords */

    channels		chan; // image format
    // derives from Image.chan
    int			depth;		/* number of bits per pixel */

    Rectangle 	clipr;	/* clipping region */
    bool		repl;	/* flag: data replicates to tile clipr */

    Display		*display;	/* display holding data */

    /*s: [[Image]] layer fields */
    Screen		*screen;	/* nil if not a window */
    /*e: [[Image]] layer fields */

    // Extra
    /*s: [[Image]] extra fields */
    Image		*next;	/* next in list of windows */
    /*e: [[Image]] extra fields */
};
/*e: struct Image */

/*s: struct Font */
struct Font
{
    // ref_own<filename>, // e.g., /lib/font/bit/lucm/latin1.9.font
    char		*name;
    // ref<Display>
    Display		*display;

    short		height;	/* max height of image, interline spacing */
    short		ascent;	/* top of image to baseline */

    /*s: [[Font]] subfont spec fields */
    // array<ref_own<Cachefont>> (length = Font.nsub)
    Cachefont	**sub;	/* as read from file */
    short		nsub;	/* number of subfonts */
    /*e: [[Font]] subfont spec fields */
    /*s: [[Font]] subfont cache fields */
    // growing_array<Cachesubf> (size = Font.nsubf, init = NFSUBF, max = MAXSUBF)
    Cachesubf	*subf;
    int		nsubf;	/* size of subfont list */
    /*e: [[Font]] subfont cache fields */
    /*s: [[Font]] character cache fields */
    // ref_own<Image>, growing image
    Image		*cacheimage;
    /*x: [[Font]] character cache fields */
    ulong		age;	/* increasing counter; used for LRU */
    /*x: [[Font]] character cache fields */
    // growing_hash<Rune, Cacheinfo> (size = Font.ncache, bucketsize = NFLOOK)
    Cacheinfo	*cache;
    int		ncache;	/* size of cache */
    /*e: [[Font]] character cache fields */
    /*s: [[Font]] other fields */
    short		width;	/* widest so far; used in caching only */	
    int		maxdepth;	/* maximum depth of all loaded subfonts */
    /*e: [[Font]] other fields */
};
/*e: struct Font */



/*s: enum constants1 */
enum
{
    /*s: constant ICOSSCALE */
    ICOSSCALE	= 1024,
    /*e: constant ICOSSCALE */
    /*s: constant Borderwidth */
    //coupling: must be equal to Selborder in rio
    Borderwidth =	4,
    /*e: constant Borderwidth */
};
/*e: enum constants1 */

/*s: enum drawop */
enum Drawop
{
    /* Porter-Duff compositing operators */
    Clear	= 0,

    SinD	= 8,
    DinS	= 4,
    SoutD	= 2,
    DoutS	= 1,

    S		= SinD|SoutD,
    SoverD	= SinD|SoutD|DoutS, // classic Source over Destination
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

/*s: enum Refresh */
enum RefreshMethod
{
    /* refresh methods */
    Refbackup	= 0,
    Refnone		= 1,
    Refmesg		= 2 // incomplete apparently
};
/*e: enum Refresh */

/*s: enum Endline */
enum EndLine
{
    /* line ends */
    Endsquare	= 0, // default
    Enddisc		= 1,
    Endarrow	= 2,

    Endmask		= 0x1F
};
/*e: enum Endline */

/*s: function ARROW */
#define	ARROW(a, b, c)	(Endarrow|((a)<<5)|((b)<<14)|((c)<<23))
/*e: function ARROW */

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

/*
 * Set up by initdraw()
 */
extern	Display	*display;
extern	Image	*view; // was called 'screen' before
extern	Font	*font;

/*
 * Predefined 
 */
extern	Point		ZP;
extern	Rectangle	ZR;

//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

//
// Display
//
// This sets the following globals: display, view, font (and screen).
extern int	initdraw(Errorfn, char*, char*);
extern int	geninitdraw(char*, Errorfn, char*, char*, char*, int);
// to call after a resize event
extern int	getwindow(Display*, int);

extern Display*	initdisplay(char*, char*, Errorfn);
extern void		closedisplay(Display*);

extern int		flushimage(Display*, bool);

extern void	drawerror(Display*, char*);

/*
 * Image management
 */
extern Image*	allocimage(Display*, Rectangle, channels, bool, rgba);
extern Image* 	allocimagemix(Display*, rgba, rgba);
extern int		freeimage(Image*);

extern int		loadimage(Image*, Rectangle, byte*, int);
extern int		unloadimage(Image*, Rectangle, byte*, int);
extern Image* 	readimage(Display*, fdt, bool);
extern int		writeimage(fdt, Image*, bool);
// compressed variants
extern int		cloadimage(Image*, Rectangle, byte*, int);
extern Image* 	creadimage(Display*, int, int);

extern Image*	namedimage(Display*, char*);
extern int		nameimage(Image*, char*, bool);

//
// Channels
//
extern	int		chantodepth(channels);
extern	char*	    chantostr(char*, channels);
extern	channels	strtochan(char*);

/*
 * Colors
 */
extern	void	readcolmap(Display*, RGB*);
extern	void	writecolmap(Display*, RGB*);

extern int		rgb2cmap(int, int, int);
extern int		cmap2rgb(int);
extern int		cmap2rgba(int);

/*
 * Geometry
 */
extern Point	Pt(int, int);
extern int		eqpt(Point, Point);

extern Point	addpt(Point, Point);
extern Point	subpt(Point, Point);
extern Point	divpt(Point, int);
extern Point	mulpt(Point, int);

extern Rectangle	Rect(int, int, int, int);
extern Rectangle	Rpt(Point, Point);
extern int			eqrect(Rectangle, Rectangle);

/*s: function Dx */
#define	Dx(r)	((r).max.x-(r).min.x)
/*e: function Dx */
/*s: function Dy */
#define	Dy(r)	((r).max.y-(r).min.y)
/*e: function Dy */

extern Rectangle	rectaddpt(Rectangle, Point);
extern Rectangle	rectsubpt(Rectangle, Point);
extern Rectangle	insetrect(Rectangle, int);
extern Rectangle	canonrect(Rectangle);
extern int		rectXrect(Rectangle, Rectangle);
extern int		rectinrect(Rectangle, Rectangle);
extern void		combinerect(Rectangle*, Rectangle);
extern int		ptinrect(Point, Rectangle);

extern void		icossin(int, int*, int*);
extern void		icossin2(int, int, int*, int*);

// Clipping and replication
extern void		replclipr(Image*, int, Rectangle);
extern int		rectclip(Rectangle*, Rectangle);
extern int		drawreplxy(int, int, int);	/* used to be drawsetxy */
extern Point	drawrepl(Rectangle, Point);

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
extern int	fillbezier(Image*, Point, Point, Point, Point, int, Image*, Point);
extern int	fillbezierop(Image*, Point, Point, Point, Point, int, Image*, Point, Drawop);
extern int	fillbezspline(Image*, Point*, int, int, Image*, Point);
extern int	fillbezsplineop(Image*, Point*, int, int, Image*, Point, Drawop);

extern int	bezsplinepts(Point*, int, Point**);


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


extern Point	stringsize(Font*, char*);
extern int		stringwidth(Font*, char*);
extern int		stringnwidth(Font*, char*, int);

extern Point	runestringsize(Font*, Rune*);
extern int		runestringwidth(Font*, Rune*);
extern int		runestringnwidth(Font*, Rune*, int);


/*
 * Font management
 */
extern Font*	buildfont(Display*, char*, char*);
extern Font*	openfont(Display*, char*);
extern void		freefont(Font*);

/*
 * One of a kind
 */
extern int		mousescrollsize(int);

extern int	bytesperline(Rectangle, int);
extern int	wordsperline(Rectangle, int);

// seems related to font
extern void		lockdisplay(Display*);
extern void		unlockdisplay(Display*);

//
// Dumpers
//
#pragma varargck	type	"R"	Rectangle
#pragma varargck	type	"P"	Point
extern	int	Rfmt(Fmt*);
extern	int	Pfmt(Fmt*);

/*e: include/draw.h */
