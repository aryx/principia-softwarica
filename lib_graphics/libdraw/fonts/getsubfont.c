/*s: lib_graphics/libdraw/getsubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function _getsubfont */
/*
 * Default version: treat as file name
 */

Subfont*
_getsubfont(Display *d, char *name)
{
    int fd;
    Subfont *f;

    fd = open(name, OREAD);
        
    if(fd < 0){
        fprint(2, "getsubfont: can't open %s: %r\n", name);
        return 0;
    }
    /*
     * unlock display so i/o happens with display released, unless
     * user is doing his own locking, in which case this could break things.
     * _getsubfont is called only from string.c and stringwidth.c,
     * which are known to be safe to have this done.
     */
    if(d && d->locking == false)
        unlockdisplay(d);
    f = readsubfont(d, name, fd, d && d->locking==false);
    if(d && d->locking == false)
        lockdisplay(d);
    if(f == 0)
        fprint(2, "getsubfont: can't read %s: %r\n", name);
    close(fd);
    setmalloctag(f, getcallerpc(&d));
    return f;
}
/*e: function _getsubfont */
/*e: lib_graphics/libdraw/getsubfont.c */
