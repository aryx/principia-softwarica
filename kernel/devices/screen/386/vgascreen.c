/*s: kernel/devices/screen/386/vgascreen.c */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"
#include <ureg.h>

#include <draw.h>
#include <memdraw.h>
#include <cursor.h>

#include "../port/screen.h"
#include "vga.h"

//---------------------------------------------------------------------------
// vgax.c
//---------------------------------------------------------------------------

/*s: global vgaxlock */
static Lock vgaxlock;           /* access to index registers */
/*e: global vgaxlock */

/*s: function vgaxi */
int
vgaxi(long port, uchar index)
{
    uchar data;

    ilock(&vgaxlock);
    switch(port){

    case Seqx:
    case Crtx:
    case Grx:
        outb(port, index);
        data = inb(port+1);
        break;

    case Attrx:
        /*
         * Allow processor access to the colour
         * palette registers. Writes to Attrx must
         * be preceded by a read from Status1 to
         * initialise the register to point to the
         * index register and not the data register.
         * Processor access is allowed by turning
         * off bit 0x20.
         */
        inb(Status1);
        if(index < 0x10){
            outb(Attrx, index);
            data = inb(Attrx+1);
            inb(Status1);
            outb(Attrx, 0x20|index);
        }
        else{
            outb(Attrx, 0x20|index);
            data = inb(Attrx+1);
        }
        break;

    default:
        iunlock(&vgaxlock);
        return -1;
    }
    iunlock(&vgaxlock);

    return data & 0xFF;
}
/*e: function vgaxi */

/*s: function vgaxo */
int
vgaxo(long port, uchar index, uchar data)
{
    ilock(&vgaxlock);
    switch(port){

    case Seqx:
    case Crtx:
    case Grx:
        /*
         * We could use an outport here, but some chips
         * (e.g. 86C928) have trouble with that for some
         * registers.
         */
        outb(port, index);
        outb(port+1, data);
        break;

    case Attrx:
        inb(Status1);
        if(index < 0x10){
            outb(Attrx, index);
            outb(Attrx, data);
            inb(Status1);
            outb(Attrx, 0x20|index);
        }
        else{
            outb(Attrx, 0x20|index);
            outb(Attrx, data);
        }
        break;

    default:
        iunlock(&vgaxlock);
        return -1;
    }
    iunlock(&vgaxlock);

    return 0;
}
/*e: function vgaxo */

//---------------------------------------------------------------------------
// vga.c
//---------------------------------------------------------------------------

/*s: global back2 */
static Memimage* back;
/*e: global back2 */
/*s: global conscol */
static Memimage *conscol;
/*e: global conscol */

/*s: global curpos */
static Point curpos;
/*e: global curpos */
/*s: global window bis */
static Rectangle window;
/*e: global window bis */
/*s: global xp */
static int *xp;
/*e: global xp */
/*s: global xbuf */
static int xbuf[256];
/*e: global xbuf */
/*s: global vgascreenlock */
Lock vgascreenlock;
/*e: global vgascreenlock */
/*s: function vgaimageinit */
//int drawdebug;

void
vgaimageinit(ulong chan)
{
    if(back == nil){
        back = allocmemimage(Rect(0,0,1,1), chan);  /* RSC BUG */
        if(back == nil)
            panic("back alloc");        /* RSC BUG */
        back->flags |= Frepl;
        back->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
        memfillcolor(back, DBlack);
    }

    if(conscol == nil){
        conscol = allocmemimage(Rect(0,0,1,1), chan);   /* RSC BUG */
        if(conscol == nil)
            panic("conscol alloc"); /* RSC BUG */
        conscol->flags |= Frepl;
        conscol->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
        memfillcolor(conscol, DWhite);
    }
}
/*e: function vgaimageinit */

