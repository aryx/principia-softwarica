/*s: lib_graphics/libmemlayer/load.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function memload */
errorneg1
memload(Memimage *dst, Rectangle r, byte *data, int n, bool iscompressed)
{
    errorneg1 (*loadfn)(Memimage*, Rectangle, byte*, int);
    Memlayer *dl;
    /*s: [[memload()]] other locals */
    Rectangle lr; // saved logical r
    int dx;
    Memimage *tmp;
    /*e: [[memload()]] other locals */

    loadfn = loadmemimage;
    /*s: [[memload()]] if iscompressed */
    if(iscompressed)
        loadfn = cloadmemimage;
    /*e: [[memload()]] if iscompressed */

    Top:
    dl = dst->layer;
    if(dl == nil)
        return (*loadfn)(dst, r, data, n);
    // else
    /*s: [[memload()]] if dst has a layer */
    /*
     * Convert to screen coordinates.
     */
    lr = r;
    // r = rectaddpt(r, dl->delta)
    r.min.x += dl->delta.x;
    r.min.y += dl->delta.y;
    r.max.x += dl->delta.x;
    r.max.y += dl->delta.y;

    dx = dl->delta.x & (7 / dst->depth);

    /*s: [[memload()]] if dst is fully visible can optimize */
    if(dl->clear && dx==0){
        dst = dl->screen->image;
        goto Top;
    }
    /*e: [[memload()]] if dst is fully visible can optimize */

    /*
     * dst is an obscured layer or data is unaligned
     */
    /*s: [[memload()]] if data is aligned, load on save and expose */
    if(dl->save && dx==0){
        n = loadfn(dl->save, lr, data, n);
        if(n > 0)
            memlexpose(dst, r);
        return n;
    }
    /*e: [[memload()]] if data is aligned, load on save and expose */
    // else
    tmp = allocmemimage(lr, dst->chan);
    /*s: [[memload()]] sanity check tmp */
    if(tmp == nil)
        return ERROR_NEG1;
    /*e: [[memload()]] sanity check tmp */
    n = loadfn(tmp, lr, data, n);
    memdraw(dst, lr, tmp, lr.min, nil, lr.min, S);
    freememimage(tmp);
    return n;
    /*e: [[memload()]] if dst has a layer */
}
/*e: function memload */
/*e: lib_graphics/libmemlayer/load.c */
