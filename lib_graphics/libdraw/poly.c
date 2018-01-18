/*s: lib_graphics/libdraw/poly.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function [[addcoord]] */
static
uchar*
addcoord(byte *p, int oldx, int newx)
{
    int dx;

    dx = newx-oldx;
    /* does dx fit in 7 signed bits? */
    if((unsigned)(dx - -0x40) <= 0x7F)
        *p++ = dx&0x7F;
    else{
        *p++ = 0x80 | (newx&0x7F);
        *p++ = newx>>7;
        *p++ = newx>>15;
    }
    return p;
}
/*e: function [[addcoord]] */

/*s: function [[dopoly]] */
static
void
dopoly(int cmd, Image *dst, Point *pp, int np, int end0, int end1, int radius, Image *src, Point *sp, Drawop op)
{
    byte *a, *t, *u;
    int i, ox, oy;

    /*s: [[dopoly()]] sanity check n */
    if(np == 0)
        return;
    /*e: [[dopoly()]] sanity check n */
    t = malloc(np*2*3);
    /*s: [[dopoly()]] sanity check t */
    if(t == nil)
        return;
    /*e: [[dopoly()]] sanity check t */
    u = t;
    ox = oy = 0;
    for(i=0; i<np; i++){
        u = addcoord(u, ox, pp[i].x);
        ox = pp[i].x;
        u = addcoord(u, oy, pp[i].y);
        oy = pp[i].y;
    }

    _setdrawop(dst->display, op);

    /// polygon: 'p' dstid[4] n[2] end0[4] end1[4] radius[4] srcid[4] sp[2*4] p0[2*4] dp[2*2*n]
    a = bufimage(dst->display, 1+4+2+4+4+4+4+2*4+(u-t));
    /*s: [[dopoly()]] sanity check a */
    if(a == nil){
        free(t);
        fprint(2, "image poly: %r\n");
        return;
    }
    /*e: [[dopoly()]] sanity check a */
    a[0] = cmd;
    BPLONG(a+1, dst->id);
    BPSHORT(a+5, np-1);
    BPLONG(a+7, end0);
    BPLONG(a+11, end1);
    BPLONG(a+15, radius);
    BPLONG(a+19, src->id);
    BPLONG(a+23, sp->x);
    BPLONG(a+27, sp->y);
    memmove(a+31, t, u-t);
    free(t);
}
/*e: function [[dopoly]] */

/*s: function [[poly]] */
void
poly(Image *dst, Point *p, int np, int end0, int end1, int radius, Image *src, Point sp)
{
    dopoly('p', dst, p, np, end0, end1, radius, src, &sp, SoverD);
}
/*e: function [[poly]] */

/*s: function [[polyop]] */
void
polyop(Image *dst, Point *p, int np, int end0, int end1, int radius, Image *src, Point sp, Drawop op)
{
    dopoly('p', dst, p, np, end0, end1, radius, src, &sp, op);
}
/*e: function [[polyop]] */

/*s: function [[fillpoly]] */
void
fillpoly(Image *dst, Point *p, int np, int wind, Image *src, Point sp)
{
    dopoly('P', dst, p, np, wind, 0, 0, src, &sp, SoverD);
}
/*e: function [[fillpoly]] */

/*s: function [[fillpolyop]] */
void
fillpolyop(Image *dst, Point *p, int np, int wind, Image *src, Point sp, Drawop op)
{
    dopoly('P', dst, p, np, wind, 0, 0, src, &sp, op);
}
/*e: function [[fillpolyop]] */
/*e: lib_graphics/libdraw/poly.c */
