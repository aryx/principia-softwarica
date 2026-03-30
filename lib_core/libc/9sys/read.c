/*s: libc/9sys/read.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[read]] */
long
read(fdt fd, void *buf, long n)
{
    return pread(fd, buf, n, -1LL);
}
/*e: function [[read]] */
/*e: libc/9sys/read.c */
