/*s: lib_graphics/libmemdraw/draw.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <pool.h>

#define DBG1 if(0) print
#define DBG if(0) print

extern Pool* imagmem;

/*s: global drawdebug */
bool drawdebug;
/*e: global drawdebug */

/*s: function RGB2K */
/* perfect approximation to NTSC = .299r+.587g+.114b when 0 ≤ r,g,b < 256 */
#define RGB2K(r,g,b)	((156763*(r)+307758*(g)+59769*(b))>>19)
/*e: function RGB2K */

/*s: function MUL */
#define MUL(x, y, t)	(t = (x)*(y)+128, (t+(t>>8))>>8)
/*e: function MUL */
/*s: constant MASK13 */
#define MASK13	0xFF00FF00
/*e: constant MASK13 */
/*s: constant MASK02 */
#define MASK02	0x00FF00FF
/*e: constant MASK02 */
/*s: function MUL13 */
#define MUL13(a, x, t)		(t = (a)*(((x)&MASK13)>>8)+128, ((t+((t>>8)&MASK02))>>8)&MASK02)
/*e: function MUL13 */
/*s: function MUL02 */
#define MUL02(a, x, t)		(t = (a)*(((x)&MASK02)>>0)+128, ((t+((t>>8)&MASK02))>>8)&MASK02)
/*e: function MUL02 */
/*s: function MUL0123 */
#define MUL0123(a, x, s, t)	((MUL13(a, x, s)<<8)|MUL02(a, x, t))
/*e: function MUL0123 */

static void mktables(void);

typedef int Subdraw(Memdrawparam*);
static Subdraw chardraw, alphadraw, memoptdraw;

/*s: global memones */
static Memimage*	memones;
/*e: global memones */
/*s: global memzeros */
static Memimage*	memzeros;
/*e: global memzeros */
/*s: global memwhite */
Memimage *memwhite;
/*e: global memwhite */
/*s: global memblack */
Memimage *memblack;
/*e: global memblack */
/*s: global memtransparent */
Memimage *memtransparent;
/*e: global memtransparent */
/*s: global memopaque */
Memimage *memopaque;
/*e: global memopaque */

int	_ifmt(Fmt*);

/*s: function memimageinit */
void
memimageinit(void)
{
    /*s: [[memimageinit()]] only once guard */
    static bool didinit = false;
    if(didinit)
        return;
    didinit = true;
    /*e: [[memimageinit()]] only once guard */

    /*s: [[memimageinit()]] set image pool allocator move */
    if(  strcmp(imagmem->name, "Image") == 0 
      || strcmp(imagmem->name, "image") == 0
      )
        imagmem->move = memimagemove;
    /*e: [[memimageinit()]] set image pool allocator move */
    /*s: [[memimageinit()]] initializations */
    _memmkcmap();
    /*x: [[memimageinit()]] initializations */
    mktables();
    /*e: [[memimageinit()]] initializations */
    /*s: [[memimageinit()]] install dumpers */
    fmtinstall('P', Pfmt);
    fmtinstall('R', Rfmt); 
    /*x: [[memimageinit()]] install dumpers */
    fmtinstall('b', _ifmt);
    /*e: [[memimageinit()]] install dumpers */

    memzeros = allocmemimage(Rect(0,0,1,1), GREY1);
    memzeros->flags |= Frepl;
    memzeros->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
    *byteaddr(memzeros, ZP) = 0;

    memones = allocmemimage(Rect(0,0,1,1), GREY1);
    memones->flags |= Frepl;
    memones->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
    *byteaddr(memones, ZP) = ~0;

    /*s: [[memimageinit()]] sanity check memxxx */
    if(memones == nil || memzeros == nil)
        assert(0 /*cannot initialize memimage library */);	/* RSC BUG */
    /*e: [[memimageinit()]] sanity check memxxx */

    memwhite = memones;
    memblack = memzeros;
    memopaque = memones;
    memtransparent = memzeros;
}
/*e: function memimageinit */

static ulong imgtorgba(Memimage*, ulong);
static ulong rgbatoimg(Memimage*, ulong);
static ulong pixelbits(Memimage*, Point);

/*s: function memimagedraw */
void
memimagedraw(Memimage *dst, Rectangle r, Memimage *src, Point p0, Memimage *mask, Point p1, int op)
{
    Memdrawparam par;

    DBG1("memimagedraw %p/%luX %R @ %p %p/%luX %P %p/%luX %P... ", dst, dst->chan, r, dst->data->bdata, src, src->chan, p0, mask, mask->chan, p1);
    /*s: [[memdraw()]] sanity check mask */
    if(mask == nil)
        mask = memopaque;
    /*e: [[memdraw()]] sanity check mask */
    /*s: [[memdraw()]] sanity check op */
    if(op < Clear || op > SoverD){
        DBG1("op out of range: %d\n", op);
        return;
    }
    /*e: [[memdraw()]] sanity check op */

    // Clipping
    /*s: [[memdraw()]] call drawclip, if empty rectangle return */
    if(!drawclip(dst, &r, src, &p0, mask, &p1,   &par.sr, &par.mr)){
        DBG1("empty clipped rectangle\n");
        return;
    }
    /*e: [[memdraw()]] call drawclip, if empty rectangle return */

    par.op = op;
    par.dst = dst;
    par.r = r;
    par.src = src;
    par.mask = mask;
    /* par.sr set by drawclip */
    /* par.mr set by drawclip */

    // Replicating (and adjust par.state)
    par.state = 0;
    /*s: [[memimagedraw()]] if src is repl */
    if(src->flags&Frepl){
        par.state |= Replsrc;
        if(Dx(src->r)==1 && Dy(src->r)==1){
            par.state |= Simplesrc;
            par.sval = pixelbits(src, src->r.min);

            par.srgba = imgtorgba(src, par.sval);
            par.sdval = rgbatoimg(dst, par.srgba);

            /*s: [[memimagedraw()]] when src is repl, sanity check pixel value */
            if((par.srgba&0xFF) == 0 && (op&DoutS)){
                DBG1("fill with transparent source\n");
                return;	/* no-op successfully handled */
            }
            /*e: [[memimagedraw()]] when src is repl, sanity check pixel value */
        }
    }
    /*e: [[memimagedraw()]] if src is repl */
    /*s: [[memimagedraw()]] if mask is repl */
    if(mask->flags & Frepl){
        par.state |= Replmask;
        if(Dx(mask->r)==1 && Dy(mask->r)==1){
            par.state |= Simplemask;
            par.mval = pixelbits(mask, mask->r.min);

            if(par.mval == 0 && (op&DoutS)){
                DBG1("fill with zero mask\n");
                return;	/* no-op successfully handled */
            }

            if(par.mval == ~0)
                par.state |= Fullmask;
            par.mrgba = imgtorgba(mask, par.mval);
        }
    }
    /*e: [[memimagedraw()]] if mask is repl */

    DBG1("dr %R sr %R mr %R...", r, par.sr, par.mr);
    DBG1("draw dr %R sr %R mr %R %lux\n", r, par.sr, par.mr, par.state);

    // Optimizing
    /*
     * Now that we've clipped the parameters down to be consistent, we 
     * simply try sub-drawing routines in order until we find one that was able
     * to handle us.  If the sub-drawing routine returns zero, it means it was
     * unable to satisfy the request, so we do not return.
     */
    /*s: [[memimagedraw()]] try hwdraw */
    /*
     * Hardware support.  Each video driver provides this function,
     * which checks to see if there is anything it can help with.
     * There could be an if around this checking to see if dst is in video memory.
     */
    if(hwdraw(&par)){
        DBG1("hwdraw handled\n");
        return;
    }
    /*e: [[memimagedraw()]] try hwdraw */
    /*s: [[memimagedraw()]] try memoptdraw */
    /*
     * Optimizations using memmove and memset.
     */
    if(memoptdraw(&par)){
        DBG1("memopt handled\n");
        return;
    }
    /*e: [[memimagedraw()]] try memoptdraw */
    /*s: [[memimagedraw()]] try chardraw */
    /*
     * Character drawing.
     * Solid source color being painted through a boolean mask onto a 
     * high res image.
     */
    if(chardraw(&par)){
        DBG1("chardraw handled\n");
        return;
    }
    /*e: [[memimagedraw()]] try chardraw */
    // else

    // Compositing
    /*
     * General calculation-laden case that does alpha for each pixel.
     */
    alphadraw(&par);
    DBG("alphadraw handled\n");
}
/*e: function memimagedraw */

/*s: function drawclip */
/*
 * Clip the destination rectangle further based on the properties of the 
 * source and mask rectangles.  Once the destination rectangle is properly
 * clipped, adjust the source and mask rectangles to be the same size.
 * Then if source or mask is replicated, move its clipped rectangle
 * so that its minimum point falls within the repl rectangle.
 *
 * Return zero if the final rectangle is null.
 */
