/*s: 9sys/getpid.c */
#include    <u.h>
#include    <libc.h>

/*s: function getpid */
int
getpid(void)
{
    char b[20];
    int f;

    memset(b, 0, sizeof(b));
    f = open("#c/pid", 0);
    if(f >= 0) {
        read(f, b, sizeof(b));
        close(f);
    }
    return atol(b);
}
/*e: function getpid */
/*e: 9sys/getpid.c */
