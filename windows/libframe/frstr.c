/*s: windows/libframe/frstr.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <frame.h>

/*s: constant CHUNK (windows/libframe/frstr.c) */
#define	CHUNK	16
/*e: constant CHUNK (windows/libframe/frstr.c) */
/*s: function ROUNDUP */
#define	ROUNDUP(n)	((n+CHUNK)&~(CHUNK-1))
/*e: function ROUNDUP */

/*s: function _frallocstr */
uchar *
_frallocstr(Frame *f, unsigned n)
{
    uchar *p;

    p = malloc(ROUNDUP(n));
    if(p == nil)
        drawerror(f->display, "out of memory");
    return p;
}
/*e: function _frallocstr */

/*s: function _frinsure */
void
_frinsure(Frame *f, int bn, unsigned n)
{
    Frbox *b;
    uchar *p;

    b = &f->box[bn];
    if(b->nrune < 0)
        drawerror(f->display, "_frinsure");
    if(ROUNDUP(b->nrune) > n)	/* > guarantees room for terminal NUL */
        return;
    p = _frallocstr(f, n);
    b = &f->box[bn];
    memmove(p, b->ptr, NBYTE(b)+1);
    free(b->ptr);
    b->ptr = p;
}
/*e: function _frinsure */
/*e: windows/libframe/frstr.c */
