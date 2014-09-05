/*s: lib_graphics/libdraw/freesubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function freesubfont */
void
freesubfont(Subfont *f)
{
    if(f == 0)
        return;
    f->ref--;
    if(f->ref > 0)
        return;
    uninstallsubfont(f);
    free(f->info);	/* note: f->info must have been malloc'ed! */
    freeimage(f->bits);
    free(f);
}
/*e: function freesubfont */
/*e: lib_graphics/libdraw/freesubfont.c */
