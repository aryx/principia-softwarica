/*s: libc/9sys/write.c */
#include    <u.h>
#include    <libc.h>

/*s: function [[write]] */
long
write(fdt fd, void *buf, long n)
{
    return pwrite(fd, buf, n, -1LL);
}
/*e: function [[write]] */
/*e: libc/9sys/write.c */
