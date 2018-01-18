/*s: arm/notejmp.c */
#include <u.h>
#include <libc.h>
#include <ureg.h>

/*s: function [[notejmp]](arm) */
void
notejmp(void *vr, jmp_buf j, int ret)
{
    struct Ureg *r = vr;

    r->r0 = ret;
    if(ret == 0)
        r->r0 = 1;
    r->pc = j[JMPBUFPC];
    r->r13 = j[JMPBUFSP];
    noted(NCONT);
}
/*e: function [[notejmp]](arm) */
/*e: arm/notejmp.c */
