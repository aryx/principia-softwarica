/*s: libc/9sys/truerand.c */
#include <u.h>
#include <libc.h>

/*s: function [[truerand]] */
ulong
truerand(void)
{
    ulong x;
    static fdt randfd = -1;

    if(randfd < 0)
        randfd = open("/dev/random", OREAD|OCEXEC);
    if(randfd < 0)
        sysfatal("can't open /dev/random");
    if(read(randfd, &x, sizeof(x)) != sizeof(x))
        sysfatal("can't read /dev/random");

    return x;
}
/*e: function [[truerand]] */
/*e: libc/9sys/truerand.c */
