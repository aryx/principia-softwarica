/*s: lib_graphics/libdraw/error.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function [[drawerror]] */
void
drawerror(Display *d, char *s)
{
    char err[ERRMAX];

    if(d && d->error)
        d->error(d, s);
    else{
        errstr(err, sizeof err);
        fprint(STDERR, "draw: %s: %s\n", s, err);
        exits(s); // extreme!
    }
}
/*e: function [[drawerror]] */

/*e: lib_graphics/libdraw/error.c */
