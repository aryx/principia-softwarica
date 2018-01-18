/*s: 9sys/dirwstat.c */
#include <u.h>
#include <libc.h>
#include <fcall.h>

/*s: function [[dirwstat]] */
int
dirwstat(char *name, Dir *d)
{
    uchar *buf;
    int r;

    r = sizeD2M(d);
    buf = malloc(r);
    if(buf == nil)
        return -1;
    convD2M(d, buf, r);
    r = wstat(name, buf, r);
    free(buf);
    return r;
}
/*e: function [[dirwstat]] */
/*e: 9sys/dirwstat.c */
