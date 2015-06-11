/*s: lib_networking/libip/ipaux.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: global IPv4bcast */
/*
 *  well known IP addresses
 */
uchar IPv4bcast[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,

    0xff, 0xff, 0xff, 0xff
};
/*e: global IPv4bcast */
/*s: global IPv4allsys */
uchar IPv4allsys[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,

    0xe0, 0, 0, 0x01
};
/*e: global IPv4allsys */
/*s: global IPv4allrouter */
uchar IPv4allrouter[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,

    0xe0, 0, 0, 0x02
};
/*e: global IPv4allrouter */
/*s: global IPallbits */
uchar IPallbits[IPaddrlen] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff
};
/*e: global IPallbits */
/*s: global IPnoaddr */
ipaddr IPnoaddr;
/*e: global IPnoaddr */

/*s: global v4prefix */
/*
 *  prefix of all v4 addresses
 */
ipaddr v4prefix = {
    // first 12
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,

    // rest are ipv4 numbers
    0, 0, 0, 0
};
/*e: global v4prefix */

/*s: function isv4 */
bool
isv4(ipaddr ip)
{
    return memcmp(ip, v4prefix, IPv4off) == 0;
}
/*e: function isv4 */

/*s: function v4tov6 */
/*
 *  the following routines are unrolled with no memset's to speed
 *  up the usual case
 */
void
v4tov6(ipaddr v6, ipv4 v4)
{
    v6[0] = 0; v6[1] = 0; v6[2] = 0; v6[3] = 0;
    v6[4] = 0; v6[5] = 0; v6[6] = 0; v6[7] = 0;
    v6[8] = 0; v6[9] = 0; v6[10] = 0xff; v6[11] = 0xff;

    v6[12] = v4[0];
    v6[13] = v4[1];
    v6[14] = v4[2];
    v6[15] = v4[3];
}
/*e: function v4tov6 */

/*s: function v6tov4 */
errorneg1
v6tov4(ipv4 v4, ipaddr v6)
{
    if(v6[0] == 0 && v6[1] == 0 && v6[2] == 0 && v6[3] == 0
    && v6[4] == 0 && v6[5] == 0 && v6[6] == 0 && v6[7] == 0
    && v6[8] == 0 && v6[9] == 0 && v6[10] == 0xff && v6[11] == 0xff)
    {
        v4[0] = v6[12];
        v4[1] = v6[13];
        v4[2] = v6[14];
        v4[3] = v6[15];
        return OK_0;
    }
    /*s: [[v6tov4()]] else if ipv6 address */
    else {
           memset(v4, 0, 4);
           if(memcmp(v6, IPnoaddr, IPaddrlen) == 0)
               return OK_0;
           return ERROR_NEG1;
    }
    /*e: [[v6tov4()]] else if ipv6 address */
}
/*e: function v6tov4 */
/*e: lib_networking/libip/ipaux.c */
