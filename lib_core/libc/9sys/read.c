/*s: 9sys/read.c */
#include    <u.h>
#include    <libc.h>

/*s: function read */
long
read(int fd, void *buf, long n)
{
    return pread(fd, buf, n, -1LL);
}
/*e: function read */
/*e: 9sys/read.c */
