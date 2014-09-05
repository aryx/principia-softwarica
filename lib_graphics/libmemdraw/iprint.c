/*s: lib_graphics/libmemdraw/iprint.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function memdraw_iprint */
int
memdraw_iprint(char*,...)
{
    return -1;
}
/*e: function memdraw_iprint */

/*s: global iprint */
int		(*iprint)(char*, ...) = &memdraw_iprint;
/*e: global iprint */

/*e: lib_graphics/libmemdraw/iprint.c */
