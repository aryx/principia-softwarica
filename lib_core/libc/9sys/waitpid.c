/*s: 9sys/waitpid.c */
#include <u.h>
#include <libc.h>
#include <fcall.h>

/*s: function [[waitpid]] */
int
waitpid(void)
{
    int n;
    char buf[512];
    char *fld[5];

    n = await(buf, sizeof buf-1);
    if(n <= 0)
        return -1;
    buf[n] = '\0';
    if(tokenize(buf, fld, nelem(fld)) != nelem(fld)){
        werrstr("couldn't parse wait message");
        return -1;
    }
    return atoi(fld[0]);
}
/*e: function [[waitpid]] */

/*e: 9sys/waitpid.c */
