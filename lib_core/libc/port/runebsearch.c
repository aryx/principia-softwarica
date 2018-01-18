/*s: port/runebsearch.c */
#include <u.h>
#include <libc.h>

/*s: function [[_runebsearch]] */
Rune*
_runebsearch(Rune c, Rune *t, int n, int ne)
{
    Rune *p;
    int m;

    while(n > 1) {
        m = n/2;
        p = t + m*ne;
        if(c >= p[0]) {
            t = p;
            n = n-m;
        } else
            n = m;
    }
    if(n && c >= t[0])
        return t;
    return 0;
}
/*e: function [[_runebsearch]] */
/*e: port/runebsearch.c */