/*s: function vgascroll */
static void
vgascroll(VGAscr* scr)
{
    int h, o;
    Point p;
    Rectangle r;

    h = scr->memdefont->height;
    o = 8*h;
    r = Rpt(window.min, Pt(window.max.x, window.max.y-o));
    p = Pt(window.min.x, window.min.y+o);
    memimagedraw(scr->gscreen, r, scr->gscreen, p, nil, p, S);
    r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
    memimagedraw(scr->gscreen, r, back, ZP, nil, ZP, S);

    curpos.y -= o;
}
/*e: function vgascroll */

/*s: function vgascreenputc */
static void
vgascreenputc(VGAscr* scr, char* buf, Rectangle *flushr)
{
    Point p;
    int h, w, pos;
    Rectangle r;

//  drawdebug = 1;
    if(xp < xbuf || xp >= &xbuf[sizeof(xbuf)])
        xp = xbuf;

    h = scr->memdefont->height;
    switch(buf[0]){

    case '\n':
        if(curpos.y+h >= window.max.y){
            vgascroll(scr);
            *flushr = window;
        }
        curpos.y += h;
        vgascreenputc(scr, "\r", flushr);
        break;

    case '\r':
        xp = xbuf;
        curpos.x = window.min.x;
        break;

    case '\t':
        p = memsubfontwidth(scr->memdefont, " ");
        w = p.x;
        if(curpos.x >= window.max.x-4*w)
            vgascreenputc(scr, "\n", flushr);

        pos = (curpos.x-window.min.x)/w;
        pos = 4-(pos%4);
        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x+pos*w, curpos.y + h);
        memimagedraw(scr->gscreen, r, back, back->r.min, nil, back->r.min, S);
        curpos.x += pos*w;
        break;

    case '\b':
        if(xp <= xbuf)
            break;
        xp--;
        r = Rect(*xp, curpos.y, curpos.x, curpos.y+h);
        memimagedraw(scr->gscreen, r, back, back->r.min, nil, ZP, S);
        combinerect(flushr, r);
        curpos.x = *xp;
        break;

    case '\0':
        break;

    default:
        p = memsubfontwidth(scr->memdefont, buf);
        w = p.x;

        if(curpos.x >= window.max.x-w)
            vgascreenputc(scr, "\n", flushr);

        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x+w, curpos.y+h);
        memimagedraw(scr->gscreen, r, back, back->r.min, nil, back->r.min, S);
        memimagestring(scr->gscreen, curpos, conscol, ZP, scr->memdefont, buf);
        combinerect(flushr, r);
        curpos.x += w;
    }
//  drawdebug = 0;
}
/*e: function vgascreenputc */

/*s: function vgascreenputs */
static void
vgascreenputs(char* s, int n)
{
    int i, gotdraw;
    Rune r;
    char buf[4];
    VGAscr *scr;
    Rectangle flushr;

    scr = &vgascreen[0];

    if(!islo()){
        /*
         * Don't deadlock trying to
         * print in an interrupt.
         */
        if(!canlock(&vgascreenlock))
            return;
    }
    else
        lock(&vgascreenlock);

    /*
     * Be nice to hold this, but not going to deadlock
     * waiting for it.  Just try and see.
     */
    gotdraw = canqlock(&drawlock);

    flushr = Rect(10000, 10000, -10000, -10000);

    while(n > 0){
        i = chartorune(&r, s);
        if(i == 0){
            s++;
            --n;
            continue;
        }
        memmove(buf, s, i);
        buf[i] = 0;
        n -= i;
        s += i;
        vgascreenputc(scr, buf, &flushr);
    }
    flushmemscreen(flushr);

    if(gotdraw)
        qunlock(&drawlock);
    unlock(&vgascreenlock);
}
/*e: function vgascreenputs */

/*s: function vgascreenwin */
void
vgascreenwin(VGAscr* scr)
{
    int h, w;

    h = scr->memdefont->height;
    w = scr->memdefont->info[' '].width;

    window = insetrect(scr->gscreen->r, 48);
    window.max.x = window.min.x+((window.max.x-window.min.x)/w)*w;
    window.max.y = window.min.y+((window.max.y-window.min.y)/h)*h;
    curpos = window.min;

    screenputs = vgascreenputs;
}
/*e: function vgascreenwin */

