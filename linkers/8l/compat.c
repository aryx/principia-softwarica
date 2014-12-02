/*s: linkers/8l/compat.c */
#include	"l.h"

/*s: function malloc(x86) */
/*
 * fake malloc
 */
void*
malloc(ulong n)
{
    void *p;

    // upper_round(n, 8)
    while(n & 7)
        n++;

    while(nhunk < n)
        gethunk();
    p = hunk;
    nhunk -= n;
    hunk += n;
    return p;
}
/*e: function malloc(x86) */

/*s: function free(x86) */
void
free(void *p)
{
    USED(p);
}
/*e: function free(x86) */

/*s: function setmalloctag(x86) */
//@Scheck: looks dead, but because we redefine malloc/free we must also redefine that
void setmalloctag(void *v, ulong pc)
{
    USED(v, pc);
}
/*e: function setmalloctag(x86) */

/*s: function fileexists(x86) */
int
fileexists(char *s)
{
    byte dirbuf[400];

    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
}
/*e: function fileexists(x86) */
/*e: linkers/8l/compat.c */
