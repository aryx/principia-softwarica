/*s: networking/ip/ipconfig/ipconfig.h */
typedef struct Conf Conf;
typedef struct Ctl Ctl;

/*s: struct Conf */
struct Conf
{
    /* locally generated */
    char	*type;
    char	*dev;
    char	mpoint[32];
    int	cfd;			/* ifc control channel */
    int	dfd;			/* ifc data channel (for ppp) */
    char	*cputype;
    uchar	hwa[32];		/* hardware address */
    int	hwatype;
    int	hwalen;
    uchar	cid[32];
    int	cidlen;
    char	*baud;

    /* learned info */
    uchar	gaddr[IPaddrlen];
    uchar	laddr[IPaddrlen];
    uchar	mask[IPaddrlen];
    uchar	raddr[IPaddrlen];
    uchar	dns[2*IPaddrlen];
    uchar	fs[2*IPaddrlen];
    uchar	auth[2*IPaddrlen];
    uchar	ntp[IPaddrlen];
    int	mtu;

    /* dhcp specific */
    int	state;
    int	fd;
    ulong	xid;
    ulong	starttime;
    char	sname[64];
    char	hostname[32];
    char	domainname[64];
    uchar	server[IPaddrlen];	/* server IP address */
    ulong	offered;		/* offered lease time */
    ulong	lease;			/* lease time */
    ulong	resend;			/* # of resends for current state */
    ulong	timeout;		/* time to timeout - seconds */

    /*
     * IPv6
     */

    /* solicitation specific - XXX add support for IPv6 leases */
//	ulong	solicit_retries;

    /* router-advertisement related */
    uchar	sendra;
    uchar	recvra;
    uchar	mflag;
    uchar	oflag;
    int 	maxraint; /* rfc2461, p.39: 4sec ≤ maxraint ≤ 1800sec, def 600 */
    int	minraint;	/* 3sec ≤ minraint ≤ 0.75*maxraint */
    int	linkmtu;
    int	reachtime;	/* 3,600,000 msec, default 0 */
    int	rxmitra;	/* default 0 */
    int	ttl;		/* default 0 (unspecified) */
    /* default gateway params */
    uchar	v6gaddr[IPaddrlen];
    int	routerlt;	/* router life time */

    /* prefix related */
    uchar	v6pref[IPaddrlen];
    int	prefixlen;
    uchar	onlink;		/* flag: address is `on-link' */
    uchar	autoflag;	/* flag: autonomous */
    ulong	validlt;	/* valid lifetime (seconds) */
    ulong	preflt;		/* preferred lifetime (seconds) */
};
/*e: struct Conf */

/*s: struct Ctl */
struct Ctl
{
    Ctl	*next;
    char	*ctl;
};
/*e: struct Ctl */

extern Ctl *firstctl, **ctll;

extern Conf conf;

extern int	noconfig;
extern int	ipv6auto;
extern int	debug;
extern int	dodhcp;
extern int	dolog;
extern int	nip;
extern int	plan9;
extern int	dupl_disc;

extern int	myifc;
extern char	*vs;

void	adddefroute(char*, uchar*);
void	binddevice(void);
void	bootprequest(void);
void	controldevice(void);
void	dhcpquery(int, int);
void	dhcprecv(void);
void	dhcpsend(int);
int	dhcptimer(void);
void	dhcpwatch(int);
void	doadd(int);
void	doremove(void);
void	dounbind(void);
int	getndb(void);
int	ipconfig4(void);
int	ipconfig6(int);
long	jitter(void);
void	lookforip(char*);
void	mkclientid(void);
int	nipifcs(char*);
int	openlisten(void);
uchar	*optaddaddr(uchar*, int, uchar*);
uchar	*optaddbyte(uchar*, int, int);
uchar	*optaddstr(uchar*, int, char*);
uchar	*optadd(uchar*, int, void*, int);
uchar	*optaddulong(uchar*, int, ulong);
uchar	*optaddvec(uchar*, int, uchar*, int);
int	optgetaddrs(uchar*, int, uchar*, int);
int	optgetaddr(uchar*, int, uchar*);
int	optgetbyte(uchar*, int);
int	optgetstr(uchar*, int, char*, int);
uchar	*optget(uchar*, int, int*);
ulong	optgetulong(uchar*, int);
int	optgetvec(uchar*, int, uchar*, int);
int	parseoptions(uchar *p, int n);
int	parseverb(char*);
void	procsetname(char *fmt, ...);
void	putndb(void);
ulong	randint(ulong low, ulong hi);
void	tweakservers(void);
void	usage(void);
int	validip(uchar*);
void	warning(char *fmt, ...);
void	writendb(char*, int, int);