bool
drawclip(Memimage *dst, Rectangle *r, Memimage *src, Point *p0, Memimage *mask, Point *p1,   Rectangle *sr, Rectangle *mr)
{
    Point rmin = r->min; // save old min
    /*s: [[drawclip()]] other locals */
    Point delta;
    /*x: [[drawclip()]] other locals */
    bool splitcoords = (p0->x != p1->x) || (p0->y != p1->y);
    /*x: [[drawclip()]] other locals */
    Rectangle omr;
    /*e: [[drawclip()]] other locals */

    // empty rectangle? nothing to do then.
    if(r->min.x >= r->max.x || r->min.y >= r->max.y)
        return false;

    /* clip to destination */ // can modify r
    if(!rectclip(r, dst->r) || !rectclip(r, dst->clipr))
        return false;
    /*s: [[drawclip()]] adjust p0 and p1 if r changed */
    /* move source point */
    p0->x += r->min.x - rmin.x;
    p0->y += r->min.y - rmin.y;
    /* move mask point */
    p1->x += r->min.x - rmin.x;
    p1->y += r->min.y - rmin.y;
    /*e: [[drawclip()]] adjust p0 and p1 if r changed */

    /* map destination rectangle into source */
    sr->min = *p0;
    sr->max.x = p0->x+Dx(*r);
    sr->max.y = p0->y+Dy(*r);

    /* sr is r in source coordinates; clip to source */ // can modify sr
    if(!(src->flags&Frepl) && !rectclip(sr, src->r))
        return false;
    if(!rectclip(sr, src->clipr))
        return false;

    /* compute and clip rectangle in mask */
    /*s: [[drawclip()]] if splitcoords */
    if(splitcoords){
        /* move mask point with source */
        p1->x += sr->min.x - p0->x;
        p1->y += sr->min.y - p0->y;
        mr->min = *p1;
        mr->max.x = p1->x+Dx(*sr);
        mr->max.y = p1->y+Dy(*sr);
        omr = *mr;

        /* mr is now rectangle in mask; clip it */
        if(!(mask->flags&Frepl) && !rectclip(mr, mask->r))
            return false;
        if(!rectclip(mr, mask->clipr))
            return false;

        /* reflect any clips back to source */
        sr->min.x += mr->min.x - omr.min.x;
        sr->min.y += mr->min.y - omr.min.y;
        sr->max.x += mr->max.x - omr.max.x;
        sr->max.y += mr->max.y - omr.max.y;
        *p1 = mr->min;
    }
    /*e: [[drawclip()]] if splitcoords */
    else{
        if(!(mask->flags&Frepl) && !rectclip(sr, mask->r)) // can modify sr
            return false;
        if(!rectclip(sr, mask->clipr))
            return false;
        *p1 = sr->min;
    }

    /* move source clipping back to destination */
    /*s: [[drawclip()]] adjust r if sr or mr changed */
    delta.x = r->min.x - p0->x;
    delta.y = r->min.y - p0->y;
    r->min.x = sr->min.x + delta.x;
    r->min.y = sr->min.y + delta.y;
    r->max.x = sr->max.x + delta.x;
    r->max.y = sr->max.y + delta.y;
    /*e: [[drawclip()]] adjust r if sr or mr changed */

    /*s: [[drawclip()]] special cases */
    /* move source rectangle so sr->min is in src->r */
    /*s: [[drawclip()]] if src is repl */
    if(src->flags&Frepl) {
        delta.x = drawreplxy(src->r.min.x, src->r.max.x, sr->min.x) - sr->min.x;
        delta.y = drawreplxy(src->r.min.y, src->r.max.y, sr->min.y) - sr->min.y;
        sr->min.x += delta.x;
        sr->min.y += delta.y;
        sr->max.x += delta.x;
        sr->max.y += delta.y;
    }
    *p0 = sr->min;
    /*e: [[drawclip()]] if src is repl */
    /* move mask point so it is in mask->r */
    *p1 = drawrepl(mask->r, *p1);
    /*e: [[drawclip()]] special cases */

    mr->min = *p1;
    mr->max.x = p1->x+Dx(*sr);
    mr->max.y = p1->y+Dy(*sr);

    assert(Dx(*sr) == Dx(*mr) && Dx(*mr) == Dx(*r));
    assert(Dy(*sr) == Dy(*mr) && Dy(*mr) == Dy(*r));
    assert(ptinrect(*p0, src->r));
    assert(ptinrect(*p1, mask->r));
    assert(ptinrect(r->min, dst->r));

    return true;
}
/*e: function drawclip */

/*s: global replbit */
/*
 * Conversion tables.
 */
static uchar replbit[1+8][256];		/* replbit[x][y] is the replication of the x-bit quantity y to 8-bit depth */
/*e: global replbit */

extern int replmul[];

/*s: function mktables */
static void
mktables(void)
{
    int i, j, small;
    /*s: [[mktables()]] only once guard */
    static bool	tablesbuilt = false;
    if(tablesbuilt)
        return;
    tablesbuilt = true;
    /*e: [[mktables()]] only once guard */

    /* bit replication up to 8 bits */
    for(i=0; i<256; i++){
        for(j=0; j<=8; j++){	/* j <= 8 [sic] */
            small = i & ((1<<j)-1);
            replbit[j][i] = (small*replmul[j])>>8;
        }
    }

}
/*e: function mktables */

/*s: global ones */
static uchar ones = 0xff;
/*e: global ones */

/*
 * General alpha drawing case.  Can handle anything.
 */
typedef struct	Buffer	Buffer;
/*s: struct Buffer */
struct Buffer {
    /* used by most routines */
    byte	*red;
    byte	*grn;
    byte	*blu;

    byte	*alpha; // can be &ones

    byte	*grey;

    ulong	*rgba; // bad name, just start of pixel

    int	delta;	/* number of bytes to add to pointer to get next pixel to the right */

    /*s: [[Buffer]] boolcalc fields */
    /* used by boolcalc* for mask data */
    uchar	*m;		/* ptr to mask data r.min byte; like p->bytermin */
    int		mskip;	/* no. of left bits to skip in *m */
    uchar	*bm;		/* ptr to mask data img->r.min byte; like p->bytey0s */
    int		bmskip;	/* no. of left bits to skip in *bm */
    uchar	*em;		/* ptr to mask data img->r.max.x byte; like p->bytey0e */
    int		emskip;	/* no. of right bits to skip in *em */
    /*e: [[Buffer]] boolcalc fields */
};
/*e: struct Buffer */

typedef struct	ParamDraw	Param;
typedef Buffer	Readfn(Param*, uchar*, int);
typedef void	Writefn(Param*, uchar*, Buffer);
typedef Buffer	Calcfn(Buffer, Buffer, Buffer, int, int, int);

/*s: enum _anon_ (lib_graphics/libmemdraw/draw.c) */
enum {
    MAXBCACHE = 16
};
/*e: enum _anon_ (lib_graphics/libmemdraw/draw.c) */

/*s: struct ParamDraw */
/* giant rathole to customize functions with */
struct ParamDraw {

    Memimage *img;
    Rectangle	r;
    int	dx;	/* of r */ // size of a line of a rectangle in pixels
    int		bwidth; // image width in bytes

    // source bytes
    byte	*bytey0s;	/* byteaddr(Pt(img->r.min.x, img->r.min.y)) */
    byte	*bytermin;	/* byteaddr(Pt(r.min.x, img->r.min.y)) */
    byte	*bytey0e;	/* byteaddr(Pt(img->r.max.x, img->r.min.y)) */

    bool	needbuf;
    // destination bytes
    int	bufoff;
    int	bufdelta;
    // array<byte>, (address = Dbuf.p + bufoff, length = bufdelta)
    byte	*bufbase;

    int	dir; // -1 or 1

    /*s: [[ParamDraw]] mask fields */
    bool	alphaonly;
    Readfn	*greymaskcall;	
    /*e: [[ParamDraw]] mask fields */
    /*s: [[ParamDraw]] conversion fields */
    Readfn	*convreadcall;
    Writefn	*convwritecall;
    int	convbufoff;
    Param	*convdpar;
    int	convdx;
    /*x: [[ParamDraw]] conversion fields */
    uchar	*convbuf;
    /*e: [[ParamDraw]] conversion fields */
    /*s: [[ParamDraw]] replication fields */
    bool	replcache;	/* if set, cache buffers */
    /*x: [[ParamDraw]] replication fields */
    Readfn	*replcall;
    /*x: [[ParamDraw]] replication fields */
    Buffer	bcache[MAXBCACHE];
    ulong	bfilled;
    /*e: [[ParamDraw]] replication fields */
    /*s: [[ParamDraw]] other fields */
    bool	convgrey;
    /*e: [[ParamDraw]] other fields */
};
/*e: struct ParamDraw */

static Readfn	greymaskread, replread, readptr;

/*s: global nullwrite */
static Writefn	nullwrite;
/*e: global nullwrite */

static Calcfn	alphacalc0, alphacalc14, alphacalc2810, alphacalc3679, alphacalc5, alphacalc11, alphacalcS;
static Calcfn	boolcalc14, boolcalc236789, boolcalc1011;

static Readfn*	readfn(Memimage*);
static Readfn*	readalphafn(Memimage*);
static Writefn*	writefn(Memimage*);

static Calcfn*	boolcopyfn(Memimage*, Memimage*);
static Readfn*	convfn(Memimage*, Param*, Memimage*, Param*, int*);
static Readfn*	ptrfn(Memimage*);

/*s: global alphacalc */
static Calcfn *alphacalc[Ncomp] = 
{
    alphacalc0,         /* Clear */
    alphacalc14,        /* DoutS */
    alphacalc2810,      /* SoutD */
    alphacalc3679,      /* DxorS */
    alphacalc14,        /* DinS */
    alphacalc5,         /* D */
    alphacalc3679,      /* DatopS */
    alphacalc3679,      /* DoverS */
    alphacalc2810,      /* SinD */
    alphacalc3679,      /* SatopD */
    alphacalc2810,      /* S */
    alphacalc11,        /* SoverD */
};
/*e: global alphacalc */

/*s: global boolcalc */
static Calcfn *boolcalc[Ncomp] =
{
    alphacalc0,		/* Clear */
    boolcalc14,		/* DoutS */
    boolcalc236789,		/* SoutD */
    boolcalc236789,		/* DxorS */
    boolcalc14,		/* DinS */
    alphacalc5,		/* D */
    boolcalc236789,		/* DatopS */
    boolcalc236789,		/* DoverS */
    boolcalc236789,		/* SinD */
    boolcalc236789,		/* SatopD */
    boolcalc1011,		/* S */
    boolcalc1011,		/* SoverD */
};
/*e: global boolcalc */

/*
 * Avoid standard Lock, QLock so that can be used in kernel.
 */
typedef struct Dbuf Dbuf;
/*s: struct Dbuf */
struct Dbuf
{
    Param spar, mpar, dpar;
    // array<byte> (length = Dbuf.n)
    byte *p;
    int n;
    /*s: [[Dbuf]] other fields */
    bool inuse;
    /*e: [[Dbuf]] other fields */
};
/*e: struct Dbuf */
/*s: global dbuf */
static Dbuf dbuf[10];
/*e: global dbuf */

/*s: function allocdbuf */
static Dbuf*
allocdbuf(void)
{
    int i;

    for(i=0; i<nelem(dbuf); i++){
        if(dbuf[i].inuse)
            continue;
        if(!_tas(&dbuf[i].inuse))
            return &dbuf[i];
    }
    return nil;
}
/*e: function allocdbuf */

