/*s: lib_graphics/libmemdraw/resolution.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

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

/*s: function pixelbits */
ulong
pixelbits(Memimage *i, Point pt)
{
    byte *p;
    ulong val = 0;
    int bpp = i->depth; // bits per pixel
    /*s: [[pixelbits()]] other locals */
    int off, npack;
    /*e: [[pixelbits()]] other locals */

    p = byteaddr(i, pt);

    switch(bpp){
    /*s: [[pixelbits()]] switch bpp cases */
    case 1:
    case 2:
    case 4:
        npack = 8/bpp;
        off = pt.x % npack;
        val = p[0] >> bpp*(npack-1-off);
        val &= (1<<bpp)-1;
        break;
    /*x: [[pixelbits()]] switch bpp cases */
    case 16:
        val = p[0]|(p[1]<<8);
        break;
    /*x: [[pixelbits()]] switch bpp cases */
    case 24:
        val = p[0]|(p[1]<<8)|(p[2]<<16);
        break;
    /*e: [[pixelbits()]] switch bpp cases */
    case 8:
        val = p[0];
        break;
    case 32:
        val = p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24);
        break;
    }
    /*s: [[pixelbits()]] duplicate byte if depth less than 32 */
    // duplicate byte in the whole word
    while(bpp<32){
        val |= val<<bpp;
        bpp *= 2;
    }
    /*e: [[pixelbits()]] duplicate byte if depth less than 32 */
    return val;
}
/*e: function pixelbits */

/*e: lib_graphics/libmemdraw/resolution.c */
