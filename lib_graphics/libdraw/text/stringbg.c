/*s: lib_graphics/libdraw/stringbg.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function stringbg */
Point
stringbg(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, Image *bg, Point bgp)
{
    return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, bg, bgp, SoverD);
}
/*e: function stringbg */

/*s: function stringbgop */
Point
stringbgop(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, Image *bg, Point bgp, int op)
{
    return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, bg, bgp, op);
}
/*e: function stringbgop */

/*s: function stringnbg */
Point
stringnbg(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, int len, Image *bg, Point bgp)
{
    return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, bg, bgp, SoverD);
}
/*e: function stringnbg */

/*s: function stringnbgop */
Point
stringnbgop(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, int len, Image *bg, Point bgp, int op)
{
    return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, bg, bgp, op);
}
/*e: function stringnbgop */

/*s: function runestringbg */
Point
runestringbg(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, Image *bg, Point bgp)
{
    return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, bg, bgp, SoverD);
}
/*e: function runestringbg */

/*s: function runestringbgop */
Point
runestringbgop(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, Image *bg, Point bgp, int op)
{
    return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, bg, bgp, op);
}
/*e: function runestringbgop */

/*s: function runestringnbg */
Point
runestringnbg(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, int len, Image *bg, Point bgp)
{
    return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, bg, bgp, SoverD);
}
/*e: function runestringnbg */

/*s: function runestringnbgop */
Point
runestringnbgop(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, int len, Image *bg, Point bgp, int op)
{
    return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, bg, bgp, op);
}
/*e: function runestringnbgop */
/*e: lib_graphics/libdraw/stringbg.c */
