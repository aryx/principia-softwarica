/*s: lib_graphics/libdraw/flush.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function doflush */
static
errorneg1
doflush(Display *d)
{
    int n, nn;

    n = d->bufp - d->buf;
    if(n <= 0)
        return OK_1; // warning?

    nn=write(d->fd, d->buf, n);
    /*s: [[doflush()]] sanity check nn */
    if(nn != n){
        /*s: [[doflush()]] if _drawdebug */
        if(_drawdebug)
            fprint(2, "flushimage fail: d=%p: n=%d nn=%d %r\n", d, n, nn); /**/
        /*e: [[doflush()]] if _drawdebug */
        d->bufp = d->buf;	/* might as well; chance of continuing */
        return ERROR_NEG1;
    }
    /*e: [[doflush()]] sanity check nn */
    d->bufp = d->buf;
    return OK_1;
}
/*e: function doflush */

/*s: function flushimage */
errorneg1
flushimage(Display *d, bool visible)
{
    /*s: [[flushimage()]] sanity check d */
    if(d == nil)
        return OK_0;
    /*e: [[flushimage()]] sanity check d */
    /*s: [[flushimage()]] if visible */
    // visible: 'v'
    if(visible){
        *d->bufp++ = 'v';	/* five bytes always reserved for this */
    }
    /*e: [[flushimage()]] if visible */
    return doflush(d);
}
/*e: function flushimage */

/*s: function bufimage */
byte*
bufimage(Display *d, int n)
{
    byte *p;

    /*s: [[bufimage()]] sanity check n */
    if(n < 0 || n > d->bufsize){
        werrstr("bad count in bufimage");
        return nil;
    }
    /*e: [[bufimage()]] sanity check n */
    if(d->bufp + n  >  d->buf + d->bufsize)
        if(doflush(d) < 0)
            return nil;
    p = d->bufp;
    d->bufp += n;
    return p;
}
/*e: function bufimage */

/*e: lib_graphics/libdraw/flush.c */
