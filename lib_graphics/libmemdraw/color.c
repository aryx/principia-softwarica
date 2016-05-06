/*s: lib_graphics/libmemdraw/color.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define DBG1 if(0) print

/*s: function imgtorgba */
rgba
imgtorgba(Memimage *img, ulong val)
{
    byte r, g, b, a;
    ulong chan;
    int nb, v;
    /*s: [[imgtorgba()]] other locals */
    byte *p;
    int ov;
    /*e: [[imgtorgba()]] other locals */

    a = 0xFF;
    r = g = b = 0xAA;	/* garbage */

    for(chan=img->chan; chan; chan>>=8){
        nb = NBITS(chan);
        v = val&((1<<nb)-1);
        val >>= nb;
        ov = v;
        /*s: [[imgtorgba()]] duplicate bits if nb less than 8 */
        // duplicate bits
        while(nb < 8){
            v |= v<<nb;
            nb *= 2;
        }
        v >>= (nb-8);
        /*e: [[imgtorgba()]] duplicate bits if nb less than 8 */

        switch(TYPE(chan)){
        case CRed:
            r = v;
            break;
        case CGreen:
            g = v;
            break;
        case CBlue:
            b = v;
            break;
        case CAlpha:
            a = v;
            break;
        /*s: [[imgtorgba()]] switch chan type cases */
        case CGrey:
            r = g = b = v;
            break;
        /*x: [[imgtorgba()]] switch chan type cases */
        case CMap:
            p = img->cmap->cmap2rgb + 3*ov;
            r = *p++;
            g = *p++;	
            b = *p;
            break;
        /*e: [[imgtorgba()]] switch chan type cases */
        }
    }
    return (r<<24)|(g<<16)|(b<<8)|a;	
}
/*e: function imgtorgba */

/*s: function rgbatoimg */
ulong
rgbatoimg(Memimage *img, rgba rgba)
{
    byte r, g, b, a;
    ulong chan;
    int d, nb;
    ulong v;
    /*s: [[rgbatoimg()]] other locals */
    byte *p;
    byte m;
    /*e: [[rgbatoimg()]] other locals */

    v = 0;
    r = rgba>>24;
    g = rgba>>16;
    b = rgba>>8;
    a = rgba;
    d = 0;
    for(chan=img->chan; chan; chan>>=8){
        nb = NBITS(chan);
        switch(TYPE(chan)){
        case CRed:
            v |= (r>>(8-nb))<<d;
            break;
        case CGreen:
            v |= (g>>(8-nb))<<d;
            break;
        case CBlue:
            v |= (b>>(8-nb))<<d;
            break;
        case CAlpha:
            v |= (a>>(8-nb))<<d;
            break;
        /*s: [[rgbatoimg()]] switch chan type cases */
        case CGrey:
            m = RGB2K(r,g,b);
            v |= (m>>(8-nb))<<d;
            break;
        /*x: [[rgbatoimg()]] switch chan type cases */
        case CMap:
            p = img->cmap->rgb2cmap;
            m = p[(r>>4)*256+(g>>4)*16+(b>>4)];
            v |= (m>>(8-nb))<<d;
            break;
        /*e: [[rgbatoimg()]] switch chan type cases */
        }
        d += nb;
    }
    DBG1("rgba2img %.8lux = %.*lux\n", rgba, 2*d/8, v);
    return v;
}
/*e: function rgbatoimg */

// could put memfillcolor in draw.c so no need those extern decls.
extern void memsetl(void *vp, ulong val, int n);
extern void memset24(void *vp, ulong val, int n);

/*s: function memfillcolor */
void
memfillcolor(Memimage *i, rgba val)
{
    ulong bits;
    /*s: [[memfillcolor()]] other colors */
    int d;
    /*x: [[memfillcolor()]] other colors */
    int y;
    /*e: [[memfillcolor()]] other colors */

    if(val == DNofill)
        return;

    bits = rgbatoimg(i, val);
 
    switch(i->depth){
    /*s: [[memfillcolor()]] switch depth cases */
    case 24:	/* 24-bit images suck */
        for(y=i->r.min.y; y<i->r.max.y; y++)
            memset24(byteaddr(i, Pt(i->r.min.x, y)), bits, Dx(i->r));
        break;
    /*e: [[memfillcolor()]] switch depth cases */
    default:	/* 1, 2, 4, 8, 16, 32 */
        /*s: [[memfillcolor()]] duplicate bits if depth less than 32 */
        for(d=i->depth; d<32; d*=2)
            // duplicate bits
            bits = (bits << d) | bits;
        /*e: [[memfillcolor()]] duplicate bits if depth less than 32 */
        memsetl(wordaddr(i, i->r.min), bits, i->width * Dy(i->r));
        break;
    }
}
/*e: function memfillcolor */
/*e: lib_graphics/libmemdraw/color.c */
