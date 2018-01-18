/*s: lib_graphics/libdraw/subfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <font.h>

// Do not put this function in defont.c! Otherwise it will create a chain
// of dependencies that will force the kernel to link in lockdisplay().
// This in turn will generate a signature conflict because of the differences
// between QLock (used in Display) in libc.h and in the kernel (in lib.h).
/*s: function [[getdefont]] */
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

    // header of the image
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

    // header of the Fontchar
    hdr = p+n;
    n = atoi(hdr+0*12);
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
/*e: function [[getdefont]] */


/*s: function [[allocsubfont]] */
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
/*e: function [[allocsubfont]] */

/*s: function [[freesubfont]] */
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
/*e: function [[freesubfont]] */

/*s: function [[_getsubfont]] */
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
/*e: function [[_getsubfont]] */

/*s: function [[readsubfont]] */
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
/*e: function [[readsubfont]] */

/*s: function [[packinfo]] */
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
/*e: function [[packinfo]] */

/*s: function [[writesubfont]] */
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
/*e: function [[writesubfont]] */


/*s: function [[subfontname]] */
/*
 * Default version: convert to file name
 */
char*
subfontname(char *cfname, char *fname, int maxdepth)
{
    char *t, *u, *tmp1, *tmp2;
    int i;

    t = strdup(cfname);  /* t is the return string */
    if(strcmp(cfname, "*default*") == 0)
        return t;
    if(t[0] != '/'){
        tmp2 = strdup(fname);
        u = utfrrune(tmp2, '/');
        if(u)
            u[0] = 0;
        else
            strcpy(tmp2, ".");
        tmp1 = smprint("%s/%s", tmp2, t);
        free(tmp2);
        free(t);
        t = tmp1;
    }

    if(maxdepth > 8)
        maxdepth = 8;

    for(i=3; i>=0; i--){
        if((1<<i) > maxdepth)
            continue;
        /* try i-bit grey */
        tmp2 = smprint("%s.%d", t, i);
        if(access(tmp2, AREAD) == 0) {
            free(t);
            return tmp2;
        }
        free(tmp2);
    }

    /* try default */
    if(access(t, AREAD) == 0)
        return t;

    free(t);
    return nil;
}
/*e: function [[subfontname]] */

/*
 * Easy versions of the cache routines; may be substituted by fancier ones for other purposes
 */

/*s: global [[lastname]] */
static char	*lastname;
/*e: global [[lastname]] */
/*s: global [[lastsubfont]] */
Subfont	*lastsubfont;
/*e: global [[lastsubfont]] */

/*s: function [[lookupsubfont]] */
Subfont*
lookupsubfont(Display *d, char *name)
{
    if(d && strcmp(name, "*default*") == 0)
        return d->defaultsubfont; 
    if(lastname && strcmp(name, lastname)==0)
      if(d == lastsubfont->bits->display){
        lastsubfont->ref++;
        return lastsubfont;
    }
    return nil;
}
/*e: function [[lookupsubfont]] */

/*s: function [[installsubfont]] */
void
installsubfont(char *name, Subfont *subfont)
{
    free(lastname);
    lastname = strdup(name);
    lastsubfont = subfont;	/* notice we don't free the old one; that's your business */
}
/*e: function [[installsubfont]] */

/*s: function [[uninstallsubfont]] */
void
uninstallsubfont(Subfont *subfont)
{
    if(subfont == lastsubfont){
        lastname = nil;
        lastsubfont = nil;
    }
}
/*e: function [[uninstallsubfont]] */

/*e: lib_graphics/libdraw/subfont.c */
