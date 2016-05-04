/*s: lib_graphics/libdraw/loadimage.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function loadimage */
errorneg1
loadimage(Image *i, Rectangle r, byte *data, int ndata)
{
    int chunk;
    int n, bpl;
    long dy;
    /*s: [[loadimage()]] other locals */
    byte *a;
    /*e: [[loadimage()]] other locals */

    chunk = i->display->bufsize - 64; // a little room for header

    /*s: [[loadimage()]] sanity check r */
    if(!rectinrect(r, i->r)){
        werrstr("loadimage: bad rectangle");
        return ERROR_NEG1;
    }
    /*e: [[loadimage()]] sanity check r */
    bpl = bytesperline(r, i->depth);
    n = bpl * Dy(r);
    /*s: [[loadimage()]] sanity check ndata */
    if(n > ndata){
        werrstr("loadimage: insufficient data");
        return ERROR_NEG1;
    }
    /*e: [[loadimage()]] sanity check ndata */

    ndata = 0;
    while(r.max.y > r.min.y){
        dy = r.max.y - r.min.y;
        if(dy * bpl > chunk)
            dy = chunk / bpl;
        /*s: [[loadimage()]] sanity check dy */
        if(dy <= 0){
            werrstr("loadimage: image too wide for buffer");
            return ERROR_NEG1;
        }
        /*e: [[loadimage()]] sanity check dy */
        n = dy * bpl;

        /*s: [[loadimage()]] marshall one chunk of size n with height dy */
        // write: 'y' id[4] R[4*4] data[x*1]
        a = bufimage(i->display, 21+n);
        /*s: [[loadimage()]] sanity check a */
        if(a == nil){
            werrstr("bufimage failed");
            return ERROR_NEG1;
        }
        /*e: [[loadimage()]] sanity check a */
        a[0] = 'y';
        BPLONG(a+1, i->id);
        BPLONG(a+5, r.min.x);
        BPLONG(a+9, r.min.y);
        BPLONG(a+13, r.max.x);
        BPLONG(a+17, r.min.y + dy);
        memmove(a+21, data, n);
        /*e: [[loadimage()]] marshall one chunk of size n with height dy */

        ndata += n;
        data += n;
        r.min.y += dy; // progress
    }
    /*s: [[loadimage()]] flush and sanity check no error */
    if(flushimage(i->display, false) < 0)
        return ERROR_NEG1;
    /*e: [[loadimage()]] flush and sanity check no error */
    return ndata;
}
/*e: function loadimage */
/*e: lib_graphics/libdraw/loadimage.c */
