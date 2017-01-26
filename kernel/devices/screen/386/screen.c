/*s: kernel/devices/screen/386/screen.c */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"
#include <ureg.h>

#include <draw.h>
#include <font.h>
#include <memdraw.h>
#include <cursor.h>

#include "../port/portscreen.h"
#include "screen.h"

//TODO: can not reuse swcursor.c, WEIRD, kernel fault when run rio!
static void swcursorhide(void);
static void swcursoravoid(Rectangle);
static void swcursordraw(void);
static void swload(VGAscr*, Cursor *curs);
static int swmove(VGAscr*, Point p);
static void swcursorinit(void);

void swcursorinit_wrapper(void);

//---------------------------------------------------------------------------
// vgax.c
//---------------------------------------------------------------------------

/*s: global vgaxlock(x86) */
static Lock vgaxlock;           /* access to index registers */
/*e: global vgaxlock(x86) */

/*s: function vgaxi(x86) */
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
/*e: function vgaxi(x86) */

/*s: function vgaxo(x86) */
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
         * (e.g., 86C928) have trouble with that for some
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
/*e: function vgaxo(x86) */

//---------------------------------------------------------------------------
// vga.c
//---------------------------------------------------------------------------

/*s: global back2(x86) */
static Memimage* back;
/*e: global back2(x86) */
/*s: global conscol(x86) */
static Memimage *conscol;
/*e: global conscol(x86) */

/*s: global curpos(x86) */
static Point curpos;
/*e: global curpos(x86) */
/*s: global window bis(x86) */
static Rectangle window;
/*e: global window bis(x86) */
/*s: global xp(x86) */
static int *xp;
/*e: global xp(x86) */
/*s: global xbuf(x86) */
static int xbuf[256];
/*e: global xbuf(x86) */
/*s: global vgascreenlock(x86) */
Lock vgascreenlock;
/*e: global vgascreenlock(x86) */

/*s: function vgaimageinit(x86) */
void
vgaimageinit(channels chan)
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
/*e: function vgaimageinit(x86) */

/*s: function vgascroll(x86) */
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
    memimagedraw(gscreen, r, gscreen, p, nil, p, S);
    r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
    memimagedraw(gscreen, r, back, ZP, nil, ZP, S);

    curpos.y -= o;
}
/*e: function vgascroll(x86) */

/*s: function vgascreenputc(x86) */
static void
vgascreenputc(VGAscr* scr, char* buf, Rectangle *flushr)
{
    Point p;
    int h, w, pos;
    Rectangle r;

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
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        curpos.x += pos*w;
        break;

    case '\b':
        if(xp <= xbuf)
            break;
        xp--;
        r = Rect(*xp, curpos.y, curpos.x, curpos.y+h);
        memimagedraw(gscreen, r, back, back->r.min, nil, ZP, S);
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
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        memimagestring(gscreen, curpos, conscol, ZP, scr->memdefont, buf);
        combinerect(flushr, r);
        curpos.x += w;
    }
}
/*e: function vgascreenputc(x86) */

