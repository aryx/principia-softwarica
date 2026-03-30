/*s: libc/9sys/write.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[write]] */
long
write(fdt fd, void *buf, long n)
{
    return pwrite(fd, buf, n, -1LL);
}
/*e: function [[write]] */
/*e: libc/9sys/write.c */
