/*s: include/memdraw.h */
#pragma	src	"/sys/src/libmemdraw"
#pragma	lib	"libmemdraw.a"
// This file assumes you have included draw.h before.

typedef struct	Memimage Memimage;
typedef struct	Memdata Memdata;
typedef struct	Memsubfont Memsubfont;
typedef struct	Memcmap Memcmap;
typedef struct	Memdrawparam	Memdrawparam;

// defined in other header files
typedef struct	Memlayer Memlayer;
typedef struct	Fontchar Fontchar;
#pragma incomplete Memlayer
#pragma incomplete Fontchar

/*s: struct Memdata */
/*
 * Memdata is allocated from main pool, but .data from the image pool.
 * Memdata is allocated separately to permit patching its pointer after
 * compaction when windows share the image data.
 * The first word of data is a back pointer to the Memdata, to find
 * The word to patch.
 */
struct Memdata
{
    ulong	*base;	/* allocated data pointer */
    // the pixels!
    byte	*bdata;	/* pointer to first byte of actual data; word-aligned */

    /*s: [[Memdata]] other fields */
    bool	allocd;	/* is this malloc'd? */
    /*e: [[Memdata]] other fields */
    // Extra
    int		ref;		/* number of Memimages using this data */
};
/*e: struct Memdata */

/*s: enum fxxx */
enum {
    Frepl	= 1<<0,	/* is replicated */
    // derives from Memimage.chan
    Fgrey	= 1<<2,	/* is grey */
    Falpha	= 1<<3,	/* has explicit alpha */
    Fcmap	= 1<<4,	/* has cmap channel */
    Fbytes	= 1<<5,	/* has only 8-bit channels */
};
/*e: enum fxxx */

/*s: struct Memimage */
struct Memimage
{
    Rectangle	r;		/* rectangle in data area, local coords */

    channels	chan;	/* channel descriptions */
    // derives from Memimage.chan
    int		depth;	/* number of bits of storage per pixel */
    int		nchan;	/* number of channels */

    Rectangle	clipr;		/* clipping region */
    //bitset<enum<fxxx>, // includes if replicate via Frepl
    ulong	flags; 

    // finally, the raw pixels
    // ref_own<Memdata>
    Memdata	*data;	/* pointer to data; shared by windows in this image */

    /*s: [[MemImage]] layer fields */
    // ref_own<Memlayer>
    Memlayer	*layer;	/* nil if not a layer*/
    /*e: [[MemImage]] layer fields */
    /*s: [[MemImage]] other fields */
    int		zero;		/* data->bdata+zero==&byte containing (0,0) */
    ulong	width;	/* width in words of a single scan line */
    /*x: [[MemImage]] other fields */
    // map<enum<ImageChan>, int>
    int		shift[NChan];
    // map<enum<ImageChan>, int>
    int		mask[NChan];
    // map<enum<ImageChan>, int>
    int		nbits[NChan];
    /*x: [[MemImage]] other fields */
    Memcmap	*cmap;
    /*e: [[MemImage]] other fields */
};
/*e: struct Memimage */

/*s: struct Memcmap */
struct Memcmap
{
    // map<colorcmap8, (reg8, green8, blue8)>
    byte	cmap2rgb[3*256];
    // map<(red4,green4,blue4), colorcmap8)
    byte	rgb2cmap[16*16*16];
};
/*e: struct Memcmap */

/*s: struct Memsubfont */
/*
 * Subfonts
 *
 * given char c, Subfont *f, Fontchar *i, and Point p, one says
 *	i = f->info+c;
 *	draw(b, Rect(p.x+i->left, p.y+i->top,
 *		p.x+i->left+((i+1)->x-i->x), p.y+i->bottom),
 *		color, f->bits, Pt(i->x, i->top));
 *	p.x += i->width;
 * to draw characters in the specified color (itself a Memimage) in Memimage b.
 */
struct	Memsubfont
{
    char	*name;

    Memimage	*bits;		/* of font */

    short	n;		/* number of chars in font */
    Fontchar *info;		/* n+1 character descriptors */

    uchar	height;		/* height of bitmap */
    char	ascent;		/* top of bitmap to baseline */
};
/*e: struct Memsubfont */

/*s: enum _anon_ (include/memdraw.h)2 */
/*
 * Encapsulated parameters and information for sub-draw routines.
 */
enum Drawparams {
    Simplesrc =1<<0, // 1x1 src
    Simplemask=1<<1, // 1x1 mask

    Replsrc =1<<2,
    Replmask=1<<3,

    Fullmask=1<<4, // fully opaque mask
};
/*e: enum _anon_ (include/memdraw.h)2 */
//TODO: internal to draw.c?
/*s: struct Memdrawparam */
struct	Memdrawparam
{
    Memimage *dst;
    Rectangle	r;

