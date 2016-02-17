/*s: lib_graphics/libdraw/subfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function allocsubfont */
Subfont*
allocsubfont(char *name, int n, int height, int ascent, Fontchar *info, Image *i)
{
    Subfont *f;

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
    /*s: [[allocsubfont()]] lookup subfont if name */
    if(name){
        f->name = strdup(name);
        if(lookupsubfont(i->display, name) == nil)
            installsubfont(name, f);
    }
    /*e: [[allocsubfont()]] lookup subfont if name */
    else
        f->name = nil;
    return f;
}
/*e: function allocsubfont */
/*e: lib_graphics/libdraw/subfont.c */
