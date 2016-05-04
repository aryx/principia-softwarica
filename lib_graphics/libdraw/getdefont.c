/*s: lib_graphics/libdraw/getdefont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function getdefont */
Subfont*
getdefont(Display *d)
{
    Subfont *f; // TODO sf
    Image *i;
    Rectangle r;
    int ld;
    char *p, *hdr;
    int n;
    Fontchar *fc;

    p = (char*)defontdata;
    /*s: [[getdefont()]] adjust p if not word-aligned */
    /*
     * make sure data is word-aligned.  this is true with Plan 9 compilers
     * but not in general.  the byte order is right because the data is
     * declared as char*, not ulong*.
     */
    n = (int)(uvlong)p & 3;				/* stupid ape */
    if(n != 0){
        memmove(p+(4-n), p, sizeofdefont-n);
        p += 4-n;
    }
    /*e: [[getdefont()]] adjust p if not word-aligned */

    ld = atoi(p+0*12);
    r.min.x = atoi(p+1*12);
    r.min.y = atoi(p+2*12);
    r.max.x = atoi(p+3*12);
    r.max.y = atoi(p+4*12);

    i = allocimage(d, r, drawld2chan[ld], false, 0);
    /*s: [[getdefont()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[getdefont()]] sanity check i */

    p += 5*12;
    n = loadimage(i, r, (byte*)p, (defontdata + sizeofdefont)-(uchar*)p);
    /*s: [[getdefont()]] sanity check n */
    if(n < 0){
        freeimage(i);
        return nil;
    }
    /*e: [[getdefont()]] sanity check n */

    hdr = p+n;
    n = atoi(hdr);
    p = hdr+3*12;
    fc = malloc(sizeof(Fontchar)*(n+1));
    /*s: [[getdefont()]] sanity check fc */
    if(fc == nil){
        freeimage(i);
        return nil;
    }
    /*e: [[getdefont()]] sanity check fc */
    _unpackinfo(fc, (byte*)p, n);

    f = allocsubfont("*default*", n, atoi(hdr+1*12), atoi(hdr+2*12), fc, i);
    /*s: [[getdefont()]] sanity check f */
    if(f == nil){
        freeimage(i);
        free(fc);
        return nil;
    }
    /*e: [[getdefont()]] sanity check f */
    return f;
}
/*e: function getdefont */
/*e: lib_graphics/libdraw/getdefont.c */