/*s: function vgablank */
/*
 * Supposedly this is the way to turn DPMS
 * monitors off using just the VGA registers.
 * Unfortunately, it seems to mess up the video mode
 * on the cards I've tried.
 */
void
vgablank(VGAscr*, int blank)
{
    uchar seq1, crtc17;

    if(blank) {
        seq1 = 0x00;
        crtc17 = 0x80;
    } else {
        seq1 = 0x20;
        crtc17 = 0x00;
    }

    outs(Seqx, 0x0100);         /* synchronous reset */
    seq1 |= vgaxi(Seqx, 1) & ~0x20;
    vgaxo(Seqx, 1, seq1);
    crtc17 |= vgaxi(Crtx, 0x17) & ~0x80;
    delay(10);
    vgaxo(Crtx, 0x17, crtc17);
    outs(Crtx, 0x0300);             /* end synchronous reset */
}
/*e: function vgablank */

/*s: function addvgaseg */
void
addvgaseg(char *name, ulong pa, ulong size)
{
    Physseg seg;

    memset(&seg, 0, sizeof seg);
    seg.attr = SG_PHYSICAL;
    seg.name = name;
    seg.pa = pa;
    seg.size = size;
    addphysseg(&seg);
}
/*e: function addvgaseg */


//---------------------------------------------------------------------------
// vgascreen.c
//---------------------------------------------------------------------------


//#define RGB2K(r,g,b)    ((156763*(r)+307758*(g)+59769*(b))>>19)

extern void swcursorhide(void);
extern void swcursoravoid(Rectangle);
extern void vgalinearpci(VGAscr*);


//already in libdraw/arith.c
/*s: global physgscreenr */
Rectangle physgscreenr;
/*e: global physgscreenr */

/*s: global gscreendata */
Memdata gscreendata;
/*e: global gscreendata */
/*s: global gscreen */
Memimage *gscreen;
/*e: global gscreen */

/*s: global vgascreen */
VGAscr vgascreen[1];
/*e: global vgascreen */

/*s: global arrow */
Cursor  arrow = {
    { -1, -1 },
    { 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C, 
      0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04, 
      0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04, 
      0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40, 
    },
    { 0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0, 
      0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8, 
      0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8, 
      0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00, 
    },
};
/*e: global arrow */

/*s: global didswcursorinit */
int didswcursorinit;
/*e: global didswcursorinit */

/*s: global softscreen */
static void *softscreen;
/*e: global softscreen */



bool
ishwimage(Memimage* i)
{
  return 
    (vgascreen[0].gscreendata && 
     i->data->bdata == vgascreen[0].gscreendata->bdata);
}


/*s: function screensize */
int
screensize(int x, int y, int z, ulong chan)
{
    VGAscr *scr;
    void *oldsoft;

    lock(&vgascreenlock);
    if(waserror()){
        unlock(&vgascreenlock);
        nexterror();
    }

    memimageinit();
    scr = &vgascreen[0];
    oldsoft = softscreen;

    if(scr->paddr == 0){
        int width = (x*z)/BI2WD;
        void *p;

        p = xalloc(width*BY2WD*y);
        if(p == nil)
            error("no memory for vga soft screen");
        gscreendata.bdata = softscreen = p;
        if(scr->dev && scr->dev->page){
            scr->vaddr = KADDR(VGAMEM());
            scr->apsize = 1<<16; // >>
        }
        scr->useflush = 1;
    }
    else{
        gscreendata.bdata = scr->vaddr;
        scr->useflush = scr->dev && scr->dev->flush;
    }

    scr->gscreen = nil;
    if(gscreen)
        freememimage(gscreen);
    gscreen = allocmemimaged(Rect(0,0,x,y), chan, &gscreendata);
    if(gscreen == nil)
        error("no memory for vga memimage");
    vgaimageinit(chan);

    scr->palettedepth = 6;  /* default */
    scr->gscreendata = &gscreendata;
    scr->memdefont = getmemdefont();
    scr->gscreen = gscreen;

    physgscreenr = gscreen->r;
    unlock(&vgascreenlock);
    poperror();
    if(oldsoft)
        xfree(oldsoft);

    memimagedraw(gscreen, gscreen->r, memblack, ZP, nil, ZP, S);
    flushmemscreen(gscreen->r);

    if(didswcursorinit)
        swcursorinit();
    drawcmap();
    return 0;
}
/*e: function screensize */

