/*s: port/perror.c */
#include <u.h>
#include <libc.h>

/*s: function perror */
void
perror(char *s)
{
    char buf[ERRMAX];

    buf[0] = '\0';
    errstr(buf, sizeof buf);
    if(s && *s)
        fprint(2, "%s: %s\n", s, buf);
    else
        fprint(2, "%s\n", buf);
}
/*e: function perror */
/*e: port/perror.c */
