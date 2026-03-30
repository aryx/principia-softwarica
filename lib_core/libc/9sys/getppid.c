/*s: libc/9sys/getppid.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[getppid]] */
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
/*e: function [[getppid]] */
/*e: libc/9sys/getppid.c */
