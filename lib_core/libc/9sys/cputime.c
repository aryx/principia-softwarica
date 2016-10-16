/*s: 9sys/cputime.c */
#include <u.h>
#include <libc.h>

/*s: constant HZ */
#define HZ  1000
/*e: constant HZ */

/*s: function cputime */
double
cputime(void)
{
    long t[4];
    int i;

    times(t);
    for(i=1; i<4; i++)
        t[0] += t[i];
    return t[0] / (double)HZ;
}
/*e: function cputime */
/*e: 9sys/cputime.c */
