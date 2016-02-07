/*s: lib_graphics/libdraw/replclipr.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function replclipr */
void
replclipr(Image *i, bool repl, Rectangle clipr)
{
    /*s: [[replclipr()]] body */
    byte *b;

    // set repl and clip: 'c' dstid[4] repl[1] clipR[4*4] */
    b = bufimage(i->display, 1+4+1+4*4);
    b[0] = 'c';
    BPLONG(b+1, i->id);
    repl = (repl != 0);
    b[5] = repl;
    BPLONG(b+6, clipr.min.x);
    BPLONG(b+10, clipr.min.y);
    BPLONG(b+14, clipr.max.x);
    BPLONG(b+18, clipr.max.y);
    i->repl = repl;
    i->clipr = clipr;
    /*e: [[replclipr()]] body */
}
/*e: function replclipr */
/*e: lib_graphics/libdraw/replclipr.c */
