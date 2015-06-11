/*s: lib_networking/libip/classmask.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: global classmask */
static ipaddr classmask[4] = {
    // class A
    0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  
    0xff,0x00,0x00,0x00, // 255.0.0.0
    // class A
    0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  
    0xff,0x00,0x00,0x00, // 255.0.0.0
    // class B
    0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  
    0xff,0xff,0x00,0x00, // 255.255.0.0
    // class C
    0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  0xff,0xff,0xff,0xff,  
    0xff,0xff,0xff,0x00, // 255.255.255.0
};
/*e: global classmask */

/*s: global v6loopback */
static uchar v6loopback[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0x01
};
/*e: global v6loopback */

/*s: global v6linklocal */
static uchar v6linklocal[IPaddrlen] = {
    0xfe, 0x80, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
/*e: global v6linklocal */
/*s: global v6linklocalmask */
static uchar v6linklocalmask[IPaddrlen] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0, 0, 0, 0,
    0, 0, 0, 0
};
/*e: global v6linklocalmask */
/*s: global v6llpreflen */
static int v6llpreflen = 8;	/* link-local prefix length in bytes */
/*e: global v6llpreflen */

/*s: global v6multicast */
static uchar v6multicast[IPaddrlen] = {
    0xff, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
/*e: global v6multicast */
/*s: global v6multicastmask */
static uchar v6multicastmask[IPaddrlen] = {
    0xff, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
/*e: global v6multicastmask */
/*s: global v6mcpreflen */
static int v6mcpreflen = 1;	/* multicast prefix length */
/*e: global v6mcpreflen */

/*s: global v6solicitednode */
static uchar v6solicitednode[IPaddrlen] = {
    0xff, 0x02, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0x01,
    0xff, 0, 0, 0
};
/*e: global v6solicitednode */
/*s: global v6solicitednodemask */
static uchar v6solicitednodemask[IPaddrlen] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0x0, 0x0, 0x0
};
/*e: global v6solicitednodemask */
/*s: global v6snpreflen */
static int v6snpreflen = 13;
/*e: global v6snpreflen */

/*s: function defmask */
uchar*
defmask(ipaddr ip)
{
    if(isv4(ip))
        return classmask[ip[IPv4off]>>6];
    /*s: [[defmask()]] if ipv6 */
    else {
        if(ipcmp(ip, v6loopback) == 0)
            return IPallbits;
        else if(memcmp(ip, v6linklocal, v6llpreflen) == 0)
            return v6linklocalmask;
        else if(memcmp(ip, v6solicitednode, v6snpreflen) == 0)
            return v6solicitednodemask;
        else if(memcmp(ip, v6multicast, v6mcpreflen) == 0)
            return v6multicastmask;
        return IPallbits;
    }
    /*e: [[defmask()]] if ipv6 */
}
/*e: function defmask */

/*s: function maskip */
void
maskip(ipaddr from, ipaddr mask, ipaddr to)
{
    int i;

    for(i = 0; i < IPaddrlen; i++)
        to[i] = from[i] & mask[i];
}
/*e: function maskip */
/*e: lib_networking/libip/classmask.c */
