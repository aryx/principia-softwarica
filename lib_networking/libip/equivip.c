/*s: lib_networking/libip/equivip.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: function equivip4 */
int
equivip4(uchar *a, uchar *b)
{
    int i;

    for(i = 0; i < 4; i++)
        if(a[i] != b[i])
            return 0;
    return 1;
}
/*e: function equivip4 */

/*s: function equivip6 */
int
equivip6(uchar *a, uchar *b)
{
    int i;

    for(i = 0; i < IPaddrlen; i++)
        if(a[i] != b[i])
            return 0;
    return 1;
}
/*e: function equivip6 */
/*e: lib_networking/libip/equivip.c */