/*s: function screenaperture */
int
screenaperture(int size, int align)
{
    VGAscr *scr;

    scr = &vgascreen[0];

    if(scr->paddr)  /* set up during enable */
        return 0;

    if(size == 0)
        return 0;

    if(scr->dev && scr->dev->linear){
        scr->dev->linear(scr, size, align);
        return 0;
    }

    /*
     * Need to allocate some physical address space.
     * The driver will tell the card to use it.
     */
    size = PGROUND(size);
    scr->paddr = upaalloc(size, align);
    if(scr->paddr == 0)
        return -1;
    scr->vaddr = vmap(scr->paddr, size);
    if(scr->vaddr == nil)
        return -1;
    scr->apsize = size;

    return 0;
}
/*e: function screenaperture */

/*s: function attachscreen */
uchar*
attachscreen(Rectangle* r, ulong* chan, int* d, int* width, int *softscreen)
{
    VGAscr *scr;

    scr = &vgascreen[0];
    if(scr->gscreen == nil || scr->gscreendata == nil)
        return nil;

    *r = scr->gscreen->clipr;
    *chan = scr->gscreen->chan;
    *d = scr->gscreen->depth;
    *width = scr->gscreen->width;
    *softscreen = scr->useflush;

    return scr->gscreendata->bdata;
}
/*e: function attachscreen */

/*s: function flushmemscreen */
/*
 * It would be fair to say that this doesn't work for >8-bit screens.
 */
void
flushmemscreen(Rectangle r)
{
    VGAscr *scr;
    uchar *sp, *disp, *sdisp, *edisp;
    int y, len, incs, off, page;

    scr = &vgascreen[0];
    if(scr->dev && scr->dev->flush){
        scr->dev->flush(scr, r);
        return;
    }
    if(scr->gscreen == nil || scr->useflush == 0)
        return;
    if(scr->dev == nil || scr->dev->page == nil)
        return;

    if(rectclip(&r, scr->gscreen->r) == 0)
        return;

    incs = scr->gscreen->width * BY2WD;

    switch(scr->gscreen->depth){
    default:
        len = 0;
        panic("flushmemscreen: depth\n");
        break;
    case 8:
        len = Dx(r);
        break;
    }
    if(len < 1)
        return;

    off = r.min.y*scr->gscreen->width*BY2WD+(r.min.x*scr->gscreen->depth)/8;
    page = off/scr->apsize;
    off %= scr->apsize;
    disp = scr->vaddr;
    sdisp = disp+off;
    edisp = disp+scr->apsize;

    off = r.min.y*scr->gscreen->width*BY2WD+(r.min.x*scr->gscreen->depth)/8;

    sp = scr->gscreendata->bdata + off;

    scr->dev->page(scr, page);
    for(y = r.min.y; y < r.max.y; y++) {
        if(sdisp + incs < edisp) {
            memmove(sdisp, sp, len);
            sp += incs;
            sdisp += incs;
        }
        else {
            off = edisp - sdisp;
            page++;
            if(off <= len){
                if(off > 0)
                    memmove(sdisp, sp, off);
                scr->dev->page(scr, page);
                if(len - off > 0)
                    memmove(disp, sp+off, len - off);
            }
            else {
                memmove(sdisp, sp, len);
                scr->dev->page(scr, page);
            }
            sp += incs;
            sdisp += incs - scr->apsize;
        }
    }
}
/*e: function flushmemscreen */

