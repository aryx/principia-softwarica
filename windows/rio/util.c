/*s: windows/rio/util.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>

#include "dat.h"
#include "fns.h"

/*s: function cvttorunes */
void
cvttorunes(char *p, int n, Rune *r, int *nb, int *nr, int *nulls)
{
    uchar *q;
    Rune *s;
    int j, w;

    /*
     * Always guaranteed that n bytes may be interpreted
     * without worrying about partial runes.  This may mean
     * reading up to UTFmax-1 more bytes than n; the caller
     * knows this.  If n is a firm limit, the caller should
     * set p[n] = 0.
     */
    q = (uchar*)p;
    s = r;
    for(j=0; j<n; j+=w){
        if(*q < Runeself){
            w = 1;
            *s = *q++;
        }else{
            w = chartorune(s, (char*)q);
            q += w;
        }
        if(*s)
            s++;
        else if(nulls)
                *nulls = TRUE;
    }
    *nb = (char*)q-p;
    *nr = s-r;
}
/*e: function cvttorunes */

/*s: function error */
void
error(char *s)
{
    fprint(2, "rio: %s: %r\n", s);
    if(errorshouldabort)
        abort();
    threadexitsall("error");
}
/*e: function error */

/*s: function erealloc */
void*
erealloc(void *p, uint n)
{
    p = realloc(p, n);
    if(p == nil)
        error("realloc failed");
    return p;
}
/*e: function erealloc */

/*s: function emalloc */
void*
emalloc(uint n)
{
    void *p;

    p = malloc(n);
    if(p == nil)
        error("malloc failed");
    memset(p, 0, n);
    return p;
}
/*e: function emalloc */

/*s: function estrdup */
char*
estrdup(char *s)
{
    char *p;

    p = malloc(strlen(s)+1);
    if(p == nil)
        error("strdup failed");
    strcpy(p, s);
    return p;
}
/*e: function estrdup */

/*s: function isalnum */
int
isalnum(Rune c)
{
    /*
     * Hard to get absolutely right.  Use what we know about ASCII
     * and assume anything above the Latin control characters is
     * potentially an alphanumeric.
     */
    if(c <= ' ')
        return FALSE;
    if(0x7F<=c && c<=0xA0)
        return FALSE;
    if(utfrune("!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", c))
        return FALSE;
    return TRUE;
}
/*e: function isalnum */

/*s: function strrune */
Rune*
strrune(Rune *s, Rune c)
{
    Rune c1;

    if(c == 0) {
        while(*s++)
            ;
        return s-1;
    }

    while(c1 = *s++)
        if(c1 == c)
            return s-1;
    return nil;
}
/*e: function strrune */

/*s: function min */
int
min(int a, int b)
{
    if(a < b)
        return a;
    return b;
}
/*e: function min */

/*s: function max (windows/rio/util.c) */
int
max(int a, int b)
{
    if(a > b)
        return a;
    return b;
}
/*e: function max (windows/rio/util.c) */

/*s: function runetobyte */
char*
runetobyte(Rune *r, int n, int *ip)
{
    char *s;
    int m;

    s = emalloc(n*UTFmax+1);
    m = snprint(s, n*UTFmax+1, "%.*S", n, r);
    *ip = m;
    return s;
}
/*e: function runetobyte */

/*e: windows/rio/util.c */
