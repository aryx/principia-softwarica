/*s: lib_graphics/libdraw/subfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <font.h>

/*s: function allocsubfont */
Subfont*
allocsubfont(char *name, int n, int height, int ascent, Fontchar *info, Image *i)
{
    Subfont *f; // TODO sf

    /*s: [[allocsubfont()]] sanity check height */
    assert(height != 0 /* allocsubfont */);
    /*e: [[allocsubfont()]] sanity check height */

    f = malloc(sizeof(Subfont));
    /*s: [[allocsubfont()]] sanity check f */
    if(f == nil)
        return nil;
    /*e: [[allocsubfont()]] sanity check f */
    f->n = n;
    f->height = height;
    f->ascent = ascent;
    f->info = info;
    f->bits = i;
    f->ref = 1;
    /*s: [[allocsubfont()]] install subfont if name */
    if(name){
        f->name = strdup(name);
        if(lookupsubfont(i->display, name) == nil)
            installsubfont(name, f);
    }
    /*e: [[allocsubfont()]] install subfont if name */
    else
        f->name = nil;
    return f;
}
/*e: function allocsubfont */

/*s: function freesubfont */
void
freesubfont(Subfont *f)
{
    /*s: [[freesubfont()]] sanity check f */
    if(f == nil)
        return;
    /*e: [[freesubfont()]] sanity check f */
    f->ref--;
    if(f->ref > 0)
        return;

    // else
    uninstallsubfont(f);
    free(f->info);	/* note: f->info must have been malloc'ed! */
    freeimage(f->bits);
    free(f);
}
/*e: function freesubfont */

/*s: function _getsubfont */
/*
 * Default version: treat as file name
 */
Subfont*
_getsubfont(Display *d, char *name)
{
    fdt fd;
    Subfont *f;

    fd = open(name, OREAD);
    /*s: [[_getsubfont()]] sanity check fd */
    if(fd < 0){
        fprint(2, "getsubfont: can't open %s: %r\n", name);
        return nil;
    }
    /*e: [[_getsubfont()]] sanity check fd */

    /*s: [[_getsubfont()]] locking part1 */
    /*
     * unlock display so i/o happens with display released, unless
     * user is doing his own locking, in which case this could break things.
     * _getsubfont is called only from string.c and stringwidth.c,
     * which are known to be safe to have this done.
     */
    if(d && !d->locking)
        unlockdisplay(d);
    /*e: [[_getsubfont()]] locking part1 */
    f = readsubfont(d, name, fd, d && !d->locking);
    /*s: [[_getsubfont()]] locking part2 */
    if(d && !d->locking)
        lockdisplay(d);
    /*e: [[_getsubfont()]] locking part2 */
    /*s: [[_getsubfont()]] sanity check f */
    if(f == nil)
        fprint(2, "getsubfont: can't read %s: %r\n", name);
    /*e: [[_getsubfont()]] sanity check f */
    close(fd);
    /*s: [[_getsubfont()]] set malloc tag for debug */
    setmalloctag(f, getcallerpc(&d));
    /*e: [[_getsubfont()]] set malloc tag for debug */

    return f;
}
/*e: function _getsubfont */

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

/*s: function packinfo */
static
void
packinfo(Fontchar *fc, uchar *p, int n)
{
    int j;

    for(j=0;  j<=n;  j++){
        p[0] = fc->x;
        p[1] = fc->x>>8;
        p[2] = fc->top;
        p[3] = fc->bottom;
        p[4] = fc->left;
        p[5] = fc->width;
        fc++;
        p += 6;
    }
}
/*e: function packinfo */

/*s: function writesubfont */
errorneg1
writesubfont(fdt fd, Subfont *f)
{
    char hdr[3*12+1];
    byte *data;
    int nb;

    sprint(hdr, "%11d %11d %11d ", f->n, f->height, f->ascent);
    if(write(fd, hdr, 3*12) != 3*12){
    Err:
        werrstr("writesubfont: bad write: %r");
        return -1;
    }
    nb = 6*(f->n+1);
    data = malloc(nb);
    /*s: [[writesubfont()]] sanity check data */
    if(data == nil)
        return ERROR_NEG1;
    /*e: [[writesubfont()]] sanity check data */
    packinfo(f->info, data, f->n);
    if(write(fd, data, nb) != nb)
        goto Err;
    free(data);
    return OK_0;
}
/*e: function writesubfont */

/*e: lib_graphics/libdraw/subfont.c */
