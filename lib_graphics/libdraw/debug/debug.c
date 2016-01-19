/*s: lib_graphics/libdraw/debug.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function drawsetdebug */
void
drawsetdebug(bool v)
{
    uchar *a;
    a = bufimage(display, 1+1);
    if(a == 0){
        fprint(2, "drawsetdebug: %r\n");
        return;
    }
    a[0] = 'D';
    a[1] = v;
}
/*e: function drawsetdebug */
/*e: lib_graphics/libdraw/debug.c */