/*s: function getparam */
static void
getparam(Param *p, Memimage *img, Rectangle r, bool convgrey, bool needbuf, int *ndrawbuf)
{
    int nbuf;

    memset(p, 0, sizeof(Param));

    p->img = img;
    p->r = r;
    p->dx = Dx(r);
    p->needbuf = needbuf;
    p->convgrey = convgrey;

    assert(img->r.min.x <= r.min.x && r.min.x < img->r.max.x);

    p->bytey0s  = byteaddr(img, Pt(img->r.min.x, img->r.min.y));
    p->bytermin = byteaddr(img, Pt(r.min.x, img->r.min.y));
    p->bytey0e  = byteaddr(img, Pt(img->r.max.x, img->r.min.y));

    p->bwidth   = sizeof(ulong)*img->width;

    assert(p->bytey0s <= p->bytermin && p->bytermin <= p->bytey0e);
    if(p->r.min.x == p->img->r.min.x)
        assert(p->bytermin == p->bytey0s);

    nbuf = 1;
    /*s: [[getparam()]] if small replicated rectangle */
    if((img->flags&Frepl) && Dy(img->r) <= MAXBCACHE && Dy(img->r) < Dy(r)){
        p->replcache = true;
        nbuf = Dy(img->r);
    }
    /*e: [[getparam()]] if small replicated rectangle */

    p->bufdelta = 4 * p->dx;
    p->bufoff = *ndrawbuf;
    *ndrawbuf += p->bufdelta * nbuf;
}
/*e: function getparam */

/*s: function clipy */
static void
clipy(Memimage *img, int *y)
{
    int dy;

    dy = Dy(img->r);
    if(*y == dy)
        *y = 0;
    else if(*y == -1)
        *y = dy-1;
    assert(0 <= *y && *y < dy);
}
/*e: function clipy */

/*s: function alphadraw */
/*
 * For each scan line, we expand the pixels from source, mask, and destination
 * into byte-aligned red, green, blue, alpha, and grey channels.  If buffering
 * is not needed and the channels were already byte-aligned 
 * (grey8, rgb24, rgba32, rgb32),
 * the readers need not copy the data: they can simply return pointers to the
 * data.
 * If the destination image is grey and the source is not, it is converted
 * using the NTSC formula.
 *
 * Once we have all the channels, we call either rgbcalc or greycalc, 
 * depending on whether the destination image is color.  This is allowed to
 * overwrite the dst buffer (perhaps the actual data, perhaps a copy) with 
 * its result.  It should only overwrite the dst buffer
 * with the same format (i.e. red bytes with red bytes, etc.)  A new buffer
 * is returned from the calculator, and that buffer is passed to a function 
 * to write it to the destination.
 * If the buffer is already pointing at the destination, the writing function 
 * is a no-op.
 */
static int
alphadraw(Memdrawparam *par)
{
    /*s: [[alphadraw()]] locals */
    Memimage *src, *mask, *dst;
    Rectangle r, sr, mr;
    int dx, dy;
    // enum<Drawop>
    int op;
    /*x: [[alphadraw()]] locals */
    bool isgrey;
    /*x: [[alphadraw()]] locals */
    Readfn *rdsrc, *rdmask, *rddst;
    Calcfn *calc;
    Writefn *wrdst;
    /*x: [[alphadraw()]] locals */
    Buffer bsrc, bdst, bmask;
    /*x: [[alphadraw()]] locals */
    int starty, endy;
    int dsty, srcy, masky;
    int dir;
    int y; // iteration variable, we iterate over lines
    /*x: [[alphadraw()]] locals */
    bool needbuf;
    /*x: [[alphadraw()]] locals */
    Dbuf *z;
    /*x: [[alphadraw()]] locals */
    byte *drawbuf;
    int ndrawbuf;
    /*e: [[alphadraw()]] locals */

    r = par->r;
    dx = Dx(r);
    dy = Dy(r);
    src = par->src;
    mask = par->mask;	
    dst = par->dst;
    sr = par->sr;
    mr = par->mr;
    op = par->op;

    starty = 0;
    endy = dy;
    dir = 1;

    isgrey = dst->flags&Fgrey;
    /*s: [[alphadraw()]] set needbuf and possibly adjust starty and endy */
    /*
     * Buffering when src and dst are the same bitmap is sufficient but not 
     * necessary.  There are stronger conditions we could use.  We could
     * check to see if the rectangles intersect, and if simply moving in the
     * correct y direction can avoid the need to buffer.
     */
    needbuf = (src->data == dst->data);

    if (needbuf && byteaddr(dst, r.min) > byteaddr(src, sr.min)) {
        dir = -1;
        starty = dy-1;
        endy = -1;
    }
    /*e: [[alphadraw()]] set needbuf and possibly adjust starty and endy */

    /*s: [[alphadraw()]] set Dbuf z part1 */
    z = allocdbuf();
    /*s: [[alphadraw()]] sanity check z */
    if(z == nil)
        return 0;
    /*e: [[alphadraw()]] sanity check z */
    /*s: [[alphadraw()]] set z params part1 */
    ndrawbuf = 0;
    getparam(&z->spar, src, sr,  isgrey, needbuf, &ndrawbuf);
    getparam(&z->dpar, dst, r,   isgrey, needbuf, &ndrawbuf);
    getparam(&z->mpar, mask, mr, false,  needbuf, &ndrawbuf);
    z->spar.dir = z->mpar.dir = z->dpar.dir = dir;
    /*e: [[alphadraw()]] set z params part1 */
    /*e: [[alphadraw()]] set Dbuf z part1 */

    /*s: [[alphadraw()]] if source has no alpha and simple bit mask */
    /*
     * If the mask is purely boolean, we can convert from src to dst format
     * when we read src, and then just copy it to dst where the mask tells us to.
     * This requires a boolean (1-bit grey) mask and lack of a source alpha channel.
     *
     * The computation is accomplished by assigning the function pointers as follows:
     *	rdsrc - read and convert source into dst format in a buffer
     * 	rdmask - convert mask to bytes, set pointer to it
     * 	rddst - fill with pointer to real dst data, but do no reads
     *	calc - copy src onto dst when mask says to.
     *	wrdst - do nothing
     * This is slightly sleazy, since things aren't doing exactly what their names say,
     * but it avoids a fair amount of code duplication to make this a case here
     * rather than have a separate booldraw.
     */
    if(!(src->flags&Falpha) 
    && mask->chan == GREY1 
    && dst->depth >= 8 
    && op == SoverD){
        rdsrc = convfn(dst, &z->dpar, src, &z->spar, &ndrawbuf);
        rddst = readptr;
        rdmask = readfn(mask);
        calc = boolcopyfn(dst, mask);
        wrdst = nullwrite;
    }
    /*e: [[alphadraw()]] if source has no alpha and simple bit mask */
    else{
        /* usual alphadraw parameter fetching */
        rdsrc = readfn(src);
        rddst = readfn(dst);
        wrdst = writefn(dst);
        calc = alphacalc[op];
        /*s: [[alphadraw()]] set rdmask */
        if(mask->flags&Falpha){
            rdmask = readalphafn(mask);
            z->mpar.alphaonly = true;
        }
        /*
         * If there is no alpha channel, we'll ask for a grey channel
         * and pretend it is the alpha.
         */
        else{
            z->mpar.greymaskcall = readfn(mask);
            z->mpar.convgrey = true;
            rdmask = greymaskread;

            /*s: [[alphadraw()]] when mask and source have no alpha, possibly adapt calc */
            /*
             * Should really be above, but then boolcopyfns would have
             * to deal with bit alignment, and I haven't written that.
             *
             * This is a common case for things like ellipse drawing.
             * When there's no alpha involved and the mask is boolean,
             * we can avoid all the division and multiplication.
             */
            if(mask->chan == GREY1 && !(src->flags&Falpha))
                calc = boolcalc[op];
            else if(op == SoverD && !(src->flags&Falpha))
                calc = alphacalcS;
            /*e: [[alphadraw()]] when mask and source have no alpha, possibly adapt calc */
        }
        /*e: [[alphadraw()]] set rdmask */
    }

    /*s: [[alphadraw()]] when small repl rectangle optimization */
    /*
     * If the image has a small enough repl rectangle,
     * we can just read each line once and cache them.
     */
    if(z->spar.replcache){
        z->spar.replcall = rdsrc;
        rdsrc = replread;
    }
    if(z->mpar.replcache){
        z->mpar.replcall = rdmask;
        rdmask = replread;
    }
    /*e: [[alphadraw()]] when small repl rectangle optimization */
    /*s: [[alphadraw()]] set Dbuf z part2 */
    /*s: [[alphadraw()]] grow drawbuf if needed */
    if(z->n < ndrawbuf){
        free(z->p);
        z->p = mallocz(ndrawbuf, 0);
        /*s: [[alphadraw()]] sanity check [[z->p]] */
        if(z->p == nil){
            z->inuse = false;
            return 0;
        }
        /*e: [[alphadraw()]] sanity check [[z->p]] */
        z->n = ndrawbuf;
    }
    /*e: [[alphadraw()]] grow drawbuf if needed */
    drawbuf = z->p;
    /*s: [[alphadraw()]] set z params part2 */
    /*
     * Before we were saving only offsets from drawbuf in the parameter
     * structures; now that drawbuf has been grown to accomodate us,
     * we can fill in the pointers.
     */
    z->spar.bufbase = drawbuf + z->spar.bufoff;
    z->mpar.bufbase = drawbuf + z->mpar.bufoff;
    z->dpar.bufbase = drawbuf + z->dpar.bufoff;
    /*x: [[alphadraw()]] set z params part2 */
    z->spar.convbuf = drawbuf + z->spar.convbufoff;
    /*e: [[alphadraw()]] set z params part2 */
    /*e: [[alphadraw()]] set Dbuf z part2 */

   /*
    * srcy, masky, and dsty are offsets from the top of their
    * respective Rectangles.  they need to be contained within
    * the rectangles, so clipy can keep them there without division.
    */
    srcy  = (starty + sr.min.y - src->r.min.y)  % Dy(src->r);
    masky = (starty + mr.min.y - mask->r.min.y) % Dy(mask->r);
    dsty  =  starty + r.min.y  - dst->r.min.y;

    assert(0 <= srcy  && srcy  < Dy(src->r));
    assert(0 <= masky && masky < Dy(mask->r));
    assert(0 <= dsty  && dsty  < Dy(dst->r));

    // the big loop!
    for(y=starty; y!=endy; y+=dir, srcy+=dir, masky+=dir, dsty+=dir){
        /*s: [[alphadraw()]] clipping */
        clipy(src, &srcy);
        clipy(dst, &dsty);
        clipy(mask, &masky);
        /*e: [[alphadraw()]] clipping */

        bsrc  = rdsrc (&z->spar, z->spar.bufbase, srcy);
        bmask = rdmask(&z->mpar, z->mpar.bufbase, masky);
        bdst  = rddst (&z->dpar, z->dpar.bufbase, dsty);

        // !!The calc dispatch!!
        bdst = calc(bdst, bsrc, bmask, dx, isgrey, op);
        wrdst(&z->dpar, z->dpar.bytermin + dsty * z->dpar.bwidth, bdst);
    }
    /*s: [[alphadraw()]] free z */
    z->inuse = false;
    /*e: [[alphadraw()]] free z */
    return 1;
}
/*e: function alphadraw */

