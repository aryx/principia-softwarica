/*s: lib_graphics/libmemdraw/draw.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define DBG1 if(0) print
#define DBG if(0) print

/*s: global drawdebug */
bool drawdebug;
/*e: global drawdebug */

typedef int Subdraw(Memdrawparam*);
static Subdraw chardraw, memoptdraw;

// in alphadraw.c
Subdraw alphadraw;
// in color.c
ulong imgtorgba(Memimage*, ulong);
ulong rgbatoimg(Memimage*, ulong);
// in resolution.c
ulong pixelbits(Memimage*, Point);

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
    /*s: [[memimagedraw()]] sanity check op */
    if(op < Clear || op > SoverD){
        DBG1("op out of range: %d\n", op);
        return;
    }
    /*e: [[memimagedraw()]] sanity check op */

    // Clipping
    /*s: [[memimagedraw()]] call drawclip, if empty rectangle return */
    if(!drawclip(dst, &r, src, &p0, mask, &p1,   &par.sr, &par.mr)){
        DBG1("empty clipped rectangle\n");
        return;
    }
    /*e: [[memimagedraw()]] call drawclip, if empty rectangle return */

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

    /*s: [[drawclip()]] clipping to destination */
    /* clip to destination */ // can modify r
    if(!rectclip(r, dst->r) || !rectclip(r, dst->clipr))
        return false;
    /*s: [[drawclip()]] adjust p0 and p1 if r changed */
    /* move source point */
    // p0 = addpt(p0, subpb(r->min, rmin))
    p0->x += r->min.x - rmin.x;
    p0->y += r->min.y - rmin.y;
    /* move mask point */
    // p1 = addpt(p1, subpb(r->min, rmin))
    p1->x += r->min.x - rmin.x;
    p1->y += r->min.y - rmin.y;
    /*e: [[drawclip()]] adjust p0 and p1 if r changed */
    /*e: [[drawclip()]] clipping to destination */
    /*s: [[drawclip()]] clipping to source */
    /* map destination rectangle into source */
    sr->min = *p0;
    sr->max.x = p0->x+Dx(*r);
    sr->max.y = p0->y+Dy(*r);

    /* sr is r in source coordinates; clip to source */ // can modify sr
    if(!(src->flags&Frepl) && !rectclip(sr, src->r))
        return false;
    if(!rectclip(sr, src->clipr))
        return false;
    /*e: [[drawclip()]] clipping to source */
    /*s: [[drawclip()]] clipping to mask */
    /* compute and clip rectangle in mask */
    /*s: [[drawclip()]] if splitcoords */
    if(splitcoords){
        /* move mask point with source */
        // p1 = addpt(p1, subpt(sr->min, p0))
        p1->x += sr->min.x - p0->x;
        p1->y += sr->min.y - p0->y;

        mr->min = *p1;
        mr->max.x = p1->x + Dx(*sr);
        mr->max.y = p1->y + Dy(*sr);
        omr = *mr;

        /* mr is now rectangle in mask; clip it */
        if(!(mask->flags&Frepl) && !rectclip(mr, mask->r))
            return false;
        if(!rectclip(mr, mask->clipr))
            return false;

        /* reflect any clips back to source */
        // sr = rectsub(mr, omr)
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
    /*e: [[drawclip()]] clipping to mask */

    /* move source clipping back to destination */
    /*s: [[drawclip()]] adjust r if sr or mr changed */
    delta.x = r->min.x - p0->x;
    delta.y = r->min.y - p0->y;
    // r = rectaddpt(sr, delta)
    r->min.x = sr->min.x + delta.x;
    r->min.y = sr->min.y + delta.y;
    r->max.x = sr->max.x + delta.x;
    r->max.y = sr->max.y + delta.y;
    /*e: [[drawclip()]] adjust r if sr or mr changed */

    /*s: [[drawclip()]] clipping and replication handling */
    /* move source rectangle so sr->min is in src->r */
    /*s: [[drawclip()]] if src is repl */
    if(src->flags&Frepl) {
        delta.x = drawreplxy(src->r.min.x, src->r.max.x, sr->min.x) - sr->min.x;
        delta.y = drawreplxy(src->r.min.y, src->r.max.y, sr->min.y) - sr->min.y;
        // sr = rectaddpt(sr, delta)
        sr->min.x += delta.x;
        sr->min.y += delta.y;
        sr->max.x += delta.x;
        sr->max.y += delta.y;
    }
    *p0 = sr->min;
    /*e: [[drawclip()]] if src is repl */
    /* move mask point so it is in mask->r */
    /*s: [[drawclip()]] mask move */
    *p1 = drawrepl(mask->r, *p1);

    mr->min = *p1;
    mr->max.x = p1->x+Dx(*sr);
    mr->max.y = p1->y+Dy(*sr);
    /*e: [[drawclip()]] mask move */
    /*e: [[drawclip()]] clipping and replication handling */

    assert(Dx(*sr) == Dx(*mr) && Dx(*mr) == Dx(*r));
    assert(Dy(*sr) == Dy(*mr) && Dy(*mr) == Dy(*r));
    assert(ptinrect(*p0, src->r));
    assert(ptinrect(*p1, mask->r));
    assert(ptinrect(r->min, dst->r));

    return true;
}
/*e: function drawclip */



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
void
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
void
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
        byte p[4]; // source
        byte *dp;  // destination
        int dwid;
        /*s: [[memoptdraw()]] locals for memset case */
        int ppb, np, nb;
        uchar lm, rm;
        int d;
        /*e: [[memoptdraw()]] locals for memset case */

        dwid = dst->width * sizeof(ulong);
        dp = byteaddr(dst, par->r.min);
        v = par->sdval;

        switch(dst->depth){
        /*s: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
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
        /*x: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
        default:
            assert(0 /* bad dest depth in memoptdraw */);
        /*x: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
        case 8:
            for(y=0; y < dy; y++, dp += dwid)
                memset(dp, v, dx);
            return true;
        /*x: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
        case 16:
            p[0] = v;		/* make little endian */
            p[1] = v>>8;
            v = *(ushort*)p;
            DBG("dp=%p; dx=%d; for(y=0; y<%d; y++, dp+=%d)\nmemsets(dp, v, dx);\n",         	dp, dx, dy, dwid);
            for(y=0; y<dy; y++, dp+=dwid)
                memsets(dp, v, dx);
            return 1;
        /*x: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
        case 24:
            for(y=0; y<dy; y++, dp+=dwid)
                memset24(dp, v, dx);
            return 1;
        /*e: [[memoptdraw()]] when condition for memset, switch depth of dst cases */
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
        long swid, dwid;
        long nb;
        int dir = 1;

        /*s: [[memoptdraw()]] when condition for memmove, change possibly dir */
        if(src->data == dst->data && 
           byteaddr(dst, par->r.min) > byteaddr(src, par->sr.min))
            dir = -1;
        /*e: [[memoptdraw()]] when condition for memmove, change possibly dir */

        swid = src->width * sizeof(ulong);
        dwid = dst->width * sizeof(ulong);
        sp = byteaddr(src, par->sr.min);
        dp = byteaddr(dst, par->r.min);
        /*s: [[memoptdraw()]] when condition for memmove, if negative dir */
        if(dir == -1){
            sp += (dy-1)*swid;
            dp += (dy-1)*dwid;
            swid = -swid;
            dwid = -dwid;
        }
        /*e: [[memoptdraw()]] when condition for memmove, if negative dir */
        nb = (dx * src->depth)/8;
        for(y=0; y<dy; y++, sp+=swid, dp+=dwid)
            memmove(dp, sp, nb);
        return true;
    }
    /*e: [[memoptdraw()]] if condition for memmove */
    /*s: [[memoptdraw()]] if 1 bit mask, src, and dest */
    /*
     * If we have a 1-bit mask, 1-bit source, and 1-bit destination, and
     * they're all bit aligned, we can just use bit operators.  This happens
     * when we're manipulating boolean masks, e.g., in the arc code.
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

/*e: lib_graphics/libmemdraw/draw.c */