/*s: function getcolor */
void
getcolor(ulong p, ulong* pr, ulong* pg, ulong* pb)
{
    VGAscr *scr;
    ulong x;

    scr = &vgascreen[0];
    if(scr->gscreen == nil)
        return;

    switch(scr->gscreen->depth){
    default:
        x = 0x0F;
        break;
    case 8:
        x = 0xFF;
        break;
    }
    p &= x;

    lock(&cursor);
    *pr = scr->colormap[p][0];
    *pg = scr->colormap[p][1];
    *pb = scr->colormap[p][2];
    unlock(&cursor);
}
/*e: function getcolor */

/*s: function setpalette */
int
setpalette(ulong p, ulong r, ulong g, ulong b)
{
    VGAscr *scr;
    int d;

    scr = &vgascreen[0];
    d = scr->palettedepth;

    lock(&cursor);
    scr->colormap[p][0] = r;
    scr->colormap[p][1] = g;
    scr->colormap[p][2] = b;
    vgao(PaddrW, p);
    vgao(Pdata, r>>(32-d));
    vgao(Pdata, g>>(32-d));
    vgao(Pdata, b>>(32-d));
    unlock(&cursor);

    return ~0;
}
/*e: function setpalette */

/*s: function setcolor */
/*
 * On some video cards (e.g. Mach64), the palette is used as the 
 * DAC registers for >8-bit modes.  We don't want to set them when the user
 * is trying to set a colormap and the card is in one of these modes.
 */
int
setcolor(ulong p, ulong r, ulong g, ulong b)
{
    VGAscr *scr;
    int x;

    scr = &vgascreen[0];
    if(scr->gscreen == nil)
        return 0;

    switch(scr->gscreen->depth){
    case 1:
    case 2:
    case 4:
        x = 0x0F;
        break;
    case 8:
        x = 0xFF;
        break;
    default:
        return 0;
    }
    p &= x;

    return setpalette(p, r, g, b);
}
/*e: function setcolor */

/*s: function cursoron */
int
cursoron(bool dolock)
{
    VGAscr *scr;
    int v;

    scr = &vgascreen[0];
    if(scr->cur == nil || scr->cur->move == nil)
        return 0;

    if(dolock)
        lock(&cursor);
    v = scr->cur->move(scr, mousexy());
    if(dolock)
        unlock(&cursor);

    return v;
}
/*e: function cursoron */

/*s: function cursoroff */
void
cursoroff(int)
{
}
/*e: function cursoroff */

/*s: function ksetcursor */
void
ksetcursor(Cursor* curs)
{
    VGAscr *scr;

    scr = &vgascreen[0];
    if(scr->cur == nil || scr->cur->load == nil)
        return;

    scr->cur->load(scr, curs);
}
/*e: function ksetcursor */

/*s: global hwaccel */
int hwaccel = 1;
/*e: global hwaccel */
/*s: global hwblank */
int hwblank = 0;    /* turned on by drivers that are known good */
/*e: global hwblank */
/*s: global panning */
int panning = 0;
/*e: global panning */