/*
 * IPv6
 */

void	doipv6(int);
int	ipconfig6(int);
void	recvra6(void);
void	sendra6(void);
void	v6paraminit(Conf *);

typedef struct Headers Headers;
typedef struct Ip4hdr  Ip4hdr;
typedef struct Lladdropt Lladdropt;
typedef struct Mtuopt Mtuopt;
typedef struct Prefixopt Prefixopt;
typedef struct Routeradv Routeradv;
typedef struct Routersol Routersol;

/*s: enum _anon_ (networking/ip/ipconfig/ipconfig.h) */
enum {
    IsRouter 	= 1,
    IsHostRecv	= 2,
    IsHostNoRecv	= 3,

    MAClen		= 6,

    IPv4		= 4,
    IPv6		= 6,
    Defmtu		= 1400,

    IP_HOPBYHOP	= 0,
    ICMPv4		= 1,
    IP_IGMPPROTO	= 2,
    IP_TCPPROTO	= 6,
    IP_UDPPROTO	= 17,
    IP_ILPROTO	= 40,
    IP_v6ROUTE	= 43,
    IP_v6FRAG	= 44,
    IP_IPsecESP	= 50,
    IP_IPsecAH	= 51,
    IP_v6NOMORE	= 59,
    ICMP6_RS	= 133,
    ICMP6_RA	= 134,

    IP_IN_IP	= 41,
};
/*e: enum _anon_ (networking/ip/ipconfig/ipconfig.h) */

/*s: enum _anon_ (networking/ip/ipconfig/ipconfig.h)2 */
enum {
    MFMASK = 1 << 7,
    OCMASK = 1 << 6,
    OLMASK = 1 << 7,
    AFMASK = 1 << 6,
};
/*e: enum _anon_ (networking/ip/ipconfig/ipconfig.h)2 */

/*s: enum _anon_ (networking/ip/ipconfig/ipconfig.h)3 */
enum {
    MAXTTL		= 255,
    D64HLEN		= IPV6HDR_LEN - IPV4HDR_LEN,
    IP_MAX		= 32*1024,
};
/*e: enum _anon_ (networking/ip/ipconfig/ipconfig.h)3 */

/*s: struct Headers */
struct Headers {
    uchar	dst[IPaddrlen];
    uchar	src[IPaddrlen];
};
/*e: struct Headers */

/*s: struct Routersol */
struct Routersol {
    uchar	vcf[4];		/* version:4, traffic class:8, flow label:20 */
    uchar	ploadlen[2];	/* payload length: packet length - 40 */
    uchar	proto;		/* next header	type */
    uchar	ttl;		/* hop limit */
    uchar	src[IPaddrlen];
    uchar	dst[IPaddrlen];
    uchar	type;
    uchar	code;
    uchar	cksum[2];
    uchar	res[4];
};
/*e: struct Routersol */

/*s: struct Routeradv */
struct Routeradv {
    uchar	vcf[4];		/* version:4, traffic class:8, flow label:20 */
    uchar	ploadlen[2];	/* payload length: packet length - 40 */
    uchar	proto;		/* next header	type */
    uchar	ttl;		/* hop limit */
    uchar	src[IPaddrlen];
    uchar	dst[IPaddrlen];
    uchar	type;
    uchar	code;
    uchar	cksum[2];
    uchar	cttl;
    uchar	mor;
    uchar	routerlt[2];
    uchar	rchbltime[4];
    uchar	rxmtimer[4];
};
/*e: struct Routeradv */

/*s: struct Lladdropt */
struct Lladdropt {
    uchar	type;
    uchar	len;
    uchar	lladdr[MAClen];
};
/*e: struct Lladdropt */

/*s: struct Prefixopt */
struct Prefixopt {
    uchar	type;
    uchar	len;
    uchar	plen;
    uchar	lar;
    uchar	validlt[4];
    uchar	preflt[4];
    uchar	reserv[4];
    uchar	pref[IPaddrlen];
};
/*e: struct Prefixopt */

/*s: struct Mtuopt */
struct Mtuopt {
    uchar	type;
    uchar	len;
    uchar	reserv[2];
    uchar	mtu[4];
};
/*e: struct Mtuopt */

void	ea2lla(uchar *lla, uchar *ea);
void	ipv62smcast(uchar *smcast, uchar *a);
/*e: networking/ip/ipconfig/ipconfig.h */
