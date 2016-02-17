/*s: lib_graphics/libmemdraw/subfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function allocmemsubfont */
Memsubfont*
allocmemsubfont(char *name, int n, int height, int ascent, Fontchar *info, Memimage *i)
{
    Memsubfont *f;
 
    f = malloc(sizeof(Memsubfont));
    if(f == 0)
        return 0;
    f->n = n;
    f->height = height;
    f->ascent = ascent;
    f->info = info;
    f->bits = i;
    if(name)
        f->name = strdup(name);
    else
        f->name = 0;
    return f;
}
/*e: function allocmemsubfont */

/*e: lib_graphics/libmemdraw/subfont.c */
