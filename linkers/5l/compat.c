/*s: linkers/5l/compat.c */
#include	"l.h"

/*s: function malloc(arm) */
/*
 * fake malloc
 */
void*
malloc(ulong n)
{
    void *p;

    while(n & 7)
        n++;
    while(nhunk < n)
        gethunk();
    p = hunk;
    nhunk -= n;
    hunk += n;
    return p;
}
/*e: function malloc(arm) */

/*s: function free(arm) */
void
free(void *p)
{
    USED(p);
}
/*e: function free(arm) */

/*s: function calloc(arm) */
void*
calloc(ulong m, ulong n)
{
    void *p;

    n *= m;
    p = malloc(n);
    memset(p, 0, n);
    return p;
}
/*e: function calloc(arm) */

/*s: function realloc(arm) */
void*
realloc(void *p, ulong n)
{
    fprint(2, "realloc(0x%p %ld) called\n", p, n);
    abort();
    return 0;
}
/*e: function realloc(arm) */

/*s: function mysbrk(arm) */
void*
mysbrk(ulong size)
{
    return sbrk(size);
}
/*e: function mysbrk(arm) */

/*s: function setmalloctag(arm) */
void
setmalloctag(void *v, ulong pc)
{
    USED(v, pc);
}
/*e: function setmalloctag(arm) */

/*s: function fileexists(arm) */
int
fileexists(char *s)
{
    uchar dirbuf[400];

    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
}
/*e: function fileexists(arm) */
/*e: linkers/5l/compat.c */
