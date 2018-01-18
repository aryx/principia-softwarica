/*s: libc/9sys/dirfwstat.c */
#include <u.h>
#include <libc.h>
#include <fcall.h>

/*s: function [[dirfwstat]] */
int
dirfwstat(int fd, Dir *d)
{
    uchar *buf;
    int r;

    r = sizeD2M(d);
    buf = malloc(r);
    if(buf == nil)
        return -1;
    convD2M(d, buf, r);
    r = fwstat(fd, buf, r);
    free(buf);
    return r;
}
/*e: function [[dirfwstat]] */
/*e: libc/9sys/dirfwstat.c */
