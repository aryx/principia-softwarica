/*s: cc/compat.c */
#include	"cc.h"
#include	"compat"

/*s: function malloc */
/*
 * fake mallocs
 */
void*
malloc(ulong n)
{
    return alloc(n);
}
/*e: function malloc */

/*s: function calloc */
void*
calloc(ulong m, ulong n)
{
    return alloc(m*n);
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

/*s: function free */
void
free(void*)
{
}
/*e: function free */

/*s: function mallocz */
/* needed when profiling */
void*
mallocz(ulong size, int clr)
{
    void *v;

    v = alloc(size);
    if(clr && v != nil)
        memset(v, 0, size);
    return v;
}
/*e: function mallocz */

/*s: function setmalloctag */
void
setmalloctag(void*, ulong)
{
}
/*e: function setmalloctag */
/*e: cc/compat.c */
