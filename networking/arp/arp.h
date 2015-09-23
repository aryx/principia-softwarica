/*s: networking/ip/arp.h */
/*
 *  this file used by (at least) the kernel, arpd, snoopy, tboot
 */
typedef struct Arppkt	Arppkt;
typedef struct Arpentry	Arpentry;
typedef struct Arpstats	Arpstats;

/*s: struct Arppkt */
/* Format of ethernet arp request */
struct Arppkt {
    uchar	d[6];
    uchar	s[6];
    uchar	type[2];
    uchar	hrd[2];
    uchar	pro[2];
    uchar	hln;
    uchar	pln;
    uchar	op[2];
    uchar	sha[6];
    uchar	spa[4];
    uchar	tha[6];
    uchar	tpa[4];
    };
/*e: struct Arppkt */

/*s: constant ARPSIZE */
#define ARPSIZE		42
/*e: constant ARPSIZE */

/*s: struct Arpentry */
/* Format of request from starp to user level arpd */
struct Arpentry {
    uchar	etaddr[6];
    uchar	ipaddr[4];
    };
/*e: struct Arpentry */

/*s: struct Arpstats */
/* Arp cache statistics */
struct Arpstats {
    int	hit;
    int	miss;
    int	failed;
    };
/*e: struct Arpstats */

/*s: constant ET_ARP */
#define ET_ARP		0x0806
/*e: constant ET_ARP */
/*s: constant ET_RARP */
#define ET_RARP		0x8035
/*e: constant ET_RARP */

/*s: constant ARP_REQUEST */
#define ARP_REQUEST	1
/*e: constant ARP_REQUEST */
/*s: constant ARP_REPLY */
#define ARP_REPLY	2
/*e: constant ARP_REPLY */
/*s: constant RARP_REQUEST */
#define RARP_REQUEST	3
/*e: constant RARP_REQUEST */
/*s: constant RARP_REPLY */
#define RARP_REPLY	4
/*e: constant RARP_REPLY */
/*e: networking/ip/arp.h */
