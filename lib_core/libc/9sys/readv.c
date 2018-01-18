/*s: 9sys/readv.c */
#include <u.h>
#include <libc.h>

/*s: function [[ioreadv]] */
static
long
ioreadv(int fd, IOchunk *io, int nio, vlong offset)
{
    int i;
    long m, n, tot;
    char *buf, *p;

    tot = 0;
    for(i=0; i<nio; i++)
        tot += io[i].len;
    buf = malloc(tot);
    if(buf == nil)
        return -1;

    tot = pread(fd, buf, tot, offset);

    p = buf;
    n = tot;
    for(i=0; i<nio; i++){
        if(n <= 0)
            break;
        m = io->len;
        if(m > n)
            m = n;
        memmove(io->addr, p, m);
        n -= m;
        p += m;
        io++;
    }

    free(buf);
    return tot;
}
/*e: function [[ioreadv]] */

/*s: function [[readv]] */
long
readv(int fd, IOchunk *io, int nio)
{
    return ioreadv(fd, io, nio, -1LL);
}
/*e: function [[readv]] */

/*s: function [[preadv]] */
long
preadv(int fd, IOchunk *io, int nio, vlong off)
{
    return ioreadv(fd, io, nio, off);
}
/*e: function [[preadv]] */
/*e: 9sys/readv.c */
