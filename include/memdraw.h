/*s: include/memdraw.h */
#pragma	src	"/sys/src/libmemdraw"
#pragma	lib	"libmemdraw.a"

typedef struct	Memimage Memimage;
typedef struct	Memdata Memdata;
typedef struct	Memsubfont Memsubfont;
typedef struct	Memlayer Memlayer;
typedef struct	Memcmap Memcmap;
typedef struct	Memdrawparam	Memdrawparam;

#pragma incomplete Memlayer

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
    int		ref;		/* number of Memimages using this data */

    void*	imref;
    int		allocd;	/* is this malloc'd? */
    /*e: [[Memdata]] other fields */
};
/*e: struct Memdata */

/*s: enum fxxx */
enum {
    Frepl	= 1<<0,	/* is replicated */
    Fsimple	= 1<<1,	/* is 1x1 */
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
    Rectangle	clipr;		/* clipping region */
    //bitset<enum<fxxx>
    ulong	flags;

    int		depth;	/* number of bits of storage per pixel */
    ulong	chan;	/* channel descriptions */
    int		nchan;	/* number of channels */

    // finally, the raw pixels
    // ref_own<Memdata>
    Memdata	*data;	/* pointer to data; shared by windows in this image */

    /*s: [[MemImage]] other fields */
    int		zero;		/* data->bdata+zero==&byte containing (0,0) */
    ulong	width;	/* width in words of a single scan line */

    int		shift[NChan];
    int		mask[NChan];
    int		nbits[NChan];
    /*x: [[MemImage]] other fields */
    Memcmap	*cmap;
    /*x: [[MemImage]] other fields */
    Memlayer	*layer;	/* nil if not a layer*/
    /*e: [[MemImage]] other fields */
};
/*e: struct Memimage */

/*s: struct Memcmap */
struct Memcmap
{
    uchar	cmap2rgb[3*256];
    uchar	rgb2cmap[16*16*16];
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
    short	n;		/* number of chars in font */
    uchar	height;		/* height of bitmap */
    char	ascent;		/* top of bitmap to baseline */

    Fontchar *info;		/* n+1 character descriptors */

    Memimage	*bits;		/* of font */
};
/*e: struct Memsubfont */

/*s: enum _anon_ (include/memdraw.h)2 */
/*
 * Encapsulated parameters and information for sub-draw routines.
 */
enum {
    Simplesrc=1<<0,
    Simplemask=1<<1,
    Replsrc=1<<2,
    Replmask=1<<3,
    Fullmask=1<<4,
};
/*e: enum _anon_ (include/memdraw.h)2 */
/*s: struct Memdrawparam */
struct	Memdrawparam
{
    Memimage *dst;
    Rectangle	r;
    Memimage *src;
    Rectangle sr;
    Memimage *mask;
    Rectangle mr;
    int op;

    ulong state;
    ulong mval;		/* if Simplemask, the mask pixel in mask format */
    ulong mrgba;	/* mval in rgba */
    ulong sval;		/* if Simplesrc, the source pixel in src format */
    ulong srgba;	/* sval in rgba */
    ulong sdval;	/* sval in dst format */
};
/*e: struct Memdrawparam */

/*
 * Memimage management
 */
extern Memimage*	allocmemimage(Rectangle, ulong);
extern Memimage*	allocmemimaged(Rectangle, ulong, Memdata*);
extern void			freememimage(Memimage*);

extern Memimage*	readmemimage(int);
extern int			writememimage(int, Memimage*);
extern int			loadmemimage(Memimage*, Rectangle, uchar*, int);
extern int			unloadmemimage(Memimage*, Rectangle, uchar*, int);
extern Memimage*	creadmemimage(int);
extern int			cloadmemimage(Memimage*, Rectangle, uchar*, int);

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

// !!!
extern int	hwdraw(Memdrawparam*);


extern Rectangle	memlinebbox(Point, Point, int, int, int);
extern int			memlineendsize(int);
extern void			_memmkcmap(void);

/*
 * Subfont management
 */
extern Memsubfont*	allocmemsubfont(char*, int, int, int, Fontchar*, Memimage*);
extern void			freememsubfont(Memsubfont*);
extern Memsubfont*	openmemsubfont(char*);
extern Point		memsubfontwidth(Memsubfont*, char*);
extern Memsubfont*	getmemdefont(void);

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
