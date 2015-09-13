/*s: assemblers/aa/utils.c */
#include "aa.h"

/*s: function alloc */
/*
 * real allocs
 */
void*
alloc(long n)
{
    void *p;

    while((uintptr)hunk & MAXALIGN) {
        hunk++;
        nhunk--;
    }

    while(nhunk < n)
        gethunk();
    p = hunk;
    nhunk -= n;
    hunk += n;

    return p;
}
/*e: function alloc */

/*s: function allocn */
void*
allocn(void *p, long on, long n)
{
    void *q;

    q = (uchar*)p + on;
    if(q != hunk || nhunk < n) {

        while(nhunk < on+n)
            gethunk();
        memmove(hunk, p, on);
        p = hunk;
        hunk += on;
        nhunk -= on;

    }
    hunk += n;
    nhunk -= n;
    return p;
}
/*e: function allocn */

/*s: function gethunk */
void
gethunk(void)
{
    char *h;
    long nh;

    nh = NHUNK;
    if(thunk >= 10L*NHUNK)
        nh = 10L*NHUNK;

    h = (char*)sbrk(nh);
    if(h == (char*)-1) {
        yyerror("out of memory");
        errorexit();
    }
    hunk = h;
    nhunk = nh;
    thunk += nh;
}
/*e: function gethunk */

/*s: function errorexit */
void
errorexit(void)
{

    if(outfile)
        remove(outfile);
    exits("error");
}
/*e: function errorexit */

/*e: assemblers/aa/utils.c */
