/*s: lib_graphics/libmemdraw/hooks.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function memdraw_hwdraw */
int
hwdraw(Memdrawparam*)
{
    return 0;	/* could not satisfy request */
}
/*e: function memdraw_hwdraw */

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

/*e: lib_graphics/libmemdraw/hooks.c */