/*s: function vgascreenputs(x86) */
static void
vgascreenputs(char* s, int n)
{
    int i;
    bool gotdraw;
    Rune r;
    char buf[4];
    VGAscr *scr;
    Rectangle flushr;

    scr = &vgascreen;

    if(!arch_islo()){
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
    arch_flushmemscreen(flushr);

    if(gotdraw)
        qunlock(&drawlock);
    unlock(&vgascreenlock);
}
/*e: function vgascreenputs(x86) */

/*s: function vgascreenwin(x86) */
void
vgascreenwin(VGAscr* scr)
{
    int h, w;

    h = scr->memdefont->height;
    w = scr->memdefont->info[' '].width;

    window = insetrect(gscreen->r, 48);
    window.max.x = window.min.x+((window.max.x-window.min.x)/w)*w;
    window.max.y = window.min.y+((window.max.y-window.min.y)/h)*h;
    curpos = window.min;

    screenputs = vgascreenputs;
}
/*e: function vgascreenwin(x86) */

/*s: function vgablank(x86) */
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
/*e: function vgablank(x86) */

/*s: function addvgaseg(x86) */
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
/*e: function addvgaseg(x86) */


//---------------------------------------------------------------------------
// vgascreen.c
//---------------------------------------------------------------------------


//#define RGB2K(r,g,b)    ((156763*(r)+307758*(g)+59769*(b))>>19)

extern void vgalinearpci(VGAscr*);


//already in libdraw/arith.c

/*s: global vgascreen(x86) */
VGAscr vgascreen;
/*e: global vgascreen(x86) */

/*s: global didswcursorinit(x86) */
bool didswcursorinit;
/*e: global didswcursorinit(x86) */

/*s: global softscreen(x86) */
static void *softscreen;
/*e: global softscreen(x86) */

/*s: function ishwimage(x86) */
bool
arch_ishwimage(Memimage* i)
{
  return (i->data->bdata == gscreendata.bdata);
}
/*e: function ishwimage(x86) */


/*s: function screensize(x86) */
int
screensize(int x, int y, int z, ulong chan)
{
    VGAscr *scr = &vgascreen;
    /*s: [[screensize()]] other locals */
    void *oldsoft;
    /*e: [[screensize()]] other locals */

    /*s: [[screensize()]] lock */
    lock(&vgascreenlock);
    if(waserror()){
        unlock(&vgascreenlock);
        nexterror();
    }
    /*e: [[screensize()]] lock */
    /*s: [[screensize()]] initializations part1 */
    oldsoft = softscreen;
    /*x: [[screensize()]] initializations part1 */
    memimageinit();
    /*e: [[screensize()]] initializations part1 */

    /*s: [[screensize()]] set [[gscreendata.bdata]] */
    if(scr->paddr == 0){
        int width = (x*z)/BI2BY; // width in bytes
        void *p;

        // !!the alloc!!
        p = xalloc(width*y);
        /*s: [[screensize()]] sanity check p */
        if(p == nil)
            error("no memory for vga soft screen");
        /*e: [[screensize()]] sanity check p */
        gscreendata.bdata = p;
        scr->useflush = true;
        /*s: [[screensize()]] when use softscreen, other settings */
        softscreen = gscreendata.bdata;
        /*x: [[screensize()]] when use softscreen, other settings */
        if(scr->dev && scr->dev->page){
            scr->vaddr = KADDR(VGAMEM());
            scr->apsize = 1<<16;
        }
        /*e: [[screensize()]] when use softscreen, other settings */
    }
    else{
        // direct frame buffer
        gscreendata.bdata = scr->vaddr;
        scr->useflush = (scr->dev && scr->dev->flush);
    }
    /*e: [[screensize()]] set [[gscreendata.bdata]] */

    /*s: [[screensize()]] free previous gscreen */
    if(gscreen)
        freememimage(gscreen);
    /*e: [[screensize()]] free previous gscreen */
    // Setting gscreen!!
    gscreen = allocmemimaged(Rect(0,0,x,y), chan, &gscreendata);
    /*s: [[screensize()]] sanity check gscreen */
    if(gscreen == nil)
        error("no memory for vga memimage");
    /*e: [[screensize()]] sanity check gscreen */

    /*s: [[screensize()]] vga settings */
    scr->palettedepth = 6;  /* default */
    /*x: [[screensize()]] vga settings */
    scr->memdefont = getmemdefont();
    /*x: [[screensize()]] vga settings */
    vgaimageinit(chan);
    /*e: [[screensize()]] vga settings */
    /*s: [[screensize()]] unlock */
    unlock(&vgascreenlock);
    poperror();
    /*e: [[screensize()]] unlock */

    // initial draw
    memimagedraw(gscreen, gscreen->r, memblack, ZP, nil, ZP, S);
    arch_flushmemscreen(gscreen->r);

    /*s: [[screensize()]] initializations part2 */
    if(oldsoft)
        xfree(oldsoft);
    /*x: [[screensize()]] initializations part2 */
    if(didswcursorinit)
        swcursorinit_wrapper();
    /*x: [[screensize()]] initializations part2 */
    drawcmap();
    /*x: [[screensize()]] initializations part2 */
    physgscreenr = gscreen->r;
    /*e: [[screensize()]] initializations part2 */
    return OK_0;
}
/*e: function screensize(x86) */

/*s: function screenaperture(x86) */
int
screenaperture(int size, int align)
{
    VGAscr *scr;

    scr = &vgascreen;

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
/*e: function screenaperture(x86) */

/*s: function attachscreen(x86) */
byte*
arch_attachscreen(Rectangle* r, ulong* chan, int* d, int* width, bool *softscreen)
{

    /*s: [[attachscreen()]] sanity check gscreen */
    if(gscreen == nil || gscreendata.bdata == nil)
        return nil;
    /*e: [[attachscreen()]] sanity check gscreen */

    *r          = gscreen->clipr;
    *chan       = gscreen->chan;
    *d          = gscreen->depth;
    *width      = gscreen->width;

    *softscreen = vgascreen.useflush;

    return gscreendata.bdata;
}
/*e: function attachscreen(x86) */

/*s: function flushmemscreen(x86) */
/*
 * It would be fair to say that this doesn't work for >8-bit screens.
 */
void
arch_flushmemscreen(Rectangle r)
{
    VGAscr *scr;
    /*s: [[flushmemscreen()]] other locals */
    byte *sp, *disp, *sdisp, *edisp;
    int y, len, incs, off, page;
    /*e: [[flushmemscreen()]] other locals */

    scr = &vgascreen;

    // call the device driver flush hook
    if(scr->dev && scr->dev->flush){
        scr->dev->flush(scr, r);
        return;
    }
    // else
    if(gscreen == nil || !scr->useflush)
        return;
    if(scr->dev == nil || scr->dev->page == nil)
        return;
    if(!rectclip(&r, gscreen->r))
        return;

    /*s: [[flushmemscreen()]] use VGA page */
    incs = gscreen->width * BY2WD;

    switch(gscreen->depth){
    case 8:
        len = Dx(r);
        break;
    default:
        len = 0;
        panic("flushmemscreen: depth\n");
        break;
    }
    if(len < 1)
        return;

    off = r.min.y * gscreen->width * BY2WD 
           + (r.min.x * gscreen->depth)/8;
    page = off/scr->apsize;
    off %= scr->apsize;
    disp = scr->vaddr;
    sdisp = disp+off;
    edisp = disp+scr->apsize;

    off = r.min.y * gscreen->width * BY2WD
           + (r.min.x * gscreen->depth)/8;

    sp = gscreendata.bdata + off;

    // call device driver again, for subpart
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

                // call device driver again, for subpart
                scr->dev->page(scr, page);
                if(len - off > 0)
                    memmove(disp, sp+off, len - off);
            }
            else {
                memmove(sdisp, sp, len);
                // call device driver again, for subpart
                scr->dev->page(scr, page);
            }
            sp += incs;
            sdisp += incs - scr->apsize;
        }
    }
    /*e: [[flushmemscreen()]] use VGA page */
}
/*e: function flushmemscreen(x86) */

