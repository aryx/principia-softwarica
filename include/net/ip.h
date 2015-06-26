/*s: include/net/ip.h */
#pragma	src	"/sys/src/libip"
#pragma	lib	"libip.a"

/*s: enum _anon_ */
enum 
{
    /*s: constant IPaddrlen */
    IPaddrlen=	16,
    /*e: constant IPaddrlen */
    /*s: constant IPv4addrlen */
    IPv4addrlen=	4,
    /*e: constant IPv4addrlen */
    /*s: constant IPv4off */
    IPv4off=	12,
    /*e: constant IPv4off */

    IPllen=		4,
    IPV4HDR_LEN=	20,

    /* vihl & vcf[0] values */
    IP_VER4= 	0x40,
    IP_VER6=	0x60,
};
/*e: enum _anon_ */

/*s: typedef ipv4 */
typedef uchar ipv4[IPv4addrlen];
/*e: typedef ipv4 */
/*s: typedef ipaddr */
typedef uchar ipaddr[IPaddrlen];
/*e: typedef ipaddr */
/*s: typedef iplong */
typedef ulong iplong;
/*e: typedef iplong */


// forward decl
typedef struct Ipifc Ipifc;
typedef struct Iplifc Iplifc;
typedef struct Ipv6rp Ipv6rp;
typedef struct Ip6hdr Ip6hdr;
typedef struct Icmp6hdr Icmp6hdr;
typedef struct Udphdr Udphdr;

/*
 *  for reading /net/ipifc
 */

/*s: struct Iplifc (user) */
/* local address */
struct Iplifc
{
    /* per address on the ip interface */
    ipaddr	ip;
    ipaddr	mask;
    ipaddr	net;		/* ip & mask */

    /*s: [[Iplifc(user)]] ipv6 fields */
    ulong	preflt;			/* preferred lifetime */
    ulong	validlt;		/* valid lifetime */
    /*e: [[Iplifc(user)]] ipv6 fields */

    // Extra
    /*s: [[Iplifc(user)]] extra fields */
    // list<ref_own<Iplifc>>, head = Ipifc.lifc
    Iplifc	*next;
    /*e: [[Iplifc(user)]] extra fields */
};
/*e: struct Iplifc (user) */

/*s: struct Ipv6rp */
/* default values, one per stack */
struct Ipv6rp
{
    int	mflag;
    int	oflag;
    int maxraint;
    int	minraint;
    int	linkmtu;
    int	reachtime;
    int	rxmitra;
    int	ttl;
    int	routerlt;	
};
/*e: struct Ipv6rp */

/*s: struct Ipifc (user) */
/* actual interface */
struct Ipifc
{
    /* per ip interface */

    char 	dev[64]; // e.g. "/net/ether0"
    int		mtu;
    // list<ref_own<Iplifc> (next = Iplifc.next)
    Iplifc	*lifc;

    /*s: [[Ipifc(user)]] stat fields */
    ulong	pktin;
    ulong	pktout;
    ulong	errin;
    ulong	errout;
    /*e: [[Ipifc(user)]] stat fields */
    /*s: [[Ipifc(user)]] ipv6 fields */
    Ipv6rp	rp;
    /*x: [[Ipifc(user)]] ipv6 fields */
    uchar	sendra6;		/* on == send router adv */
    uchar	recvra6;		/* on == rcv router adv */
    /*e: [[Ipifc(user)]] ipv6 fields */

    //Extra
    /*s: [[Ipifc(user)]] extra fields */
    Ipifc	*next;
    /*x: [[Ipifc(user)]] extra fields */
    int	index;			/* number of interface in ipifc dir */
    /*e: [[Ipifc(user)]] extra fields */
};
/*e: struct Ipifc (user) */

/*s: macro ISIPV6MCAST */
#define ISIPV6MCAST(addr)	((addr)[0] == 0xff)
/*e: macro ISIPV6MCAST */
/*s: macro ISIPV6LINKLOCAL */
#define ISIPV6LINKLOCAL(addr) ((addr)[0] == 0xfe && ((addr)[1] & 0xc0) == 0x80)
/*e: macro ISIPV6LINKLOCAL */

/*s: enum _anon_ (include/net/ip.h) */
/*
 * ipv6 constants
 * `ra' is `router advertisement', `rs' is `router solicitation'.
 * `na' is `neighbour advertisement'.
 */
enum {
    IPV6HDR_LEN	= 40,

