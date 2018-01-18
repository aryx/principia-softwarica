/*s: libc/9sys/rerrstr.c */
#include <u.h>
#include <libc.h>

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