/*s: function hwdraw */
//@Scheck: not dead, actually this is overriding some def in libmemdraw!! ugly
int hwdraw(Memdrawparam *par)
{
    VGAscr *scr;
    Memimage *dst, *src, *mask;
    int m;

    if(hwaccel == 0)
        return 0;

    scr = &vgascreen[0];
    if((dst=par->dst) == nil || dst->data == nil)
        return 0;
    if((src=par->src) == nil || src->data == nil)
        return 0;
    if((mask=par->mask) == nil || mask->data == nil)
        return 0;

    if(scr->cur == &swcursor){
        /*
         * always calling swcursorhide here doesn't cure
         * leaving cursor tracks nor failing to refresh menus
         * with the latest libmemdraw/draw.c.
         */
        if(dst->data->bdata == gscreendata.bdata)
            swcursoravoid(par->r);
        if(src->data->bdata == gscreendata.bdata)
            swcursoravoid(par->sr);
        if(mask->data->bdata == gscreendata.bdata)
            swcursoravoid(par->mr);
    }
    
    if(dst->data->bdata != gscreendata.bdata)
        return 0;

    if(scr->fill==nil && scr->scroll==nil)
        return 0;

    /*
     * If we have an opaque mask and source is one opaque
     * pixel we can convert to the destination format and just
     * replicate with memset.
     */
    m = Simplesrc|Simplemask|Fullmask;
    if(scr->fill
    && (par->state&m)==m
    && ((par->srgba&0xFF) == 0xFF)
    && (par->op&S) == S)
        return scr->fill(scr, par->r, par->sdval);

    /*
     * If no source alpha, an opaque mask, we can just copy the
     * source onto the destination.  If the channels are the same and
     * the source is not replicated, memmove suffices.
     */
    m = Simplemask|Fullmask;
    if(scr->scroll
    && src->data->bdata==dst->data->bdata
    && !(src->flags&Falpha)
    && (par->state&m)==m
    && (par->op&S) == S)
        return scr->scroll(scr, par->r, par->sr);

    return 0;   
}
/*e: function hwdraw */

/*s: function blankscreen bis */
void
blankscreen(int blank)
{
    VGAscr *scr;

    scr = &vgascreen[0];
    if(hwblank){
        if(scr->blank)
            scr->blank(scr, blank);
        else
            vgablank(scr, blank);
    }
}
/*e: function blankscreen bis */

/*s: function vgalinearpciid */
void
vgalinearpciid(VGAscr *scr, int vid, int did)
{
    Pcidev *p;

    p = nil;
    while((p = pcimatch(p, vid, 0)) != nil){
        if(p->ccrb != 3)    /* video card */
            continue;
        if(did != 0 && p->did != did)
            continue;
        break;
    }
    if(p == nil)
        error("pci video card not found");

    scr->pci = p;
    vgalinearpci(scr);
}
/*e: function vgalinearpciid */

/*s: function vgalinearpci */
void
vgalinearpci(VGAscr *scr)
{
    ulong paddr;
    int i, size, best;
    Pcidev *p;
    
    p = scr->pci;
    if(p == nil)
        return;

    /*
     * Scan for largest memory region on card.
     * Some S3 cards (e.g. Savage) have enormous
     * mmio regions (but even larger frame buffers).
     * Some 3dfx cards (e.g., Voodoo3) have mmio
     * buffers the same size as the frame buffer,
     * but only the frame buffer is marked as
     * prefetchable (bar&8).  If a card doesn't fit
     * into these heuristics, its driver will have to
     * call vgalinearaddr directly.
     */
    best = -1;
    for(i=0; i<nelem(p->mem); i++){
        if(p->mem[i].bar&1) /* not memory */
            continue;
        if(p->mem[i].size < 640*480)    /* not big enough */
            continue;
        if(best==-1 
        || p->mem[i].size > p->mem[best].size 
        || (p->mem[i].size == p->mem[best].size 
          && (p->mem[i].bar&8)
          && !(p->mem[best].bar&8)))
            best = i;
    }
    if(best >= 0){
        paddr = p->mem[best].bar & ~0x0F;
        size = p->mem[best].size;
        vgalinearaddr(scr, paddr, size);
        return;
    }
    error("no video memory found on pci card");
}
/*e: function vgalinearpci */

