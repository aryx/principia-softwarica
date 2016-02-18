/*s: lib_graphics/libdraw/readsubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function readsubfonti */
Subfont*
readsubfonti(Display*d, char *name, fdt fd, Image *ai, bool dolock)
{
    char hdr[3*12+4+1];
    int n;
    uchar *p;
    Fontchar *fc;
    Subfont *f;
    Image *i;

    // image

    i = ai;
    if(i == nil){
        i = readimage(d, fd, dolock); // the call!
        if(i == nil)
            return nil;
    }

    // fontchars

    if(read(fd, hdr, 3*12) != 3*12){
        if(ai == nil)
            freeimage(i);
        werrstr("rdsubfonfile: header read error: %r");
        return nil;
    }
    n = atoi(hdr);
    p = malloc(6*(n+1));
    if(p == nil)
        goto Err;
    if(read(fd, p, 6*(n+1)) != 6*(n+1)){
        werrstr("rdsubfonfile: fontchar read error: %r");
    Err:
        if(ai == nil)
            freeimage(i);
        free(p);
        return nil;
    }
    fc = malloc(sizeof(Fontchar)*(n+1));
    if(fc == nil)
        goto Err;
    _unpackinfo(fc, p, n);
    if(dolock)
        lockdisplay(d);

    f = allocsubfont(name, n, atoi(hdr+12), atoi(hdr+24), fc, i);
    if(dolock)
        unlockdisplay(d);
    if(f == nil){
        free(fc);
        goto Err;
    }
    free(p);
    return f;
}
/*e: function readsubfonti */

/*s: function readsubfont */
Subfont*
readsubfont(Display *d, char *name, fdt fd, bool dolock)
{
    return readsubfonti(d, name, fd, nil, dolock);
}
/*e: function readsubfont */
/*e: lib_graphics/libdraw/readsubfont.c */
