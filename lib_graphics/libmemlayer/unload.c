/*s: lib_graphics/libmemlayer/unload.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function memunload */
errorneg1
memunload(Memimage *src, Rectangle r, byte *data, int n)
{
    Memlayer *dl;
    /*s: [[memunload()]] other locals */
    Rectangle lr;
    int dx;
    Memimage *tmp;
    /*e: [[memunload()]] other locals */

    Top:
    dl = src->layer;
    if(dl == nil)
        return unloadmemimage(src, r, data, n);
    // else
    /*s: [[memunload()]] if src has layer */
    /*
     Convert to screen coordinates.
     */
    lr = r;
    // r = rectaddpt(r, dl->delta)
    r.min.x += dl->delta.x;
    r.min.y += dl->delta.y;
    r.max.x += dl->delta.x;
    r.max.y += dl->delta.y;

    dx = dl->delta.x&(7/src->depth);
    if(dl->clear && dx==0){
        src = dl->screen->image;
        goto Top;
    }

    /*
     * src is an obscured layer or data is unaligned
     */
    if(dl->save && dx==0){
        if(dl->refreshfn != nil)
            return ERROR_NEG1;	/* can't unload window if it's not Refbackup */
        if(n > 0)
            memlhide(src, r);
        n = unloadmemimage(dl->save, lr, data, n);
        return n;
    }

    tmp = allocmemimage(lr, src->chan);
    if(tmp == nil)
        return ERROR_NEG1;
    memdraw(tmp, lr, src, lr.min, nil, lr.min, S);
    n = unloadmemimage(tmp, lr, data, n);
    freememimage(tmp);
    return n;
    /*e: [[memunload()]] if src has layer */
}
/*e: function memunload */
/*e: lib_graphics/libmemlayer/unload.c */