/*s: function vgalinearaddr */
void
vgalinearaddr(VGAscr *scr, ulong paddr, int size)
{
    int x, nsize;
    ulong npaddr;

    /*
     * new approach.  instead of trying to resize this
     * later, let's assume that we can just allocate the
     * entire window to start with.
     */

    if(scr->paddr == paddr && size <= scr->apsize)
        return;

    if(scr->paddr){
        /*
         * could call vunmap and vmap,
         * but worried about dangling pointers in devdraw
         */
        error("cannot grow vga frame buffer");
    }
    
    /* round to page boundary, just in case */
    x = paddr&(BY2PG-1);
    npaddr = paddr-x;
    nsize = PGROUND(size+x);

    /*
     * Don't bother trying to map more than 4000x4000x32 = 64MB.
     * We only have a 256MB window.
     */
    if(nsize > 64*MB)
        nsize = 64*MB;
    scr->vaddr = vmap(npaddr, nsize);
    if(scr->vaddr == 0)
        error("cannot allocate vga frame buffer");
    scr->vaddr = (char*)scr->vaddr+x;
    scr->paddr = paddr;
    scr->apsize = nsize;
    /* let mtrr harmlessly fail on old CPUs, e.g., P54C */
    if(!waserror()){
        //mtrr(npaddr, nsize, "wc"); disabled mtrr
                error("mtrr disabled");
        poperror();
    }
}
/*e: function vgalinearaddr */


/*s: global swvisible */
/*
 * Software cursor. 
 */
int swvisible;  /* is the cursor visible? */
/*e: global swvisible */
/*s: global swenabled */
int swenabled;  /* is the cursor supposed to be on the screen? */
/*e: global swenabled */
/*s: global swback */
Memimage*   swback; /* screen under cursor */
/*e: global swback */
/*s: global swimg */
Memimage*   swimg;  /* cursor image */
/*e: global swimg */
/*s: global swmask */
Memimage*   swmask; /* cursor mask */
/*e: global swmask */
/*s: global swimg1 */
Memimage*   swimg1;
/*e: global swimg1 */
/*s: global swmask1 */
Memimage*   swmask1;
/*e: global swmask1 */

/*s: global swoffset */
Point   swoffset;
/*e: global swoffset */
/*s: global swrect */
Rectangle   swrect; /* screen rectangle in swback */
/*e: global swrect */
/*s: global swpt */
Point   swpt;   /* desired cursor location */
/*e: global swpt */
/*s: global swvispt */
Point   swvispt;    /* actual cursor location */
/*e: global swvispt */
/*s: global swvers */
int swvers; /* incremented each time cursor image changes */
/*e: global swvers */
/*s: global swvisvers */
int swvisvers;  /* the version on the screen */
/*e: global swvisvers */

/*s: function swcursorhide */
/*
 * called with drawlock locked for us, most of the time.
 * kernel prints at inopportune times might mean we don't
 * hold the lock, but memimagedraw is now reentrant so
 * that should be okay: worst case we get cursor droppings.
 */
void
swcursorhide(void)
{
    if(swvisible == 0)
        return;
    if(swback == nil)
        return;
    swvisible = 0;
    memimagedraw(gscreen, swrect, swback, ZP, memopaque, ZP, S);
    flushmemscreen(swrect);
}
/*e: function swcursorhide */

/*s: function swcursoravoid */
void
swcursoravoid(Rectangle r)
{
    if(swvisible && rectXrect(r, swrect))
        swcursorhide();
}
/*e: function swcursoravoid */

/*s: function swcursordraw */
void
swcursordraw(void)
{
    if(swvisible)
        return;
    if(swenabled == 0)
        return;
    if(swback == nil || swimg1 == nil || swmask1 == nil)
        return;
    assert(!canqlock(&drawlock));
    swvispt = swpt;
    swvisvers = swvers;
    swrect = rectaddpt(Rect(0,0,16,16), swvispt);
    memimagedraw(swback, swback->r, gscreen, swpt, memopaque, ZP, S);
    memimagedraw(gscreen, swrect, swimg1, ZP, swmask1, ZP, SoverD);
    flushmemscreen(swrect);
    swvisible = 1;
}
/*e: function swcursordraw */

/*s: function swenable */
/*
 * Need to lock drawlock for ourselves.
 */
