/*s: lib_graphics/libdraw/freesubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

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
/*e: lib_graphics/libdraw/freesubfont.c */
