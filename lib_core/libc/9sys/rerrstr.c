/*s: libc/9sys/rerrstr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[rerrstr]] */
void
rerrstr(char *buf, uint nbuf)
{
    char tmp[ERRMAX];

    tmp[0] = '\0';
    errstr(tmp, sizeof tmp);

    utfecpy(buf, buf+nbuf, tmp);
    errstr(tmp, sizeof tmp);
}
/*e: function [[rerrstr]] */
/*e: libc/9sys/rerrstr.c */
