/*s: 9sys/getppid.c */
#include    <u.h>
#include    <libc.h>

/*s: function getppid */
int
getppid(void)
{
    char b[20];
    int f;

    memset(b, 0, sizeof(b));
    f = open("/dev/ppid", OREAD);
    if(f >= 0) {
        read(f, b, sizeof(b));
        close(f);
    }
    return atol(b);
}
/*e: function getppid */
/*e: 9sys/getppid.c */
