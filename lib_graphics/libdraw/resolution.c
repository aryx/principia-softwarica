/*s: lib_graphics/libdraw/resolution.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function unitsperline */
static
int
unitsperline(Rectangle r, int d, int bitsperunit)
{
    ulong l, t;

    /*s: [[unitsperline()]] sanity check d */
    if(d <= 0 || d > 32)	/* being called wrong.  d is image depth. */
        abort();
    /*e: [[unitsperline()]] sanity check d */

    if(r.min.x >= 0){
        l = (r.max.x*d+bitsperunit-1)/bitsperunit;
        l -= (r.min.x*d)/bitsperunit;
    }else{			/* make positive before divide */
        t = (-r.min.x*d+bitsperunit-1)/bitsperunit;
        l = t+(r.max.x*d+bitsperunit-1)/bitsperunit;
    }
    return l;
}
/*e: function unitsperline */

/*s: function wordsperline */
int
wordsperline(Rectangle r, int d)
{
    return unitsperline(r, d, 8*sizeof(ulong));
}
/*e: function wordsperline */

/*s: function bytesperline */
int
bytesperline(Rectangle r, int d)
{
    return unitsperline(r, d, 8);
}
/*e: function bytesperline */
/*e: lib_graphics/libdraw/resolution.c */