    Memimage *src;
    Rectangle sr;

    Memimage *mask;
    Rectangle mr;

    // enum<Drawop>
    int op;

    /*s: [[Memdrawparam]] other fields */
    // enum<Drawparams>
    ulong state;
    /*x: [[Memdrawparam]] other fields */
    ulong sval;		/* if Simplesrc, the source pixel in src format */
    rgba  srgba;	/* sval in rgba */
    ulong sdval;	/* sval in dst format */
    /*x: [[Memdrawparam]] other fields */
    ulong mval;		/* if Simplemask, the mask pixel in mask format */
    rgba  mrgba;	/* mval in rgba */
    /*e: [[Memdrawparam]] other fields */
};
/*e: struct Memdrawparam */

/*
 * Memimage management
 */
extern Memimage*	allocmemimage(Rectangle, ulong);
extern Memimage*	allocmemimaged(Rectangle, ulong, Memdata*);
extern void			freememimage(Memimage*);

extern Memimage*	readmemimage(fdt);
extern int			writememimage(int, Memimage*);
extern int			loadmemimage(Memimage*, Rectangle, uchar*, int);
extern int			unloadmemimage(Memimage*, Rectangle, uchar*, int);
extern Memimage*	creadmemimage(int);
extern int			cloadmemimage(Memimage*, Rectangle, uchar*, int);

// misc utilities
extern ulong*	wordaddr(Memimage*, Point);
extern uchar*	byteaddr(Memimage*, Point);

extern int		drawclip(Memimage*, Rectangle*, Memimage*, Point*, Memimage*, Point*, Rectangle*, Rectangle*);
extern void		memfillcolor(Memimage*, ulong);
extern int		memsetchan(Memimage*, ulong);

/*
 * Graphics
 */
extern void	memimageinit(void);

// actually in memlayer
extern void	memdraw(Memimage*, Rectangle, Memimage*, Point, Memimage*, Point, int);
extern void	memimagedraw(Memimage*, Rectangle, Memimage*, Point, Memimage*, Point, int);

// actually in memlayer/
extern void	memline(Memimage*, Point, Point, int, int, int, Memimage*, Point, int);


extern void	mempoly(Memimage*, Point*, int, int, int, int, Memimage*, Point, int);
extern void	memfillpoly(Memimage*, Point*, int, int, Memimage*, Point, int);


extern void	memellipse(Memimage*, Point, int, int, int, Memimage*, Point, int);

extern void	memarc(Memimage*, Point, int, int, int, Memimage*, Point, int, int, int);

extern Point	memimagestring(Memimage*, Point, Memimage*, Point, Memsubfont*, char*);

// TODO: cruft private?
// !!!
extern int	hwdraw(Memdrawparam*);


extern Rectangle	memlinebbox(Point, Point, int, int, int);
extern int			memlineendsize(int);
extern void			_memmkcmap(void);

/*
 * Subfont management
 */
extern Point		memsubfontwidth(Memsubfont*, char*);
extern Memsubfont*	getmemdefont(void);
extern Memsubfont*	allocmemsubfont(char*, int, int, int, Fontchar*, Memimage*);

/*
 * Predefined 
 */
extern	Memimage*	memwhite;
extern	Memimage*	memblack;
extern	Memimage*	memopaque;
extern	Memimage*	memtransparent;

extern	Memcmap	*memdefcmap;


// Forward decl? or should be fixed?
extern void	_memimageline(Memimage*, Point, Point, int, int, int, Memimage*, Point, Rectangle, int);
//todo: remove this one and rename the previous one
//extern void	memimageline(Memimage*, Point, Point, int, int, int, Memimage*, Point, int);

extern void	_memfillpolysc(Memimage*, Point*, int, int, Memimage*, Point, int, int, int, int);

// used by color.c and alphadraw.c now (and test files)
/*s: function RGB2K */
/* perfect approximation to NTSC = .299r+.587g+.114b when 0 ≤ r,g,b < 256 */
#define RGB2K(r,g,b)	((156763*(r)+307758*(g)+59769*(b))>>19)
/*e: function RGB2K */

/*
 * Kernel interface
 */
void		memimagemove(void*, void*);

/*
 * Kernel cruft
 */
extern void	rdb(void);
extern int	(*iprint)(char*, ...);
#pragma varargck argpos iprint 1

extern int		drawdebug;

/*
 * doprint interface: numbconv bit strings
 */
#pragma varargck type "llb" vlong
#pragma varargck type "llb" uvlong
#pragma varargck type "lb" long
#pragma varargck type "lb" ulong
#pragma varargck type "b" int
#pragma varargck type "b" uint

/*e: include/memdraw.h */
