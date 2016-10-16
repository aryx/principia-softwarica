/*s: 9sys/writev.c */
#include <u.h>
#include <libc.h>

/*s: function iowritev */
static
long
iowritev(int fd, IOchunk *io, int nio, vlong offset)
{
    int i;
    long tot;
    char *buf, *p;

    tot = 0;
    for(i=0; i<nio; i++)
        tot += io[i].len;
    buf = malloc(tot);
    if(buf == nil)
        return -1;

    p = buf;
    for(i=0; i<nio; i++){
        memmove(p, io->addr, io->len);
        p += io->len;
        io++;
    }

    tot = pwrite(fd, buf, tot, offset);

    free(buf);
    return tot;
}
/*e: function iowritev */

/*s: function writev */
long
writev(int fd, IOchunk *io, int nio)
{
    return iowritev(fd, io, nio, -1LL);
}
/*e: function writev */

/*s: function pwritev */
long
pwritev(int fd, IOchunk *io, int nio, vlong off)
{
    return iowritev(fd, io, nio, off);
}
/*e: function pwritev */
/*e: 9sys/writev.c */