/*s: function getcolor(x86) */
void
arch_getcolor(ulong p, ulong* pr, ulong* pg, ulong* pb)
{
    VGAscr *scr;
    ulong x;

    scr = &vgascreen;
    if(gscreen == nil)
        return;

    switch(gscreen->depth){
    default:
        x = 0x0F;
        break;
    /*s: [[getcolor()]] switch depth cases */
    case 8:
        x = 0xFF;
        break;
    /*e: [[getcolor()]] switch depth cases */
    }
    p &= x;

    lock(&cursor);
    *pr = scr->colormap[p][0];
    *pg = scr->colormap[p][1];
    *pb = scr->colormap[p][2];
    unlock(&cursor);
}
/*e: function getcolor(x86) */

/*s: function setpalette(x86) */
int
setpalette(ulong p, ulong r, ulong g, ulong b)
{
    VGAscr *scr;
    int d;

    scr = &vgascreen;
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
/*e: function setpalette(x86) */

/*s: function setcolor(x86) */
/*
 * On some video cards (e.g., Mach64), the palette is used as the 
 * DAC registers for >8-bit modes.  We don't want to set them when the user
 * is trying to set a colormap and the card is in one of these modes.
 */
int
arch_setcolor(ulong p, ulong r, ulong g, ulong b)
{
    int x;

    if(gscreen == nil)
        return 0;

    switch(gscreen->depth){
    /*s: [[setcolor()]] switch depth cases */
    case 1:
    case 2:
    case 4:
        x = 0x0F;
        break;
    /*x: [[setcolor()]] switch depth cases */
    case 8:
        x = 0xFF;
        break;
    /*e: [[setcolor()]] switch depth cases */
    default:
        return 0;
    }
    p &= x;

    return setpalette(p, r, g, b);
}
/*e: function setcolor(x86) */

/*s: function cursoron(x86) */
int
arch_cursoron(bool dolock)
{
    VGAscr *scr;
    int v;

    scr = &vgascreen;
    if(scr->cur == nil || scr->cur->move == nil)
        return 0;

    if(dolock)
        lock(&cursor);

    v = scr->cur->move(scr, mousexy());

    if(dolock)
        unlock(&cursor);

    return v;
}
/*e: function cursoron(x86) */

/*s: function cursoroff(x86) */
void
arch_cursoroff(int)
{
}
/*e: function cursoroff(x86) */

/*s: function ksetcursor(x86) */
void
arch_ksetcursor(Cursor* curs)
{
    VGAscr *scr;

    scr = &vgascreen;
    if(scr->cur == nil || scr->cur->load == nil)
        return;

    scr->cur->load(scr, curs);
}
/*e: function ksetcursor(x86) */

/*s: global hwaccel(x86) */
bool hwaccel = true;
/*e: global hwaccel(x86) */
/*s: global hwblank(x86) */
bool hwblank = false;    /* turned on by drivers that are known good */
/*e: global hwblank(x86) */
/*s: global panning(x86) */
bool panning = false;
/*e: global panning(x86) */

/*s: function hwdraw(x86) */
//@Scheck: not dead, actually this is overriding some def in libmemdraw!! ugly
bool hwdraw(Memdrawparam *par)
{
    VGAscr *scr;
    Memimage *dst, *src, *mask;
    int m;

    if(!hwaccel)
        return false;

    scr = &vgascreen;

    dst=par->dst;
    src=par->src;
    mask=par->mask;
    /*s: [[hwdraw()]] sanity check parameters */
    if(dst == nil || dst->data == nil)
        return false;
    if(src == nil || src->data == nil)
        return false;
    if(mask == nil || mask->data == nil)
        return false;
    /*e: [[hwdraw()]] sanity check parameters */

    /*s: [[hwdraw()]] if software cursor(x86) */
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
    /*e: [[hwdraw()]] if software cursor(x86) */
    
    if(dst->data->bdata != gscreendata.bdata)
        return false;
    // else
    /*s: [[hwdraw()]] when dst is the screen */
    if(scr->fill == nil && scr->scroll == nil)
        return false;
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

    return false;   
    /*e: [[hwdraw()]] when dst is the screen */
}
/*e: function hwdraw(x86) */

/*s: function blankscreen bis(x86) */
void
arch_blankscreen(int blank)
{
    VGAscr *scr;

    scr = &vgascreen;
    if(hwblank){
        if(scr->blank)
            scr->blank(scr, blank);
        else
            vgablank(scr, blank);
    }
}
/*e: function blankscreen bis(x86) */

/*s: function vgalinearpciid(x86) */
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
/*e: function vgalinearpciid(x86) */

/*s: function vgalinearpci(x86) */
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
     * Some S3 cards (e.g., Savage) have enormous
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
/*e: function vgalinearpci(x86) */

/*s: function vgalinearaddr(x86) */
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
/*e: function vgalinearaddr(x86) */


/*
 * Software cursor. 
 */
//TODO: reuse swcursor.c (but get kernel fault when run rio, WEIRD)

Cursor  arch_arrow = {
    .offset = { -1, -1 },
    .clr = { 
      0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C, 
      0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04, 
      0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04, 
      0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40, 
    },
    .set = { 
      0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0, 
      0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8, 
      0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8, 
      0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00, 
    },
};

extern bool swvisible;  /* is the cursor visible? */
extern bool swenabled;  /* is the cursor supposed to be on the screen? */
extern Memimage*   swback; /* screen under cursor */
extern Memimage*   swimg;  /* cursor image */
extern Memimage*   swmask; /* cursor mask */
extern Memimage*   swimg1;
extern Memimage*   swmask1;
extern Point   swoffset;
extern Rectangle   swrect; /* screen rectangle in swback */
extern Point   swpt;   /* desired cursor location */
extern Point   swvispt;    /* actual cursor location */
extern int swvers; /* incremented each time cursor image changes */
extern int swvisvers;  /* the version on the screen */


/*
 * called with drawlock locked for us, most of the time.
 * kernel prints at inopportune times might mean we don't
 * hold the lock, but memimagedraw is now reentrant so
 * that should be okay: worst case we get cursor droppings.
 */
static void
swcursorhide(void)
{
    if(!swvisible)
        return;
    if(swback == nil)
        return;

    swvisible = false;
    // restore what was under the cursor
    memimagedraw(gscreen, swrect, swback, ZP, memopaque, ZP, S);
    arch_flushmemscreen(swrect);
}

static void
swcursoravoid(Rectangle r)
{
    if(swvisible && rectXrect(r, swrect))
        swcursorhide();
}

static void
swcursordraw(void)
{
    if(swvisible)
        return;
    if(!swenabled)
        return;
    if(swback == nil || swimg1 == nil || swmask1 == nil)
        return;
    assert(!canqlock(&drawlock));

    swvispt = swpt;
    swvisvers = swvers;
    // cursor is 16x16 picture
    swrect = rectaddpt(Rect(0,0,16,16), swvispt);
    // save what is under the cursor
    memimagedraw(swback, swback->r, gscreen, swpt, memopaque, ZP, S);
    // draw cursor
    memimagedraw(gscreen, swrect, swimg1, ZP, swmask1, ZP, SoverD);
    arch_flushmemscreen(swrect);
    swvisible = true;
}
/*s: function swenable(x86) */
/*
 * Need to lock drawlock for ourselves.
 */
void
swenable(VGAscr*)
{
    swenabled = true;
    if(canqlock(&drawlock)){
        swcursordraw();
        qunlock(&drawlock);
    }
}
/*e: function swenable(x86) */

/*s: function swdisable(x86) */
void
swdisable(VGAscr*)
{
    swenabled = false;
    if(canqlock(&drawlock)){
        swcursorhide();
        qunlock(&drawlock);
    }
}
/*e: function swdisable(x86) */

static void
swload(VGAscr*, Cursor *curs)
{
    byte *ip, *mp;
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
            *ip++ = set & j ? 0x00 : 0xFF;
            *mp++ = (clr|set) & j ? 0xFF : 0x00;
        }
    }
    swoffset = curs->offset;
    swvers++;

    memimagedraw(swimg1,  swimg1->r,  swimg,  ZP, memopaque, ZP, S);
    memimagedraw(swmask1, swmask1->r, swmask, ZP, memopaque, ZP, S);
}

