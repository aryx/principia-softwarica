/*s: libc/port/memmove.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[memmove]] */
void*
memmove(void *a1, void *a2, ulong n)
{
    char *s1, *s2;

    if((long)n < 0)
        abort();
    s1 = a1;
    s2 = a2;
    if((s2 < s1) && (s2+n > s1))
        goto back;
    while(n > 0) {
        *s1++ = *s2++;
        n--;
    }
    return a1;

back:
    s1 += n;
    s2 += n;
    while(n > 0) {
        *--s1 = *--s2;
        n--;
    }
    return a1;
}
/*e: function [[memmove]] */
/*s: function [[memcpy]] */
void*
memcpy(void *a1, void *a2, ulong n)
{
    return memmove(a1, a2, n);
}
/*e: function [[memcpy]] */
/*e: libc/port/memmove.c */
