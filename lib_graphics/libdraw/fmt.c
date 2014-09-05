/*s: lib_graphics/libdraw/fmt.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function Rfmt */
int
Rfmt(Fmt *f)
{
    Rectangle r;

    r = va_arg(f->args, Rectangle);
    return fmtprint(f, "%P %P", r.min, r.max);
}
/*e: function Rfmt */

/*s: function Pfmt */
int
Pfmt(Fmt *f)
{
    Point p;

    p = va_arg(f->args, Point);
    return fmtprint(f, "[%d %d]", p.x, p.y);
}
/*e: function Pfmt */

/*e: lib_graphics/libdraw/fmt.c */