/*s: function alphacalc0 */
static Buffer
alphacalc0(Buffer bdst, Buffer b1, Buffer b2, int dx, int grey, int op)
{
    USED(grey);
    USED(op);
    USED(b1);
    USED(b2);

    memset(bdst.rgba, 0, dx * bdst.delta);
    return bdst;
}
/*e: function alphacalc0 */

/*s: function alphacalc14 */
static Buffer
alphacalc14(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int fd, sadelta;
    int i, sa, ma, q;
    ulong s, t;

    obdst = bdst;
    sadelta = bsrc.alpha == &ones ? 0 : bsrc.delta;
    q = bsrc.delta == 4 && bdst.delta == 4;

    for(i=0; i<dx; i++){
        sa = *bsrc.alpha;
        ma = *bmask.alpha;
        fd = MUL(sa, ma, t);
        if(op == DoutS)
            fd = 255-fd;

        if(grey){
            *bdst.grey = MUL(fd, *bdst.grey, t);
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            if(q){
                *bdst.rgba = MUL0123(fd, *bdst.rgba, s, t);
                bsrc.rgba++;
                bdst.rgba++;
                bsrc.alpha += sadelta;
                bmask.alpha += bmask.delta;
                continue;
            }
            *bdst.red = MUL(fd, *bdst.red, t);
            *bdst.grn = MUL(fd, *bdst.grn, t);
            *bdst.blu = MUL(fd, *bdst.blu, t);
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        if(bdst.alpha != &ones){
            *bdst.alpha = MUL(fd, *bdst.alpha, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        bsrc.alpha += sadelta;
    }
    return obdst;
}
/*e: function alphacalc14 */

/*s: function alphacalc2810 */
static Buffer
alphacalc2810(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int fs, sadelta;
    int i, ma, da, q;
    ulong s, t;

    obdst = bdst;
    sadelta = bsrc.alpha == &ones ? 0 : bsrc.delta;
    q = bsrc.delta == 4 && bdst.delta == 4;

    for(i=0; i<dx; i++){
        ma = *bmask.alpha;
        da = *bdst.alpha;
        if(op == SoutD)
            da = 255-da;
        fs = ma;
        if(op != S)
            fs = MUL(fs, da, t);

        if(grey){
            *bdst.grey = MUL(fs, *bsrc.grey, t);
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            if(q){
                *bdst.rgba = MUL0123(fs, *bsrc.rgba, s, t);
                bsrc.rgba++;
                bdst.rgba++;
                bmask.alpha += bmask.delta;
                bdst.alpha += bdst.delta;
                continue;
            }
            *bdst.red = MUL(fs, *bsrc.red, t);
            *bdst.grn = MUL(fs, *bsrc.grn, t);
            *bdst.blu = MUL(fs, *bsrc.blu, t);
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        if(bdst.alpha != &ones){
            *bdst.alpha = MUL(fs, *bsrc.alpha, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        bsrc.alpha += sadelta;
    }
    return obdst;
}
/*e: function alphacalc2810 */

/*s: function alphacalc3679 */
static Buffer
alphacalc3679(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, bool grey, int op)
{
    Buffer obdst;
    int fs, fd, sadelta;
    int i, sa, ma, da, q;
    ulong s, t, u, v;

    obdst = bdst;
    sadelta = (bsrc.alpha == &ones) ? 0 : bsrc.delta;
    q = (bsrc.delta == 4) && (bdst.delta == 4);

    for(i=0; i<dx; i++){
        sa = *bsrc.alpha;
        ma = *bmask.alpha;
        da = *bdst.alpha;
        if(op == SatopD)
            fs = MUL(ma, da, t);
        else
            fs = MUL(ma, 255-da, t);
        if(op == DoverS)
            fd = 255;
        else{
            fd = MUL(sa, ma, t);
            if(op != DatopS)
                fd = 255-fd;
        }

        if(grey){
            *bdst.grey = MUL(fs, *bsrc.grey, s) + MUL(fd, *bdst.grey, t);
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            if(q){
                *bdst.rgba = MUL0123(fs, *bsrc.rgba, s, t)+MUL0123(fd, *bdst.rgba, u, v);
                bsrc.rgba++;
                bdst.rgba++;
                bsrc.alpha += sadelta;
                bmask.alpha += bmask.delta;
                bdst.alpha += bdst.delta;
                continue;
            }
            *bdst.red = MUL(fs, *bsrc.red, s)+MUL(fd, *bdst.red, t);
            *bdst.grn = MUL(fs, *bsrc.grn, s)+MUL(fd, *bdst.grn, t);
            *bdst.blu = MUL(fs, *bsrc.blu, s)+MUL(fd, *bdst.blu, t);
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        if(bdst.alpha != &ones){
            *bdst.alpha = MUL(fs, sa, s)+MUL(fd, da, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        bsrc.alpha += sadelta;
    }
    return obdst;
}
/*e: function alphacalc3679 */

/*s: function alphacalc5 */
static Buffer
alphacalc5(Buffer bdst, Buffer b1, Buffer b2, int dx, int grey, int op)
{
    USED(dx);
    USED(grey);
    USED(op);
    USED(b1);
    USED(b2);

    return bdst;
}
/*e: function alphacalc5 */

/*s: function alphacalc11 */
static Buffer
alphacalc11(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, bool grey, int op)
{
    Buffer obdst;
    int fd, sadelta;
    int sa, ma;
    int i; // iterate over pixels of the line
    bool q;
    ulong s, t, u, v;

    USED(op);
    obdst = bdst;
    sadelta = (bsrc.alpha == &ones) ? 0 : bsrc.delta;
    q = (bsrc.delta == 4) && (bdst.delta == 4);

    // pixel iteration of the line
    for(i=0; i<dx; i++){
        sa = *bsrc.alpha;
        ma = *bmask.alpha;
        fd = 255 - MUL(sa, ma, t);

        /*s: [[alphacalc11()]] if isgrey */
        if(grey){
            *bdst.grey = MUL(ma, *bsrc.grey, s) + MUL(fd, *bdst.grey, t);
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }
        /*e: [[alphacalc11()]] if isgrey */
        else{
            /*s: [[alphacalc11()]] special case for 32bits source and dest */
            if(q){
                *bdst.rgba = MUL0123(ma, *bsrc.rgba, s, t)+MUL0123(fd, *bdst.rgba, u, v);
                bsrc.rgba++;
                bdst.rgba++;
                bsrc.alpha += sadelta;
                bmask.alpha += bmask.delta;
                continue;
            }
            /*e: [[alphacalc11()]] special case for 32bits source and dest */
            // else

            *bdst.red = MUL(ma, *bsrc.red, s)+MUL(fd, *bdst.red, t);
            *bdst.grn = MUL(ma, *bsrc.grn, s)+MUL(fd, *bdst.grn, t);
            *bdst.blu = MUL(ma, *bsrc.blu, s)+MUL(fd, *bdst.blu, t);

            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;

            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }

        if(bdst.alpha != &ones){
            *bdst.alpha = MUL(ma, sa, s) + MUL(fd, *bdst.alpha, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        bsrc.alpha += sadelta;
    }
    return obdst;
}
/*e: function alphacalc11 */

/*s: function alphacalcS */
/* source alpha 1 */
static Buffer
alphacalcS(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int fd;
    int i, ma;
    ulong s, t;

    USED(op);
    obdst = bdst;

    for(i=0; i<dx; i++){
        ma = *bmask.alpha;
        fd = 255-ma;

        if(grey){
            *bdst.grey = MUL(ma, *bsrc.grey, s)+MUL(fd, *bdst.grey, t);
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            *bdst.red = MUL(ma, *bsrc.red, s)+MUL(fd, *bdst.red, t);
            *bdst.grn = MUL(ma, *bsrc.grn, s)+MUL(fd, *bdst.grn, t);
            *bdst.blu = MUL(ma, *bsrc.blu, s)+MUL(fd, *bdst.blu, t);
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        if(bdst.alpha != &ones){
            *bdst.alpha = ma+MUL(fd, *bdst.alpha, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
    }
    return obdst;
}
/*e: function alphacalcS */

/*s: function boolcalc14 */
static Buffer
boolcalc14(Buffer bdst, Buffer b1, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int i, ma, zero;

    USED(b1);

    obdst = bdst;

    for(i=0; i<dx; i++){
        ma = *bmask.alpha;
        zero = ma ? op == DoutS : op == DinS;

        if(grey){
            if(zero)
                *bdst.grey = 0;
            bdst.grey += bdst.delta;
        }else{
            if(zero)
                *bdst.red = *bdst.grn = *bdst.blu = 0;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        if(bdst.alpha != &ones){
            if(zero)
                *bdst.alpha = 0;
            bdst.alpha += bdst.delta;
        }
    }
    return obdst;
}
/*e: function boolcalc14 */

/*s: function boolcalc236789 */
static Buffer
boolcalc236789(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int fs, fd;
    int i, ma, da, zero;
    ulong s, t;

    obdst = bdst;
    zero = !(op&1);

    for(i=0; i<dx; i++){
        ma = *bmask.alpha;
        da = *bdst.alpha;
        fs = da;
        if(op&2)
            fs = 255-da;
        fd = 0;
        if(op&4)
            fd = 255;

        if(grey){
            if(ma)
                *bdst.grey = MUL(fs, *bsrc.grey, s)+MUL(fd, *bdst.grey, t);
            else if(zero)
                *bdst.grey = 0;
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            if(ma){
                *bdst.red = MUL(fs, *bsrc.red, s)+MUL(fd, *bdst.red, t);
                *bdst.grn = MUL(fs, *bsrc.grn, s)+MUL(fd, *bdst.grn, t);
                *bdst.blu = MUL(fs, *bsrc.blu, s)+MUL(fd, *bdst.blu, t);
            }
            else if(zero)
                *bdst.red = *bdst.grn = *bdst.blu = 0;
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        if(bdst.alpha != &ones){
            if(ma)
                *bdst.alpha = fs+MUL(fd, da, t);
            else if(zero)
                *bdst.alpha = 0;
            bdst.alpha += bdst.delta;
        }
    }
    return obdst;
}
/*e: function boolcalc236789 */

/*s: function boolcalc1011 */
static Buffer
boolcalc1011(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int grey, int op)
{
    Buffer obdst;
    int i, ma, zero;

    obdst = bdst;
    zero = !(op&1);

    for(i=0; i<dx; i++){
        ma = *bmask.alpha;

        if(grey){
            if(ma)
                *bdst.grey = *bsrc.grey;
            else if(zero)
                *bdst.grey = 0;
            bsrc.grey += bsrc.delta;
            bdst.grey += bdst.delta;
        }else{
            if(ma){
                *bdst.red = *bsrc.red;
                *bdst.grn = *bsrc.grn;
                *bdst.blu = *bsrc.blu;
            }
            else if(zero)
                *bdst.red = *bdst.grn = *bdst.blu = 0;
            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;
            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        if(bdst.alpha != &ones){
            if(ma)
                *bdst.alpha = 255;
            else if(zero)
                *bdst.alpha = 0;
            bdst.alpha += bdst.delta;
        }
    }
    return obdst;
}
/*e: function boolcalc1011 */
/*s: function replread */
/*
 * Replicated cached scan line read.  Call the function listed in the Param,
 * but cache the result so that for replicated images we only do the work once.
 */
static Buffer
replread(Param *p, uchar *s, int y)
{
    Buffer *b;

    USED(s);
    b = &p->bcache[y];
    if((p->bfilled & (1<<y)) == 0){
        p->bfilled |= 1<<y;
        *b = p->replcall(p, p->bufbase+y*p->bufdelta, y);
    }
    return *b;
}
/*e: function replread */

/*s: function greymaskread */
/*
 * Alpha reading function that simply relabels the grey pointer.
 */
static Buffer
greymaskread(Param *p, uchar *buf, int y)
{
    Buffer b;

    b = p->greymaskcall(p, buf, y);
    b.alpha = b.grey;
    return b;
}
/*e: function greymaskread */

/*s: function readnbit */
static Buffer
readnbit(Param *p, uchar *buf, int y)
{
    Buffer b;
    Memimage *img;
    uchar *repl, *r, *w, *ow, bits;
    int i, n, sh, depth, x, dx, npack, nbits;

    b.rgba = (ulong*)buf;
    b.grey = w = buf;
    b.red = b.blu = b.grn = w;
    b.alpha = &ones;
    b.delta = 1;

    dx = p->dx;
    img = p->img;
    depth = img->depth;
    repl = &replbit[depth][0];
    npack = 8/depth;
    sh = 8-depth;

    /* copy from p->r.min.x until end of repl rectangle */
    x = p->r.min.x;
    n = dx;
    if(n > p->img->r.max.x - x)
        n = p->img->r.max.x - x;

    r = p->bytermin + y*p->bwidth;
    DBG1("readnbit dx %d %p=%p+%d*%d, *r=%d fetch %d ", dx, r, p->bytermin, y, p->bwidth, *r, n);
    bits = *r++;
    nbits = 8;
    if(i=x&(npack-1)){
        DBG1("throwaway %d...", i);
        bits <<= depth*i;
        nbits -= depth*i;
    }
    for(i=0; i<n; i++){
        if(nbits == 0){
            DBG1("(%.2ux)...", *r);
            bits = *r++;
            nbits = 8;
        }
        *w++ = repl[bits>>sh];
        DBG1("bit %x...", repl[bits>>sh]);
        bits <<= depth;
        nbits -= depth;
    }
    dx -= n;
    if(dx == 0)
        return b;

    assert(x+i == p->img->r.max.x);

    /* copy from beginning of repl rectangle until where we were before. */
    x = p->img->r.min.x;
    n = dx;
    if(n > p->r.min.x - x)
        n = p->r.min.x - x;

    r = p->bytey0s + y*p->bwidth;
    DBG1("x=%d r=%p...", x, r);
    bits = *r++;
    nbits = 8;
    if(i=x&(npack-1)){
        bits <<= depth*i;
        nbits -= depth*i;
    }
    DBG1("nbits=%d...", nbits);
    for(i=0; i<n; i++){
        if(nbits == 0){
            bits = *r++;
            nbits = 8;
        }
        *w++ = repl[bits>>sh];
        DBG1("bit %x...", repl[bits>>sh]);
        bits <<= depth;
        nbits -= depth;
        DBG1("bits %x nbits %d...", bits, nbits);
    }
    dx -= n;
    if(dx == 0)
        return b;

    assert(dx > 0);
    /* now we have exactly one full scan line: just replicate the buffer itself until we are done */
    ow = buf;
    while(dx--)
        *w++ = *ow++;

    return b;
}
/*e: function readnbit */

/*s: function writenbit */
static void
writenbit(Param *p, uchar *w, Buffer src)
{
    uchar *r;
    ulong bits;
    int i, sh, depth, npack, nbits, x, ex;

    assert(src.grey != nil && src.delta == 1);

    x = p->r.min.x;
    ex = x+p->dx;
    depth = p->img->depth;
    npack = 8/depth;

    i=x&(npack-1);
    bits = i ? (*w >> (8-depth*i)) : 0;
    nbits = depth*i;
    sh = 8-depth;
    r = src.grey;

    for(; x<ex; x++){
        bits <<= depth;
        DBG1(" %x", *r);
        bits |= (*r++ >> sh);
        nbits += depth;
        if(nbits == 8){
            *w++ = bits;
            nbits = 0;
        }
    }

    if(nbits){
        sh = 8-nbits;
        bits <<= sh;
        bits |= *w & ((1<<sh)-1);
        *w = bits;
    }
    DBG1("\n");
    return;
}
/*e: function writenbit */

/*s: function readcmap */
static Buffer
readcmap(Param *p, uchar *buf, int y)
{
    Buffer b;
    int a, convgrey, copyalpha, dx, i, m;
    uchar *q, *cmap, *begin, *end, *r, *w;

    begin = p->bytey0s + y*p->bwidth;
    r = p->bytermin + y*p->bwidth;
    end = p->bytey0e + y*p->bwidth;

    cmap = p->img->cmap->cmap2rgb;
    convgrey = p->convgrey;
    copyalpha = (p->img->flags&Falpha) ? 1 : 0;

    w = buf;
    dx = p->dx;
    if(copyalpha){
        b.alpha = buf++;
        a = p->img->shift[CAlpha]/8;
        m = p->img->shift[CMap]/8;
        for(i=0; i<dx; i++){
            *w++ = r[a];
            q = cmap+r[m]*3;
            r += 2;
            if(r == end)
                r = begin;
            if(convgrey){
                *w++ = RGB2K(q[0], q[1], q[2]);
            }else{
                *w++ = q[2];	/* blue */
                *w++ = q[1];	/* green */
                *w++ = q[0];	/* red */
            }
        }
    }else{
        b.alpha = &ones;
        for(i=0; i<dx; i++){
            q = cmap+*r++*3;
            if(r == end)
                r = begin;
            if(convgrey){
                *w++ = RGB2K(q[0], q[1], q[2]);
            }else{
                *w++ = q[2];	/* blue */
                *w++ = q[1];	/* green */
                *w++ = q[0];	/* red */
            }
        }
    }

    b.rgba = (ulong*)(buf-copyalpha);

    if(convgrey){
        b.grey = buf;
        b.red = b.blu = b.grn = buf;
        b.delta = 1+copyalpha;
    }else{
        b.blu = buf;
        b.grn = buf+1;
        b.red = buf+2;
        b.grey = nil;
        b.delta = 3+copyalpha;
    }
    return b;
}
/*e: function readcmap */

/*s: function writecmap */
static void
writecmap(Param *p, uchar *w, Buffer src)
{
    uchar *cmap, *red, *grn, *blu;
    int i, dx, delta;

    cmap = p->img->cmap->rgb2cmap;
    
    delta = src.delta;
    red = src.red;
    grn = src.grn;
    blu = src.blu;

    dx = p->dx;
    for(i=0; i<dx; i++, red+=delta, grn+=delta, blu+=delta)
        *w++ = cmap[(*red>>4)*256+(*grn>>4)*16+(*blu>>4)];
}
/*e: function writecmap */

/*s: function readbyte */
static Buffer
readbyte(Param *p, byte *buf, int y)
{
    Memimage *img;
    Buffer b;
    byte *begin, *end, *r;
    byte *w;
    int dx, nb;

    bool isgrey, convgrey;
    bool alphaonly, copyalpha;
    /*s: [[readbyte]] other locals */
    int i;
    byte *rrepl, *grepl, *brepl, *arepl, *krepl;
    byte ured, ugrn, ublu;
    ulong u;
    /*e: [[readbyte]] other locals */

    img = p->img;
    begin = p->bytey0s + y*p->bwidth;
    r = p->bytermin + y*p->bwidth;
    end = p->bytey0e + y*p->bwidth;

    w = buf;
    dx = p->dx;
    nb = img->depth/8;

    convgrey = p->convgrey;	/* convert rgb to grey */
    isgrey   = img->flags&Fgrey;
    copyalpha = img->flags&Falpha;
    alphaonly = p->alphaonly;

    DBG1("copyalpha %d alphaonly %d convgrey %d isgrey %d\n", copyalpha, alphaonly, convgrey, isgrey);

    /*s: [[readbyte()]] simple case when no repl, no grey, depth 8 */
    /* if we can, avoid processing everything */
    if(!(img->flags&Frepl) && !convgrey && (img->flags&Fbytes)){
        memset(&b, 0, sizeof(Buffer));
        if(p->needbuf){
            memmove(buf, r, dx*nb);
            r = buf;
        }
        b.rgba = (ulong*)r;

        if(copyalpha)
            b.alpha = r + img->shift[CAlpha]/8;
        else
            b.alpha = &ones;
        /*s: [[readbyte()]] in simple case, if isgrey */
        if(isgrey){
            b.grey = r + img->shift[CGrey]/8;
            b.red = b.grn = b.blu = b.grey;
        /*e: [[readbyte()]] in simple case, if isgrey */
        }else{
            b.red = r + img->shift[CRed]/8;
            b.grn = r + img->shift[CGreen]/8;
            b.blu = r + img->shift[CBlue]/8;
        }
        b.delta = nb;
        return b;
    }
    /*e: [[readbyte()]] simple case when no repl, no grey, depth 8 */
    // else
    /*s: [[readbyte()]] more complex cases, possible repl, grey, and small depth */
    DBG1("2\n");
    rrepl = replbit[img->nbits[CRed]];
    grepl = replbit[img->nbits[CGreen]];
    brepl = replbit[img->nbits[CBlue]];
    arepl = replbit[img->nbits[CAlpha]];

    krepl = replbit[img->nbits[CGrey]];

    for(i=0; i<dx; i++){
        u = r[0] | (r[1]<<8) | (r[2]<<16) | (r[3]<<24);
        if(copyalpha) {
            *w++ = arepl[(u>>img->shift[CAlpha]) & img->mask[CAlpha]];
            //DBG print("a %x\n", w[-1]);
        }

        if(isgrey)
            *w++ = krepl[(u >> img->shift[CGrey]) & img->mask[CGrey]];
        else if(!alphaonly){
            ured = rrepl[(u >> img->shift[CRed]) & img->mask[CRed]];
            ugrn = grepl[(u >> img->shift[CGreen]) & img->mask[CGreen]];
            ublu = brepl[(u >> img->shift[CBlue]) & img->mask[CBlue]];
            if(convgrey){
                DBG1("g %x %x %x\n", ured, ugrn, ublu);
                *w++ = RGB2K(ured, ugrn, ublu);
                DBG1("%x\n", w[-1]);
            }else{
                *w++ = brepl[(u >> img->shift[CBlue]) & img->mask[CBlue]];
                *w++ = grepl[(u >> img->shift[CGreen]) & img->mask[CGreen]];
                *w++ = rrepl[(u >> img->shift[CRed]) & img->mask[CRed]];
            }
        }
        r += nb;
        if(r == end)
            r = begin;
    }

    b.alpha = copyalpha ? buf : &ones;
    b.rgba = (ulong*)buf;
    if(alphaonly){
        b.red = b.grn = b.blu = b.grey = nil;
        if(!copyalpha)
            b.rgba = nil;
        b.delta = 1;
    }else if(isgrey || convgrey){
        b.grey = buf+copyalpha;
        b.red = b.grn = b.blu = buf+copyalpha;
        b.delta = copyalpha+1;
        DBG1("alpha %x grey %x\n", b.alpha ? *b.alpha : 0xFF, *b.grey);
    }else{
        b.blu = buf+copyalpha;
        b.grn = buf+copyalpha+1;
        b.grey = nil;
        b.red = buf+copyalpha+2;
        b.delta = copyalpha+3;
    }
    return b;
    /*e: [[readbyte()]] more complex cases, possible repl, grey, and small depth */
}
/*e: function readbyte */

/*s: function writebyte */
static void
writebyte(Param *p, byte *w, Buffer src)
{
    Memimage *img;
    int dx, nb;
    byte *red, *grn, *blu, *grey, *alpha;
    int i;

    bool isalpha, isgrey;
    int delta;
    byte ff;
    int adelta;
    ulong u, mask;

    img = p->img;

    red = src.red;
    grn = src.grn;
    blu = src.blu;
    alpha = src.alpha;
    delta = src.delta;
    grey = src.grey;
    dx = p->dx;

    nb = img->depth/8;
    mask = (nb==4) ? 0 : ~((1<<img->depth)-1); // >>

    isalpha = img->flags&Falpha;
    isgrey = img->flags&Fgrey;

    /*s: [[writebyte()]] alpha handling part1 */
    adelta = src.delta;
    if(isalpha && (alpha == nil || alpha == &ones)){
        ff = 0xFF;
        alpha = &ff;
        adelta = 0;
    }
    /*e: [[writebyte()]] alpha handling part1 */

    for(i=0; i<dx; i++){
        u = w[0] | (w[1]<<8) | (w[2]<<16) | (w[3]<<24);
        u &= mask;
        /*s: [[writebyte()]] if isgrey */
        if(isgrey){
            u |= ((*grey >> (8-img->nbits[CGrey])) & img->mask[CGrey]) << img->shift[CGrey];
            grey += delta;
        }
        /*e: [[writebyte()]] if isgrey */
        else{
            u |= ((*red >> (8-img->nbits[CRed])) & img->mask[CRed]) << img->shift[CRed];
            u |= ((*grn >> (8-img->nbits[CGreen])) & img->mask[CGreen]) << img->shift[CGreen];
            u |= ((*blu >> (8-img->nbits[CBlue])) & img->mask[CBlue]) << img->shift[CBlue];
            red += delta;
            grn += delta;
            blu += delta;
        }
        /*s: [[writebyte()]] alpha handling part2 */
        if(isalpha){
            u |= ((*alpha >> (8-img->nbits[CAlpha])) & img->mask[CAlpha]) << img->shift[CAlpha];
            alpha += adelta;
        }
        /*e: [[writebyte()]] alpha handling part2 */

        w[0] = u;
        w[1] = u>>8;
        w[2] = u>>16;
        w[3] = u>>24;
        w += nb;
    }
}
/*e: function writebyte */

/*s: function readfn */
static Readfn*
readfn(Memimage *img)
{
    /*s: [[readfn()]] if depth less than 8 */
    if(img->depth < 8)
        return readnbit;
    /*e: [[readfn()]] if depth less than 8 */
    /*s: [[readfn()]] if cmap */
    if(img->nbits[CMap] == 8)
        return readcmap;
    /*e: [[readfn()]] if cmap */
    return readbyte;
}
/*e: function readfn */

/*s: function readalphafn */
static Readfn*
readalphafn(Memimage *m)
{
    USED(m);
    return readbyte;
}
/*e: function readalphafn */

/*s: function writefn */
static Writefn*
writefn(Memimage *img)
{
    /*s: [[writefn()]] if depth less than 8 */
    if(img->depth < 8)
        return writenbit;
    /*e: [[writefn()]] if depth less than 8 */
    /*s: [[writefn()]] if cmap */
    if(img->chan == CMAP8)
        return writecmap;
    /*e: [[writefn()]] if cmap */
    return writebyte;
}
/*e: function writefn */

/*s: function nullwrite */
static void
nullwrite(Param *p, uchar *s, Buffer b)
{
    USED(p);
    USED(s);
    USED(b);
}
/*e: function nullwrite */

/*s: function readptr */
static Buffer
readptr(Param *p, uchar *s, int y)
{
    Buffer b;
    uchar *q;

    USED(s);
    q = p->bytermin + y*p->bwidth;
    b.red = q;	/* ptr to data */
    b.grn = b.blu = b.grey = b.alpha = nil;
    b.rgba = (ulong*)q;
    b.delta = p->img->depth/8;
    return b;
}
/*e: function readptr */

/*s: function boolmemmove */
static Buffer
boolmemmove(Buffer bdst, Buffer bsrc, Buffer b1, int dx, int i, int o)
{
    USED(i);
    USED(o);
    USED(b1);
    USED(bsrc);
    memmove(bdst.red, bsrc.red, dx*bdst.delta);
    return bdst;
}
/*e: function boolmemmove */

/*s: function boolcopy8 */
static Buffer
boolcopy8(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int i, int o)
{
    uchar *m, *r, *w, *ew;

    USED(i);
    USED(o);
    m = bmask.grey;
    w = bdst.red;
    r = bsrc.red;
    ew = w+dx;
    for(; w < ew; w++,r++)
        if(*m++)
            *w = *r;
    return bdst;	/* not used */
}
/*e: function boolcopy8 */

/*s: function boolcopy16 */
static Buffer
boolcopy16(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int i, int o)
{
    uchar *m;
    ushort *r, *w, *ew;

    USED(i);
    USED(o);
    m = bmask.grey;
    w = (ushort*)bdst.red;
    r = (ushort*)bsrc.red;
    ew = w+dx;
    for(; w < ew; w++,r++)
        if(*m++)
            *w = *r;
    return bdst;	/* not used */
}
/*e: function boolcopy16 */

/*s: function boolcopy24 */
static Buffer
boolcopy24(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int i, int o)
{
    uchar *m;
    uchar *r, *w, *ew;

    USED(i);
    USED(o);
    m = bmask.grey;
    w = bdst.red;
    r = bsrc.red;
    ew = w+dx*3;
    while(w < ew){
        if(*m++){
            *w++ = *r++;
            *w++ = *r++;
            *w++ = *r++;
        }else{
            w += 3;
            r += 3;
        }
    }
    return bdst;	/* not used */
}
/*e: function boolcopy24 */

/*s: function boolcopy32 */
static Buffer
boolcopy32(Buffer bdst, Buffer bsrc, Buffer bmask, int dx, int i, int o)
{
    uchar *m;
    ulong *r, *w, *ew;

    USED(i);
    USED(o);
    m = bmask.grey;
    w = (ulong*)bdst.red;
    r = (ulong*)bsrc.red;
    ew = w+dx;
    for(; w < ew; w++,r++)
        if(*m++)
            *w = *r;
    return bdst;	/* not used */
}
/*e: function boolcopy32 */

/*s: function genconv */
static Buffer
genconv(Param *p, uchar *buf, int y)
{
    Buffer b;
    int nb;
    uchar *r, *w, *ew;

    /* read from source into RGB format in convbuf */
    b = p->convreadcall(p, p->convbuf, y);

    /* write RGB format into dst format in buf */
    p->convwritecall(p->convdpar, buf, b);

    if(p->convdx){
        nb = p->convdpar->img->depth/8;
        r = buf;
        w = buf+nb*p->dx;
        ew = buf+nb*p->convdx;
        while(w<ew)
            *w++ = *r++;
    }

    b.red = buf;
    b.blu = b.grn = b.grey = b.alpha = nil;
    b.rgba = (ulong*)buf;
    b.delta = 0;
    
    return b;
}
/*e: function genconv */

/*s: function convfn */
static Readfn*
convfn(Memimage *dst, Param *dpar, Memimage *src, Param *spar, int *ndrawbuf)
{
    if(dst->chan == src->chan && !(src->flags&Frepl)){
        DBG1("readptr...");
        return readptr;
    }

    if(dst->chan==CMAP8 && (src->chan==GREY1||src->chan==GREY2||src->chan==GREY4)){
        /* cheat because we know the replicated value is exactly the color map entry. */
        DBG1("Readnbit...");
        return readnbit;
    }

    spar->convreadcall = readfn(src);
    spar->convwritecall = writefn(dst);
    spar->convdpar = dpar;

    /* allocate a conversion buffer */
    spar->convbufoff = *ndrawbuf;
    *ndrawbuf += spar->dx*4;

    if(spar->dx > Dx(spar->img->r)){
        spar->convdx = spar->dx;
        spar->dx = Dx(spar->img->r);
    }

    DBG1("genconv...");
    return genconv;
}
/*e: function convfn */

/*s: function pixelbits */
static ulong
pixelbits(Memimage *i, Point pt)
{
    byte *p;
    ulong val = 0;
    int bpp = i->depth; // bits per pixel
    /*s: [[pixelbits()]] other locals */
    int off, npack;
    /*e: [[pixelbits()]] other locals */

    p = byteaddr(i, pt);

    switch(bpp){
    /*s: [[pixelbits()]] switch bpp cases */
    case 1:
    case 2:
    case 4:
        npack = 8/bpp;
        off = pt.x % npack;
        val = p[0] >> bpp*(npack-1-off);
        val &= (1<<bpp)-1;
        break;
    /*x: [[pixelbits()]] switch bpp cases */
    case 16:
        val = p[0]|(p[1]<<8);
        break;
    /*x: [[pixelbits()]] switch bpp cases */
    case 24:
        val = p[0]|(p[1]<<8)|(p[2]<<16);
        break;
    /*e: [[pixelbits()]] switch bpp cases */
    case 8:
        val = p[0];
        break;
    case 32:
        val = p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24);
        break;
    }
    // duplicate byte in the whole word
    while(bpp<32){
        val |= val<<bpp;
        bpp *= 2;
    }
    return val;
}
/*e: function pixelbits */

/*s: function boolcopyfn */
static Calcfn*
boolcopyfn(Memimage *img, Memimage *mask)
{
    if(mask->flags&Frepl && Dx(mask->r)==1 && Dy(mask->r)==1 && pixelbits(mask, mask->r.min)==~0)
        return boolmemmove;

    switch(img->depth){
    case 8:
        return boolcopy8;
    case 16:
        return boolcopy16;
    case 24:
        return boolcopy24;
    case 32:
        return boolcopy32;
    default:
        assert(0 /* boolcopyfn */);
    }
    return nil;
}
/*e: function boolcopyfn */

/*s: function memsets */
static void
memsets(void *vp, ushort val, int n)
{
    ushort *p, *ep;

    p = vp;
    ep = p+n;
    while(p<ep)
        *p++ = val;
}
/*e: function memsets */

/*s: function memsetl */
static void
memsetl(void *vp, ulong val, int n)
{
    ulong *p, *ep;

    p = vp;
    ep = p+n;
    while(p < ep)
        *p++ = val;
}
/*e: function memsetl */

/*s: function memset24 */
static void
memset24(void *vp, ulong val, int n)
{
    uchar *p, *ep;
    uchar a,b,c;

    p = vp;
    ep = p+3*n;

    a = val;
    b = val>>8;
    c = val>>16;
    while(p<ep){
        *p++ = a;
        *p++ = b;
        *p++ = c;
    }
}
/*e: function memset24 */

/*s: function imgtorgba */
static ulong
imgtorgba(Memimage *img, ulong val)
{
    byte r, g, b, a;
    ulong chan;
    byte *p;
    int nb, v, ov;

    a = 0xFF;
    r = g = b = 0xAA;	/* garbage */

    for(chan=img->chan; chan; chan>>=8){
        nb = NBITS(chan);
        v = val&((1<<nb)-1);
        val >>= nb;
        ov = v;
        /*s: [[imgtorgba()]] duplicate bits if nb less than 8 */
        // duplicate bits
        while(nb < 8){
            v |= v<<nb;
            nb *= 2;
        }
        v >>= (nb-8);
        /*e: [[imgtorgba()]] duplicate bits if nb less than 8 */

        switch(TYPE(chan)){
        case CRed:
            r = v;
            break;
        case CGreen:
            g = v;
            break;
        case CBlue:
            b = v;
            break;
        case CAlpha:
            a = v;
            break;
        /*s: [[imgtorgba()]] switch chan type cases */
        case CGrey:
            r = g = b = v;
            break;
        /*x: [[imgtorgba()]] switch chan type cases */
        case CMap:
            p = img->cmap->cmap2rgb + 3*ov;
            r = *p++;
            g = *p++;	
            b = *p;
            break;
        /*e: [[imgtorgba()]] switch chan type cases */
        }
    }
    return (r<<24)|(g<<16)|(b<<8)|a;	
}
/*e: function imgtorgba */

/*s: function rgbatoimg */
static ulong
rgbatoimg(Memimage *img, ulong rgba)
{
    byte r, g, b, a;
    ulong chan;
    int d, nb;
    ulong v;
    byte *p;
    byte m;

    v = 0;
    r = rgba>>24;
    g = rgba>>16;
    b = rgba>>8;
    a = rgba;
    d = 0;
    for(chan=img->chan; chan; chan>>=8){
        nb = NBITS(chan);
        switch(TYPE(chan)){
        case CRed:
            v |= (r>>(8-nb))<<d;
            break;
        case CGreen:
            v |= (g>>(8-nb))<<d;
            break;
        case CBlue:
            v |= (b>>(8-nb))<<d;
            break;
        case CAlpha:
            v |= (a>>(8-nb))<<d;
            break;
        /*s: [[rgbatoimg()]] switch chan type cases */
        case CGrey:
            m = RGB2K(r,g,b);
            v |= (m>>(8-nb))<<d;
            break;
        /*x: [[rgbatoimg()]] switch chan type cases */
        case CMap:
            p = img->cmap->rgb2cmap;
            m = p[(r>>4)*256+(g>>4)*16+(b>>4)];
            v |= (m>>(8-nb))<<d;
            break;
        /*e: [[rgbatoimg()]] switch chan type cases */
        }
        d += nb;
    }
    DBG1("rgba2img %.8lux = %.*lux\n", rgba, 2*d/8, v);
    return v;
}
/*e: function rgbatoimg */

/*s: function memoptdraw */
static bool
memoptdraw(Memdrawparam *par)
{
    int dx, dy;
    // enum<Drawop>
    int op;
    Memimage *src, *dst;

    int m, y;
    ulong v;

    dx = Dx(par->r);
    dy = Dy(par->r);
    src = par->src;
    dst = par->dst;
    op = par->op;

    DBG1("state %lux mval %lux dd %d\n", par->state, par->mval, dst->depth);

    /*s: [[memoptdraw()]] if condition for memset */
    /*
     * If we have an opaque mask and source is one opaque pixel we can
     * convert to the destination format and just replicate with memset.
     */
    m = Simplesrc|Simplemask|Fullmask;
    if((par->state&m)==m && 
       (par->srgba&0xFF) == 0xFF && 
       (op == S || op == SoverD)){
        byte *dp;
        byte p[4];
        int dwid;
        /*s: [[memoptdraw()]] locals for memset case */
        int ppb, np, nb;
        uchar lm, rm;
        int d;
        /*e: [[memoptdraw()]] locals for memset case */

        DBG1("memopt, dst %p, dst->data->bdata %p\n", dst, dst->data->bdata);
        dwid = dst->width * sizeof(ulong);
        dp = byteaddr(dst, par->r.min);
        v = par->sdval;
        DBG1("sdval %lud, depth %d\n", v, dst->depth);

        switch(dst->depth){
        /*s: [[memoptdraw()]] switch depth of dst cases */
        case 1:
        case 2:
        case 4:
            for(d=dst->depth; d<8; d*=2)
                v |= (v<<d);
            ppb = 8/dst->depth;	/* pixels per byte */
            m = ppb-1;
            /* left edge */
            np = par->r.min.x&m;		/* no. pixels unused on left side of word */
            dx -= (ppb-np);
            nb = 8 - np * dst->depth;		/* no. bits used on right side of word */
            lm = (1<<nb)-1;
            DBG1("np %d x %d nb %d lm %ux ppb %d m %ux\n", np, par->r.min.x, nb, lm, ppb, m);	

            /* right edge */
            np = par->r.max.x&m;	/* no. pixels used on left side of word */
            dx -= np;
            nb = 8 - np * dst->depth;		/* no. bits unused on right side of word */
            rm = ~((1<<nb)-1);
            DBG1("np %d x %d nb %d rm %ux ppb %d m %ux\n", np, par->r.max.x, nb, rm, ppb, m);	

            DBG1("dx %d Dx %d\n", dx, Dx(par->r));
            /* lm, rm are masks that are 1 where we should touch the bits */
            if(dx < 0){	/* just one byte */
                lm &= rm;
                for(y=0; y<dy; y++, dp+=dwid)
                    *dp ^= (v ^ *dp) & lm;
            }else if(dx == 0){	/* no full bytes */
                if(lm)
                    dwid--;

                for(y=0; y<dy; y++, dp+=dwid){
                    if(lm){
                      DBG1("dp %p v %lux lm %ux (v ^ *dp) & lm %lux\n", dp, v, lm, (v^*dp)&lm);
                        *dp ^= (v ^ *dp) & lm;
                        dp++;
                    }
                    *dp ^= (v ^ *dp) & rm;
                }
            }else{		/* full bytes in middle */
                dx /= ppb;
                if(lm)
                    dwid--;
                dwid -= dx;

                for(y=0; y<dy; y++, dp+=dwid){
                    if(lm){
                        *dp ^= (v ^ *dp) & lm;
                        dp++;
                    }
                    memset(dp, v, dx);
                    dp += dx;
                    *dp ^= (v ^ *dp) & rm;
                }
            }
            return true;
        /*x: [[memoptdraw()]] switch depth of dst cases */
        default:
            assert(0 /* bad dest depth in memoptdraw */);
        /*x: [[memoptdraw()]] switch depth of dst cases */
        case 8:
            for(y=0; y < dy; y++, dp += dwid)
                memset(dp, v, dx);
            return true;
        /*x: [[memoptdraw()]] switch depth of dst cases */
        case 16:
            p[0] = v;		/* make little endian */
            p[1] = v>>8;
            v = *(ushort*)p;
            DBG("dp=%p; dx=%d; for(y=0; y<%d; y++, dp+=%d)\nmemsets(dp, v, dx);\n",         	dp, dx, dy, dwid);
            for(y=0; y<dy; y++, dp+=dwid)
                memsets(dp, v, dx);
            return 1;
        /*x: [[memoptdraw()]] switch depth of dst cases */
        case 24:
            for(y=0; y<dy; y++, dp+=dwid)
                memset24(dp, v, dx);
            return 1;
        /*e: [[memoptdraw()]] switch depth of dst cases */
        case 32:
            p[0] = v;		/* make little endian */
            p[1] = v>>8;
            p[2] = v>>16;
            p[3] = v>>24;
            v = *(ulong*)p;

            for(y=0; y < dy; y++, dp += dwid)
                memsetl(dp, v, dx);
            return true;
        }
    }
    /*e: [[memoptdraw()]] if condition for memset */
    /*s: [[memoptdraw()]] if condition for memmove */
    /*
     * If no source alpha, an opaque mask, we can just copy the
     * source onto the destination.  If the channels are the same and
     * the source is not replicated, memmove suffices.
     */
    m = Simplemask|Fullmask;
    if((par->state&(m|Replsrc))==m && 
        src->depth >= 8 && 
        src->chan == dst->chan && 
        !(src->flags&Falpha) && 
        (op == S || op == SoverD)){

        byte *sp, *dp;
        long swid, dwid, nb;
        int dir;

        if(src->data == dst->data && 
           byteaddr(dst, par->r.min) > byteaddr(src, par->sr.min))
            dir = -1;
        else
            dir = 1;

        swid = src->width*sizeof(ulong);
        dwid = dst->width*sizeof(ulong);
        sp = byteaddr(src, par->sr.min);
        dp = byteaddr(dst, par->r.min);
        if(dir == -1){
            sp += (dy-1)*swid;
            dp += (dy-1)*dwid;
            swid = -swid;
            dwid = -dwid;
        }
        nb = (dx*src->depth)/8;
        for(y=0; y<dy; y++, sp+=swid, dp+=dwid)
            memmove(dp, sp, nb);
        return true;
    }
    /*e: [[memoptdraw()]] if condition for memmove */
    /*s: [[memoptdraw()]] if 1 bit mask, src, and dest */
    /*
     * If we have a 1-bit mask, 1-bit source, and 1-bit destination, and
     * they're all bit aligned, we can just use bit operators.  This happens
     * when we're manipulating boolean masks, e.g. in the arc code.
     */
    if((par->state&(Simplemask|Simplesrc|Replmask|Replsrc))==0 
    && dst->chan==GREY1 
    && src->chan==GREY1 
    && par->mask->chan==GREY1 
    && (par->r.min.x&7)==(par->sr.min.x&7) 
    && (par->r.min.x&7)==(par->mr.min.x&7)){
        uchar *sp, *dp, *mp;
        uchar lm, rm;
        long swid, dwid, mwid;
        int i, x, dir;

        sp = byteaddr(src, par->sr.min);
        dp = byteaddr(dst, par->r.min);
        mp = byteaddr(par->mask, par->mr.min);
        swid = src->width*sizeof(ulong);
        dwid = dst->width*sizeof(ulong);
        mwid = par->mask->width*sizeof(ulong);

        if(src->data == dst->data 
        && byteaddr(dst, par->r.min) > byteaddr(src, par->sr.min)){
            dir = -1;
        }else
            dir = 1;

        lm = 0xFF>>(par->r.min.x&7);
        rm = 0xFF<<(8-(par->r.max.x&7));
        dx -= (8-(par->r.min.x&7)) + (par->r.max.x&7);

        if(dx < 0){	/* one byte wide */
            lm &= rm;
            if(dir == -1){
                dp += dwid*(dy-1);
                sp += swid*(dy-1);
                mp += mwid*(dy-1);
                dwid = -dwid;
                swid = -swid;
                mwid = -mwid;
            }
            for(y=0; y<dy; y++){
                *dp ^= (*dp ^ *sp) & *mp & lm;
                dp += dwid;
                sp += swid;
                mp += mwid;
            }
            return 1;
        }

        dx /= 8;
        if(dir == 1){
            i = (lm!=0)+dx+(rm!=0);
            mwid -= i;
            swid -= i;
            dwid -= i;
            for(y=0; y<dy; y++, dp+=dwid, sp+=swid, mp+=mwid){
                if(lm){
                    *dp ^= (*dp ^ *sp++) & *mp++ & lm;
                    dp++;
                }
                for(x=0; x<dx; x++){
                    *dp ^= (*dp ^ *sp++) & *mp++;
                    dp++;
                }
                if(rm){
                    *dp ^= (*dp ^ *sp++) & *mp++ & rm;
                    dp++;
                }
            }
            return 1;
        }else{
        /* dir == -1 */
            i = (lm!=0)+dx+(rm!=0);
            dp += dwid*(dy-1)+i-1;
            sp += swid*(dy-1)+i-1;
            mp += mwid*(dy-1)+i-1;
            dwid = -dwid+i;
            swid = -swid+i;
            mwid = -mwid+i;
            for(y=0; y<dy; y++, dp+=dwid, sp+=swid, mp+=mwid){
                if(rm){
                    *dp ^= (*dp ^ *sp--) & *mp-- & rm;
                    dp--;
                }
                for(x=0; x<dx; x++){
                    *dp ^= (*dp ^ *sp--) & *mp--;
                    dp--;
                }
                if(lm){
                    *dp ^= (*dp ^ *sp--) & *mp-- & lm;
                    dp--;
                }
            }
        }
        return 1;
    }
    /*e: [[memoptdraw()]] if 1 bit mask, src, and dest */
    return false;	
}
/*e: function memoptdraw */

/*s: function chardraw */
/*
 * Boolean character drawing.
 * Solid opaque color through a 1-bit greyscale mask.
 */
static bool
chardraw(Memdrawparam *par)
{
    Rectangle r, mr;
    Memimage *mask, *src, *dst;
    int op;
    int dx, dy;

    ulong bits;
    int i, ddepth, x, bx, ex, y, npack, bsh, depth;
    ulong v, maskwid, dstwid;
    uchar *wp, *rp, *q, *wc;
    ushort *ws;
    ulong *wl;
    uchar sp[4];

    DBG1("chardraw? mf %lux md %d sf %lux dxs %d dys %d dd %d ddat %p sdat %p\n",
        par->mask->flags, par->mask->depth, par->src->flags, 
        Dx(par->src->r), Dy(par->src->r), par->dst->depth, par->dst->data, par->src->data);

    mask = par->mask;
    src = par->src;
    dst = par->dst;
    r = par->r;
    mr = par->mr;
    op = par->op;

    if((par->state&(Replsrc|Simplesrc|Replmask)) != (Replsrc|Simplesrc)
    || mask->depth != 1 
    || src->flags&Falpha 
    || dst->depth<8 
    || dst->data==src->data
    || op != SoverD)
        return false;

    // else

    DBG1("chardraw...");

    depth = mask->depth;
    maskwid = mask->width * sizeof(ulong);
    rp = byteaddr(mask, mr.min);
    npack = 8/depth;
    bsh = (mr.min.x % npack) * depth;

    wp = byteaddr(dst, r.min);
    dstwid = dst->width*sizeof(ulong);
    DBG1("bsh %d\n", bsh);
    dy = Dy(r);
    dx = Dx(r);

    ddepth = dst->depth;

    /*
     * for loop counts from bsh to bsh+dx
     *
     * we want the bottom bits to be the amount
     * to shift the pixels down, so for n≡0 (mod 8) we want 
     * bottom bits 7.  for n≡1, 6, etc.
     * the bits come from -n-1.
     */

    bx = -bsh-1;
    ex = -bsh-1-dx;
    SET(bits);
    v = par->sdval;

    /* make little endian */
    sp[0] = v;
    sp[1] = v>>8;
    sp[2] = v>>16;
    sp[3] = v>>24;

    DBG1("sp %x %x %x %x\n", sp[0], sp[1], sp[2], sp[3]);
    for(y=0; y<dy; y++, rp+=maskwid, wp+=dstwid){
        q = rp;
        if(bsh)
            bits = *q++;
        switch(ddepth){
        /*s: [[chardraw()]] switch depth cases */
        case 8:
            DBG1("8loop...");
            wc = wp;
            for(x=bx; x>ex; x--, wc++){
                i = x&7;
                if(i == 8-1)
                    bits = *q++;
                DBG1("bits %lux sh %d...", bits, i);
                if((bits>>i)&1)
                    *wc = v;
            }
            break;
        /*x: [[chardraw()]] switch depth cases */
        case 16:
            ws = (ushort*)wp;
            v = *(ushort*)sp;
            for(x=bx; x>ex; x--, ws++){
                i = x&7;
                if(i == 8-1)
                    bits = *q++;
                DBG1("bits %lux sh %d...", bits, i);
                if((bits>>i)&1)
                    *ws = v;
            }
            break;
        /*x: [[chardraw()]] switch depth cases */
        case 24:
            wc = wp;
            for(x=bx; x>ex; x--, wc+=3){
                i = x&7;
                if(i == 8-1)
                    bits = *q++;
                DBG1("bits %lux sh %d...", bits, i);
                if((bits>>i)&1){
                    wc[0] = sp[0];
                    wc[1] = sp[1];
                    wc[2] = sp[2];
                }
            }
            break;
        /*e: [[chardraw()]] switch depth cases */
        case 32:
            wl = (ulong*)wp;
            v = *(ulong*)sp;
            for(x=bx; x>ex; x--, wl++){
                i = x&7;
                if(i == 8-1)
                    bits = *q++;
                DBG1("bits %lux sh %d...", bits, i);
                if((bits>>i)&1)
                    *wl = v;
            }
            break;
        }
    }

    DBG1("\n");	
    return 1;	
}
/*e: function chardraw */

/*s: function memfillcolor */
void
memfillcolor(Memimage *i, ulong val)
{
    ulong bits;
    int d, y;

    if(val == DNofill)
        return;

    bits = rgbatoimg(i, val);
    switch(i->depth){
    /*s: [[memfillcolor()]] switch depth cases */
    case 24:	/* 24-bit images suck */
        for(y=i->r.min.y; y<i->r.max.y; y++)
            memset24(byteaddr(i, Pt(i->r.min.x, y)), bits, Dx(i->r));
        break;
    /*e: [[memfillcolor()]] switch depth cases */
    default:	/* 1, 2, 4, 8, 16, 32 */
        for(d=i->depth; d<32; d*=2)
            // duplicate bits
            bits = (bits << d) | bits;

        memsetl(wordaddr(i, i->r.min), bits, i->width * Dy(i->r));
        break;
    }
}
/*e: function memfillcolor */

/*e: lib_graphics/libmemdraw/draw.c */
