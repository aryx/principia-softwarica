/*s: libc/port/perror.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[perror]] */
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
/*e: function [[perror]] */
/*e: libc/port/perror.c */
