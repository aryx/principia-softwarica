/*s: networking/ip/httpd/emem.c */
#include <u.h>
#include <libc.h>
#include "httpd.h"

/*s: function [[ezalloc]] */
void*
ezalloc(ulong n)
{
    void *p;

    p = malloc(n);
    if(p == nil)
        sysfatal("out of memory");
    memset(p, 0, n);
    return p;
}
/*e: function [[ezalloc]] */

/*s: function [[estrdup]]([[(networking/ip/httpd/emem.c)]]) */
char*
estrdup(char *s)
{
    s = strdup(s);
    if(s == nil)
        sysfatal("out of memory");
    return s;
}
/*e: function [[estrdup]]([[(networking/ip/httpd/emem.c)]]) */

/*e: networking/ip/httpd/emem.c */