void
swenable(VGAscr*)
{
    swenabled = 1;
    if(canqlock(&drawlock)){
        swcursordraw();
        qunlock(&drawlock);
    }
}
/*e: function swenable */

/*s: function swdisable */
void
swdisable(VGAscr*)
{
    swenabled = 0;
    if(canqlock(&drawlock)){
        swcursorhide();
        qunlock(&drawlock);
    }
}
/*e: function swdisable */

/*s: function swload */
void
swload(VGAscr*, Cursor *curs)
{
    uchar *ip, *mp;
    int i, j, set, clr;

    if(!swimg || !swmask || !swimg1 || !swmask1)
        return;
    /*
     * Build cursor image and mask.
     * Image is just the usual cursor image
     * but mask is a transparent alpha mask.
     * 
     * The 16x16x8 memimages do not have
     * padding at the end of their scan lines.
     */
    ip = byteaddr(swimg, ZP);
    mp = byteaddr(swmask, ZP);
    for(i=0; i<32; i++){
        set = curs->set[i];
        clr = curs->clr[i];
        for(j=0x80; j; j>>=1){
            *ip++ = set&j ? 0x00 : 0xFF;
            *mp++ = (clr|set)&j ? 0xFF : 0x00;
        }
    }
    swoffset = curs->offset;
    swvers++;
    memimagedraw(swimg1, swimg1->r, swimg, ZP, memopaque, ZP, S);
    memimagedraw(swmask1, swmask1->r, swmask, ZP, memopaque, ZP, S);
}
/*e: function swload */

/*s: function swmove */
int
swmove(VGAscr*, Point p)
{
    swpt = addpt(p, swoffset);
    return 0;
}
/*e: function swmove */

/*s: function swcursorclock */
void
swcursorclock(void)
{
    int x;

    if(!swenabled)
        return;
    if(swvisible && eqpt(swpt, swvispt) && swvers==swvisvers)
        return;

    x = splhi();
    if(swenabled)
    if(!swvisible || !eqpt(swpt, swvispt) || swvers!=swvisvers)
    if(canqlock(&drawlock)){
        swcursorhide();
        swcursordraw();
        qunlock(&drawlock);
    }
    splx(x);
}
/*e: function swcursorclock */

/*s: function swcursorinit */
void
swcursorinit(void)
{
    static int init, warned;
    VGAscr *scr;

    didswcursorinit = 1;
    if(!init){
        init = 1;
        addclock0link(swcursorclock, 10);
    }
    scr = &vgascreen[0];
    if(scr==nil || scr->gscreen==nil)
        return;

    if(scr->dev == nil || scr->dev->linear == nil){
        if(!warned){
            print("cannot use software cursor on non-linear vga screen\n");
            warned = 1;
        }
        return;
    }

    if(swback){
        freememimage(swback);
        freememimage(swmask);
        freememimage(swmask1);
        freememimage(swimg);
        freememimage(swimg1);
    }

    swback = allocmemimage(Rect(0,0,32,32), gscreen->chan);
    swmask = allocmemimage(Rect(0,0,16,16), GREY8);
    swmask1 = allocmemimage(Rect(0,0,16,16), GREY1);
    swimg = allocmemimage(Rect(0,0,16,16), GREY8);
    swimg1 = allocmemimage(Rect(0,0,16,16), GREY1);
    if(swback==nil || swmask==nil || swmask1==nil || swimg==nil || swimg1 == nil){
        print("software cursor: allocmemimage fails");
        return;
    }

    memfillcolor(swmask, DOpaque);
    memfillcolor(swmask1, DOpaque);
    memfillcolor(swimg, DBlack);
    memfillcolor(swimg1, DBlack);
}
/*e: function swcursorinit */

/*s: global swcursor */
VGAcur swcursor =
{
    "soft",
    swenable,
    swdisable,
    swload,
    swmove,
};
/*e: global swcursor */

/*e: kernel/devices/screen/386/vgascreen.c */
