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
    f = readsubfont(d, name, fd, d && d->locking==false);
    /*s: [[_getsubfont()]] locking part2 */
    if(d && !d->locking)
        lockdisplay(d);
    /*e: [[_getsubfont()]] locking part2 */
    /*s: [[_getsubfont()]] sanity check f */
    if(f == nil)
        fprint(2, "getsubfont: can't read %s: %r\n", name);
    /*e: [[_getsubfont()]] sanity check f */
    close(fd);
    setmalloctag(f, getcallerpc(&d));

    return f;
}
/*e: function _getsubfont */
/*e: lib_graphics/libdraw/getsubfont.c */
