/*s: rio/graphical_window.c */
#include <u.h>
#include <libc.h>
/*s: rio includes */
#include <draw.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"
/*e: rio includes */

/*s: function [[waddraw]] */
void
waddraw(Window *w, Rune *r, int nr)
{
    w->raw = runerealloc(w->raw, w->nraw+nr);
    runemove(w->raw + w->nraw, r, nr);
    w->nraw += nr;
}
/*e: function [[waddraw]] */
/*e: rio/graphical_window.c */
