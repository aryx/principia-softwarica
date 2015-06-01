/*s: lib_networking/libip/myipaddr.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: global loopbacknet */
static uchar loopbacknet[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,
    127, 0, 0, 0
};
/*e: global loopbacknet */
/*s: global loopbackmask */
static uchar loopbackmask[IPaddrlen] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0, 0, 0
};
/*e: global loopbackmask */

// find first ip addr that isn't the friggin loopback address
/*s: function myipaddr */
// unless there are no others
int
myipaddr(uchar *ip, char *net)
{
    Ipifc *nifc;
    Iplifc *lifc;
    static Ipifc *ifc;
    uchar mynet[IPaddrlen];

    ifc = readipifc(net, ifc, -1);
    for(nifc = ifc; nifc; nifc = nifc->next)
        for(lifc = nifc->lifc; lifc; lifc = lifc->next){
            maskip(lifc->ip, loopbackmask, mynet);
            if(ipcmp(mynet, loopbacknet) == 0){
                continue;
            }
            if(ipcmp(lifc->ip, IPnoaddr) != 0){
                ipmove(ip, lifc->ip);
                return 0;
            }
        }
    ipmove(ip, IPnoaddr);
    return -1;
}
/*e: function myipaddr */
/*e: lib_networking/libip/myipaddr.c */
