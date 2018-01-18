/*s: libc/9sys/getwd.c */
#include <u.h>
#include <libc.h>

static char *nsgetwd(char*, int);

/*s: function [[getwd]] */
char*
getwd(char *buf, int nbuf)
{
    int n;
    fdt fd;

    fd = open(".", OREAD);
    if(fd < 0)
        return nil;
    n = fd2path(fd, buf, nbuf);
    close(fd);
    if(n < 0)
        return nil;
    return buf;
}
/*e: function [[getwd]] */
/*e: libc/9sys/getwd.c */
