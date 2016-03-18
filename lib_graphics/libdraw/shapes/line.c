/*s: lib_graphics/libdraw/line.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function line */
void
line(Image *dst, Point p0, Point p1, int end0, int end1, int radius, Image *src, Point sp)
{
    lineop(dst, p0, p1, end0, end1, radius, src, sp, SoverD);
}
/*e: function line */

/*s: function lineop */
void
lineop(Image *dst, Point p0, Point p1, int end0, int end1, int radius, Image *src, Point sp, Drawop op)
{
    byte *a;

    _setdrawop(dst->display, op);

    // draw line: 'L' dstid[4] p0[2*4] p1[2*4] end0[4] end1[4] radius[4] srcid[4] sp[2*4]
    a = bufimage(dst->display, 1+4+2*4+2*4+4+4+4+4+2*4);
    /*s: [[lineop()]] sanity check a */
    if(a == nil){
        fprint(2, "image line: %r\n");
        return;
    }
    /*e: [[lineop()]] sanity check a */
    a[0] = 'L';
    BPLONG(a+1, dst->id);
    BPLONG(a+5, p0.x);
    BPLONG(a+9, p0.y);
    BPLONG(a+13, p1.x);
    BPLONG(a+17, p1.y);
    BPLONG(a+21, end0);
    BPLONG(a+25, end1);
    BPLONG(a+29, radius);
    BPLONG(a+33, src->id);
    BPLONG(a+37, sp.x);
    BPLONG(a+41, sp.y);
}
/*e: function lineop */
/*e: lib_graphics/libdraw/line.c */
