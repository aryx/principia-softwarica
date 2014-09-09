/*s: linkers/8l/compat.c */
#include	"l.h"

/*s: function malloc */
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
/*e: function malloc */

/*s: function free */
void
free(void *p)
{
    USED(p);
}
/*e: function free */

/*s: function calloc */
void*
calloc(ulong m, ulong n)
{
    void *p;

    n *= m;
    p = malloc(n);
    memset(p, 0, n);
    return p;
}
/*e: function calloc */

/*s: function realloc */
void*
realloc(void*, ulong)
{
    fprint(2, "realloc called\n");
    abort();
    return 0;
}
/*e: function realloc */

/*s: function mysbrk */
void*
mysbrk(ulong size)
{
    return sbrk(size);
}
/*e: function mysbrk */

/*s: function setmalloctag */
void
setmalloctag(void *v, ulong pc)
{
    USED(v, pc);
}
/*e: function setmalloctag */

/*s: function fileexists */
int
fileexists(char *s)
{
    uchar dirbuf[400];

    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
}
/*e: function fileexists */
/*e: linkers/8l/compat.c */
