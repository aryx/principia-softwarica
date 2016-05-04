/*s: lib_graphics/libdraw/allocimagemix.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function allocimagemix */
Image*
allocimagemix(Display *d, ulong color1, ulong color3)
{
    Image *t, *b;
    static Image *qmask;

    if(qmask == nil)
        qmask = allocimage(d, Rect(0,0,1,1), GREY8, true, 0x3F3F3FFF);

    /*s: [[allocimagemix()]] if depth less than 8 */
    if(d->screenimage->depth <= 8){	/* create a 2x2 texture */
        t = allocimage(d, Rect(0,0,1,1), d->screenimage->chan, false, color1);
        if(t == nil)
            return nil;

        b = allocimage(d, Rect(0,0,2,2), d->screenimage->chan, true, color3);
        if(b == nil){
            freeimage(t);
            return nil;
        }

        draw(b, Rect(0,0,1,1), t, nil, ZP);
        freeimage(t);
        return b;
    }
    /*e: [[allocimagemix()]] if depth less than 8 */
    else{	/* use a solid color, blended using alpha */
        t = allocimage(d, Rect(0,0,1,1), d->screenimage->chan, true, color1);
        if(t == nil)
            return nil;

        b = allocimage(d, Rect(0,0,1,1), d->screenimage->chan, true, color3);
        if(b == nil){
            freeimage(t);
            return nil;
        }

        draw(b, b->r, t, qmask, ZP);
        freeimage(t);
        return b;
    }
}
/*e: function allocimagemix */
/*e: lib_graphics/libdraw/allocimagemix.c */
