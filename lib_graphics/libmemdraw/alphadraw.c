/*s: lib_graphics/libmemdraw/alphadraw.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define DBG1 if(0) print

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

    ulong	*rgba; // bad name, just start of pixel
    int	delta;	/* number of bytes to add to pointer to get next pixel to the right */

    byte	*grey;

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

    // source bytes, always same y but x varies
    byte	*bytey0s;	/* byteaddr(Pt(img->r.min.x, img->r.min.y)) */
    byte	*bytermin;	/* byteaddr(Pt(r.min.x,      img->r.min.y)) */
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
    bool	convgrey;
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
    alphacalc11,        /* SoverD */ // the classic
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
    p->bytermin = byteaddr(img, Pt(r.min.x,      img->r.min.y));
    p->bytey0e  = byteaddr(img, Pt(img->r.max.x, img->r.min.y));

    assert(p->bytey0s <= p->bytermin && p->bytermin <= p->bytey0e);
    if(p->r.min.x == p->img->r.min.x)
        assert(p->bytermin == p->bytey0s);

    p->bwidth   = sizeof(ulong) * img->width;

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

    // *y = *y % Dy(img->r)

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
int
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

    /*s: [[alphadraw()]] setting srcy, masky, and dsty, and clipping part1 */
    /*
     * srcy, masky, and dsty are offsets from the top of their
     * respective Rectangles.  they need to be contained within
     * the rectangles, so clipy can keep them there without division.
     */
    srcy  = (starty + sr.min.y - src->r.min.y)  % Dy(src->r);
    masky = (starty + mr.min.y - mask->r.min.y) % Dy(mask->r);
    dsty  =  starty + r.min.y  - dst->r.min.y;
    /*e: [[alphadraw()]] setting srcy, masky, and dsty, and clipping part1 */
    assert(0 <= srcy  && srcy  < Dy(src->r));
    assert(0 <= masky && masky < Dy(mask->r));
    assert(0 <= dsty  && dsty  < Dy(dst->r));

    // the big loop!
    for(y=starty; y!=endy; y+=dir, srcy+=dir, masky+=dir, dsty+=dir){
        /*s: [[alphadraw()]] clipping part2 */
        clipy(src, &srcy);
        clipy(dst, &dsty);
        clipy(mask, &masky);
        /*e: [[alphadraw()]] clipping part2 */

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
    int sa, ma;
    int fd;
    int sadelta;
    int i; // iterate over pixels of the line
    bool q;
    // temporaries for MUL
    ulong s, t, u, v;
    // ??
    Buffer obdst;

    USED(op);
    obdst = bdst;
    /*s: [[alphacalc11()]] alpha handling part1 */
    sadelta = (bsrc.alpha == &ones) ? 0 : bsrc.delta;
    /*e: [[alphacalc11()]] alpha handling part1 */
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

            *bdst.red = MUL(ma, *bsrc.red, s) + MUL(fd, *bdst.red, t);
            *bdst.grn = MUL(ma, *bsrc.grn, s) + MUL(fd, *bdst.grn, t);
            *bdst.blu = MUL(ma, *bsrc.blu, s) + MUL(fd, *bdst.blu, t);

            bsrc.red += bsrc.delta;
            bsrc.blu += bsrc.delta;
            bsrc.grn += bsrc.delta;

            bdst.red += bdst.delta;
            bdst.blu += bdst.delta;
            bdst.grn += bdst.delta;
        }

        /*s: [[alphacalc11()]] alpha handling part2 */
        if(bdst.alpha != &ones){
            *bdst.alpha = MUL(ma, sa, s) + MUL(fd, *bdst.alpha, t);
            bdst.alpha += bdst.delta;
        }
        bmask.alpha += bmask.delta;
        bsrc.alpha += sadelta;
        /*e: [[alphacalc11()]] alpha handling part2 */
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

/*s: global replbit */
/*
 * Conversion tables.
 */
uchar replbit[1+8][256];		/* replbit[x][y] is the replication of the x-bit quantity y to 8-bit depth */
/*e: global replbit */


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
    byte *begin, *end, *r;
    Buffer b;
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
    begin = p->bytey0s  + y*p->bwidth;
    r     = p->bytermin + y*p->bwidth;
    end   = p->bytey0e  + y*p->bwidth;

    w = buf;
    dx = p->dx;
    nb = img->depth/8;

    convgrey = p->convgrey;	/* convert rgb to grey */
    isgrey   = img->flags&Fgrey;
    copyalpha = img->flags&Falpha;
    alphaonly = p->alphaonly;

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
    int delta;
    // will iterate over all the pixels in one line in src
    int i;

    bool isalpha, isgrey;
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

// in resolution.c now
extern ulong pixelbits(Memimage *i, Point pt);

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


/*e: lib_graphics/libmemdraw/alphadraw.c */
