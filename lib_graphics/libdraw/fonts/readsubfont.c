/*s: lib_graphics/libdraw/readsubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function readsubfont */
Subfont*
readsubfont(Display *d, char *name, fdt fd, bool dolock)
{
    Image *i;
    Fontchar *fc;
    char hdr[3*12 + 4+1];
    int n;
    byte *p;
    Subfont *f;

    // reading the image

    i = readimage(d, fd, dolock); // the call!
    /*s: [[readsubfont()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[readsubfont()]] sanity check i */

    // reading the fontchars

    if(read(fd, hdr, 3*12) != 3*12){
        freeimage(i);
        werrstr("rdsubfonfile: header read error: %r");
        return nil;
    }
    n = atoi(hdr);
    p = malloc(6 * (n+1));
    /*s: [[readsubfont()]] sanity check p */
    if(p == nil)
        goto Err;
    /*e: [[readsubfont()]] sanity check p */
    if(read(fd, p, 6 * (n+1)) != 6 * (n+1)){
        werrstr("rdsubfonfile: fontchar read error: %r");
    Err:
        freeimage(i);
        free(p);
        return nil;
    }
    fc = malloc(sizeof(Fontchar) * (n+1));
    /*s: [[readsubfont()]] sanity check fc */
    if(fc == nil)
        goto Err;
    /*e: [[readsubfont()]] sanity check fc */
    _unpackinfo(fc, p, n);
    /*s: [[readsubfont()]] if lock part1 */
    if(dolock)
        lockdisplay(d);
    /*e: [[readsubfont()]] if lock part1 */
    f = allocsubfont(name, n, atoi(hdr+12), atoi(hdr+24), fc, i);
    /*s: [[readsubfont()]] if lock part2 */
    if(dolock)
        unlockdisplay(d);
    /*e: [[readsubfont()]] if lock part2 */
    /*s: [[readsubfont()]] sanity check f */
    if(f == nil){
        free(fc);
        goto Err;
    }
    /*e: [[readsubfont()]] sanity check f */
    free(p);
    return f;
}
/*e: function readsubfont */
/*e: lib_graphics/libdraw/readsubfont.c */