    /* neighbour discovery option types */
    V6nd_srclladdr	= 1,
    V6nd_targlladdr	= 2,
    V6nd_pfxinfo	= 3,
    V6nd_redirhdr	= 4,
    V6nd_mtu	= 5,
    /* new since rfc2461; see iana.org/assignments/icmpv6-parameters */
    V6nd_home	= 8,
    V6nd_srcaddrs	= 9,		/* rfc3122 */
    V6nd_ip		= 17,
    /* /lib/rfc/drafts/draft-jeong-dnsop-ipv6-dns-discovery-12.txt */
    V6nd_rdns	= 25,
    /* plan 9 extensions */
    V6nd_9fs	= 250,
    V6nd_9auth	= 251,

    /* Router constants (all times in ms.) */
    Maxv6initraintvl= 16000,
    Maxv6initras	= 3,
    Maxv6finalras	= 3,
    Minv6interradelay= 3000,
    Maxv6radelay	= 500,

    /* Host constants */
    Maxv6rsdelay	= 1000,
    V6rsintvl	= 4000,
    Maxv6rss	= 3,

    /* Node constants */
    Maxv6mcastrss	= 3,
    Maxv6unicastrss	= 3,
    Maxv6anycastdelay= 1000,
    Maxv6na		= 3,
    V6reachabletime	= 30000,
    V6retranstimer	= 1000,
    V6initprobedelay= 5000,
};
/*e: enum _anon_ (include/net/ip.h) */


/*s: struct Ip6hdr */
/* V6 header on the wire */
struct Ip6hdr {
    uchar	vcf[4];		/* version:4, traffic class:8, flow label:20 */
    uchar	ploadlen[2];	/* payload length: packet length - 40 */
    uchar	proto;		/* next header type */
    uchar	ttl;		/* hop limit */
    ipaddr	src;	/* source address */
    ipaddr	dst;	/* destination address */
    uchar	payload[];
};
/*e: struct Ip6hdr */

/*s: struct Icmp6hdr */
/*
 *  user-level icmpv6 with control message "headers"
 */
struct Icmp6hdr {
    uchar	_0_[8];
    ipaddr	laddr;	/* local address */
    ipaddr	raddr;	/* remote address */
};
/*e: struct Icmp6hdr */

/*s: constant Udphdrsize */
/*
 *  user level udp headers with control message "headers"
 */
#define Udphdrsize 52 /* size of a Udphdr */
/*e: constant Udphdrsize */

/*s: struct Udphdr (user) */
struct Udphdr
{
    ipaddr	raddr;	/* V6 remote address */
    ipaddr	laddr;	/* V6 local address */
    ipaddr	ifcaddr;	/* V6 ifc addr msg was received on */

    uchar	rport[2];		/* remote port */
    uchar	lport[2];		/* local port */
};
/*e: struct Udphdr (user) */

uchar*	defmask(ipaddr);
void	maskip(ipaddr, ipaddr, ipaddr);
int		eipfmt(Fmt*);
bool	isv4(ipaddr);
vlong	parseip(uchar*, char*);
vlong	parseipmask(uchar*, char*);
char*	v4parseip(uchar*, char*);
//char*	v4parsecidr(uchar*, uchar*, char*);
int	parseether(uchar*, char*);
int	myipaddr(uchar*, char*);
int	myetheraddr(uchar*, char*);
int	equivip4(uchar*, uchar*);
int	equivip6(uchar*, uchar*);

Ipifc*	readipifc(char*, Ipifc*, int);

void	hnputs(void*, ushort);
void	hnputl(void*, uint);
void	hnputv(void*, uvlong);
ushort	nhgets(void*);
uint	nhgetl(void*);
uvlong	nhgetv(void*);

ushort	ptclbsum(uchar*, int);

int		v6tov4(uchar*, uchar*);
void	v4tov6(uchar*, uchar*);

/*s: macro ipcmp */
#define	ipcmp(x, y) memcmp(x, y, IPaddrlen)
/*e: macro ipcmp */
/*s: macro ipmove */
#define	ipmove(x, y) memmove(x, y, IPaddrlen)
/*e: macro ipmove */

extern ipaddr IPv4bcast;
extern ipaddr IPv4bcastobs;
extern ipaddr IPv4allsys;
extern ipaddr IPv4allrouter;
extern ipaddr IPnoaddr;
extern ipaddr v4prefix;
extern ipaddr IPallbits;

/*s: macro CLASS */
#define CLASS(p) ((*(uchar*)(p))>>6)
/*e: macro CLASS */

#pragma	varargck	type	"I"	uchar*
#pragma	varargck	type	"V"	uchar*
#pragma	varargck	type	"E"	uchar*
#pragma	varargck	type	"M"	uchar*
/*e: include/net/ip.h */
