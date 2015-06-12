/*s: lib_networking/libip/equivip.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: function equivip4 */
bool
equivip4(ipv4 a, ipv4 b)
{
    int i;

    for(i = 0; i < 4; i++)
        if(a[i] != b[i])
            return false;
    return true;
}
/*e: function equivip4 */

/*s: function equivip6 */
bool
equivip6(uchar *a, uchar *b)
{
    int i;

    for(i = 0; i < IPaddrlen; i++)
        if(a[i] != b[i])
            return false;
    return true;
}
/*e: function equivip6 */
/*e: lib_networking/libip/equivip.c */