static int
swmove(VGAscr*, Point p)
{
    swpt = addpt(p, swoffset);
    return 0;
}

static void
swcursorclock(void)
{
    int x;

    if(!swenabled)
        return;
    if(swvisible && eqpt(swpt, swvispt) && swvers==swvisvers)
        return;

    x = arch_splhi();
    // check again, might have changed in between
    if(swenabled)
     if(!swvisible || !eqpt(swpt, swvispt) || swvers!=swvisvers)
      if(canqlock(&drawlock)){

        swcursorhide();
        swcursordraw();

        qunlock(&drawlock);
    }
    arch_splx(x);
}

void
swcursorinit_wrapper(void)
{
    static bool init;
    VGAscr *scr;
    static bool warned;

    didswcursorinit = true;
    if(!init){
        init = true;
        addclock0link(swcursorclock, 10);
    }
    scr = &vgascreen;

    if(scr == nil || gscreen == nil)
        return;
    if(scr->dev == nil || scr->dev->linear == nil){
        if(!warned){
            print("cannot use software cursor on non-linear vga screen\n");
            warned = true;
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

    swback  = allocmemimage(Rect(0,0,32,32), gscreen->chan);

    swmask  = allocmemimage(Rect(0,0,16,16), GREY8);
    swmask1 = allocmemimage(Rect(0,0,16,16), GREY1);
    swimg   = allocmemimage(Rect(0,0,16,16), GREY8);
    swimg1  = allocmemimage(Rect(0,0,16,16), GREY1);

    if(swback == nil || swmask == nil || swmask1 == nil || swimg == nil || swimg1 == nil){
        print("software cursor: allocmemimage fails");
        return;
    }

    memfillcolor(swmask,  DOpaque);
    memfillcolor(swmask1, DOpaque);
    memfillcolor(swimg,   DBlack);
    memfillcolor(swimg1,  DBlack);
}


/*s: global swcursor(x86) */
VGAcur swcursor =
{
    .name = "soft",

    .enable  = swenable,
    .disable = swdisable,

    .load = swload,
    .move = swmove,
};
/*e: global swcursor(x86) */

/*e: kernel/devices/screen/386/screen.c */
