/*s: lib_graphics/libdraw/ellipse.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function doellipse */
static
void
doellipse(int cmd, Image *dst, Point *c, int xr, int yr, int thick, Image *src, Point *sp, int alpha, int phi, Drawop op)
{
    byte *a;

    _setdrawop(dst->display, op);

    // ellipse: 'e' dstid[4] srcid[4] center[2*4] a[4] b[4] thick[4] sp[2*4] alpha[4] phi[4]
    a = bufimage(dst->display, 1+4+4+2*4+4+4+4+2*4+2*4);
    if(a == 0){
        fprint(2, "image ellipse: %r\n");
        return;
    }
    a[0] = cmd;
    BPLONG(a+1, dst->id);
    BPLONG(a+5, src->id);
    BPLONG(a+9, c->x);
    BPLONG(a+13, c->y);
    BPLONG(a+17, xr);
    BPLONG(a+21, yr);
    BPLONG(a+25, thick);
    BPLONG(a+29, sp->x);
    BPLONG(a+33, sp->y);
    BPLONG(a+37, alpha);
    BPLONG(a+41, phi);
}
/*e: function doellipse */

/*s: function ellipse */
void
ellipse(Image *dst, Point c, int a, int b, int thick, Image *src, Point sp)
{
    doellipse('e', dst, &c, a, b, thick, src, &sp, 0, 0, SoverD);
}
/*e: function ellipse */

/*s: function ellipseop */
void
ellipseop(Image *dst, Point c, int a, int b, int thick, Image *src, Point sp, Drawop op)
{
    doellipse('e', dst, &c, a, b, thick, src, &sp, 0, 0, op);
}
/*e: function ellipseop */

/*s: function fillellipse */
void
fillellipse(Image *dst, Point c, int a, int b, Image *src, Point sp)
{
    doellipse('E', dst, &c, a, b, 0, src, &sp, 0, 0, SoverD);
}
/*e: function fillellipse */

/*s: function fillellipseop */
void
fillellipseop(Image *dst, Point c, int a, int b, Image *src, Point sp, Drawop op)
{
    doellipse('E', dst, &c, a, b, 0, src, &sp, 0, 0, op);
}
/*e: function fillellipseop */

/*s: function arc */
void
arc(Image *dst, Point c, int a, int b, int thick, Image *src, Point sp, int alpha, int phi)
{
    alpha |= 1<<31;
    doellipse('e', dst, &c, a, b, thick, src, &sp, alpha, phi, SoverD);
}
/*e: function arc */

/*s: function arcop */
void
arcop(Image *dst, Point c, int a, int b, int thick, Image *src, Point sp, int alpha, int phi, Drawop op)
{
    alpha |= 1<<31;
    doellipse('e', dst, &c, a, b, thick, src, &sp, alpha, phi, op);
}
/*e: function arcop */

/*s: function fillarc */
void
fillarc(Image *dst, Point c, int a, int b, Image *src, Point sp, int alpha, int phi)
{
    alpha |= 1<<31;
    doellipse('E', dst, &c, a, b, 0, src, &sp, alpha, phi, SoverD);
}
/*e: function fillarc */

/*s: function fillarcop */
void
fillarcop(Image *dst, Point c, int a, int b, Image *src, Point sp, int alpha, int phi, Drawop op)
{
    alpha |= 1<<31;
    doellipse('E', dst, &c, a, b, 0, src, &sp, alpha, phi, op);
}
/*e: function fillarcop */
/*e: lib_graphics/libdraw/ellipse.c */
