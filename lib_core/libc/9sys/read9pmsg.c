/*s: libc/9sys/read9pmsg.c */
#include <u.h>
#include <libc.h>
#include <fcall.h>

/*s: function [[read9pmsg]] */
error0
read9pmsg(fdt fd, void *abuf, uint n)
{
    int m, len;
    uchar *buf;

    buf = abuf;

    /* read count */
    m = readn(fd, buf, BIT32SZ);
    if(m != BIT32SZ){
        if(m < 0)
            return ERROR_NEG1;
        return ERROR_0;
    }

    len = GBIT32(buf);
    if(len <= BIT32SZ || len > n){
        werrstr("bad length in 9P2000 message header");
        return ERROR_NEG1;
    }
    len -= BIT32SZ;
    m = readn(fd, buf+BIT32SZ, len);
    if(m < len)
        return ERROR_0;
    return BIT32SZ+m;
}
/*e: function [[read9pmsg]] */
/*e: libc/9sys/read9pmsg.c */
