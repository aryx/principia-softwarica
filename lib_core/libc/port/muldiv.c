/*s: libc/port/muldiv.c */
#include <u.h>
#include <libc.h>

/*s: function [[umuldiv]] */
ulong
umuldiv(ulong a, ulong b, ulong c)
{
    double d;

    d = ((double)a * (double)b) / (double)c;
    if(d >= 4294967296.)
        abort();
    return d;
}
/*e: function [[umuldiv]] */

/*s: function [[muldiv]] */
long
muldiv(long a, long b, long c)
{
    int s;
    long v;

    s = 0;
    if(a < 0) {
        s = !s;
        a = -a;
    }
    if(b < 0) {
        s = !s;
        b = -b;
    }
    if(c < 0) {
        s = !s;
        c = -c;
    }
    v = umuldiv(a, b, c);
    if(s)
        v = -v;
    return v;
}
/*e: function [[muldiv]] */
/*e: libc/port/muldiv.c */
