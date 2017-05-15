/*s: lib_graphics/libdraw/debug.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: global _drawdebug */
bool	_drawdebug = false;
/*e: global _drawdebug */

/*s: function drawsetdebug */
void
drawsetdebug(bool v)
{
    byte *a;
    a = bufimage(display, 1+1);
    /*s: [[drawsetdebug()]] sanity check a */
    if(a == nil){
        fprint(2, "drawsetdebug: %r\n");
        return;
    }
    /*e: [[drawsetdebug()]] sanity check a */
    a[0] = 'D';
    a[1] = v;
}
/*e: function drawsetdebug */
/*e: lib_graphics/libdraw/debug.c */
