/*s: lib_graphics/libmemdraw/resolution.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function wordaddr */
/*
 * Wordaddr is deprecated.
 */
ulong*
wordaddr(Memimage *i, Point p)
{
    return (ulong*) ((uintptr)byteaddr(i, p) & ~(sizeof(ulong)-1));
}
/*e: function wordaddr */

/*s: function byteaddr */
byte*
byteaddr(Memimage *i, Point p)
{
    byte *a;

    a = (i->data->bdata + i->zero) + (sizeof(ulong) * p.y * i->width);

    /*s: [[byteaddr()]] if depth less than 8 */
    if(i->depth < 8){
        /*
         * We need to always round down,
         * but C rounds toward zero.
         */
        int np;
        np = 8/i->depth;
        if(p.x < 0)
            return a+(p.x-np+1)/np;
        else
            return a+p.x/np;
    }
    /*e: [[byteaddr()]] if depth less than 8 */
    else
        return a + p.x * (i->depth/8);
}
/*e: function byteaddr */

/*e: lib_graphics/libmemdraw/resolution.c */
