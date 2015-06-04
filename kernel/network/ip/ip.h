/*s: kernel/network/ip/ip.h */

// This file references also code in lib_networking (linked with the kernel).
// Those functions are also exported in include/ip.h.

// forward decl
typedef struct  Conv  Conv;
typedef struct  Fragment4 Fragment4;
typedef struct  Fragment6 Fragment6;
typedef struct  Fs  Fs;
typedef union Hwaddr  Hwaddr;
typedef struct  IP  IP;
typedef struct  IPaux IPaux;
typedef struct  Ip4hdr  Ip4hdr;
typedef struct  Ipfrag  Ipfrag;
typedef struct  Ipself  Ipself;
typedef struct  Ipselftab Ipselftab;
typedef struct  Iplink  Iplink;
typedef struct  Iplifc  Iplifc;
typedef struct  Ipmulti Ipmulti;
typedef struct  Ipifc Ipifc;
typedef struct  Iphash  Iphash;
typedef struct  Ipht  Ipht;
typedef struct  Netlog  Netlog;
typedef struct  Medium  Medium;
typedef struct  Proto Proto;
typedef struct  Arpent  Arpent;
typedef struct  Arp Arp;
typedef struct  Route Route;

typedef struct  Routerparams  Routerparams;
typedef struct  Hostparams  Hostparams;
typedef struct  v6router  v6router;
typedef struct  v6params  v6params;

#pragma incomplete Arp
#pragma incomplete Ipself
#pragma incomplete Ipselftab
#pragma incomplete IP
#pragma incomplete Netlog

/*s: enum _anon_ (kernel/network/ip/ip.h) */
enum
{
  Addrlen=  64,
  /*s: constant Maxproto */
  Maxproto= 20,
  /*e: constant Maxproto */
  Nhash=    64,
  Maxincall=  32, /* max. conn.s in listen q not accepted yet */
  Nchans=   1024,
  MAClen=   16,   /* longest mac address */

  MAXTTL=   255,
  DFLTTOS=  0,

  IPaddrlen=  16,
  IPv4addrlen=  4,
  IPv4off=  12,
  IPllen=   4,

  /* ip versions */
  V4=   4,
  V6=   6,
  IP_VER4=  0x40,
  IP_VER6=  0x60,
  IP_HLEN4= 5,    /* v4: Header length in words */
  IP_DF=    0x4000,   /* v4: Don't fragment */
  IP_MF=    0x2000,   /* v4: More fragments */
  IP4HDR=   20,   /* sizeof(Ip4hdr) */
  IP_MAX=   64*1024,  /* Max. Internet packet size, v4 & v6 */

  /* 2^Lroot trees in the root table */
  Lroot=    10,

  Maxpath = 64,
};
/*e: enum _anon_ (kernel/network/ip/ip.h) */

/*s: enum _anon_ (kernel/network/ip/ip.h)2 */
enum
{
  Idle=   0,
  Announcing= 1,
  Announced=  2,
  Connecting= 3,
  Connected=  4,
};
/*e: enum _anon_ (kernel/network/ip/ip.h)2 */

/*s: enum _anon_ (kernel/network/ip/ip.h)3 */
/* MIB II counters */
enum
{
  Forwarding,
  DefaultTTL,
  InReceives,
  InHdrErrors,
  InAddrErrors,
  ForwDatagrams,
  InUnknownProtos,
  InDiscards,
  InDelivers,
  OutRequests,
  OutDiscards,
  OutNoRoutes,
  ReasmTimeout,
  ReasmReqds,
  ReasmOKs,
  ReasmFails,
  FragOKs,
  FragFails,
  FragCreates,

  Nipstats,
};
/*e: enum _anon_ (kernel/network/ip/ip.h)3 */

/*s: struct Fragment4 */
struct Fragment4
{
  Block*  blist;

  ulong   src;
  ulong   dst;
  ushort  id;
  ulong   age;

  // Extra
  Fragment4*  next;
};
/*e: struct Fragment4 */

/*s: struct Fragment6 */
struct Fragment6
{
  Block*  blist;
  Fragment6*  next;
  uchar   src[IPaddrlen];
  uchar   dst[IPaddrlen];
  uint  id;
  ulong   age;
};
/*e: struct Fragment6 */

/*s: struct Ipfrag */
//@Scheck: used only for its macro below, could maybe simplify?
struct Ipfrag
{
  ushort  foff;
  ushort  flen;

  uchar payload[];
};
/*e: struct Ipfrag */

/*s: constant IPFRAGSZ */
#define IPFRAGSZ offsetof(Ipfrag, payload[0])
/*e: constant IPFRAGSZ */

/*s: struct IP (kernel) */
/* an instance of IP */
struct IP
{
  Fragment4*  flisthead4;
  Fragment4*  fragfree4;

  /*s: [[IP(kernel)]] stat fields */
  uvlong    stats[Nipstats];
  /*e: [[IP(kernel)]] stat fields */
  /*s: [[IP(kernel)]] routing fields */
  bool iprouting;  /* true if we route like a gateway */
  /*e: [[IP(kernel)]] routing fields */

  /*s: [[IP(kernel)]] ipv6 fields */
    QLock   fraglock6;
    Fragment6*  flisthead6;
    Fragment6*  fragfree6;
    Ref   id6;
  /*e: [[IP(kernel)]] ipv6 fields */

  // Extra
  QLock   fraglock4;
  Ref   id4;
};
/*e: struct IP (kernel) */

/*s: struct Ip4hdr */
/* on the wire packet header */
struct Ip4hdr
{
  uchar vihl;   /* Version and header length */
  uchar tos;    /* Type of service */
  uchar length[2];  /* packet length */
  uchar id[2];    /* ip->identification */
  uchar frag[2];  /* Fragment information */
  uchar ttl;        /* Time to live */
  uchar proto;    /* Protocol */
  uchar cksum[2]; /* Header checksum */
  uchar src[4];   /* IP source */
  uchar dst[4];   /* IP destination */
};
/*e: struct Ip4hdr */

/*s: struct Conv (kernel) */
/*
 *  one per conversation directory
 */
struct Conv
{

  uchar laddr[IPaddrlen]; /* local IP address */
  uchar raddr[IPaddrlen]; /* remote IP address */

  ushort  lport;      /* local port number */
  ushort  rport;      /* remote port number */

  uchar ipversion;

  char  *owner;     /* protections */
  int perm;

  /*s: [[Conv(kernel)]] routing fields */
  Route *r;     /* last route used */
  ulong rgen;     /* routetable generation for *r */
  /*e: [[Conv(kernel)]] routing fields */
  /*s: [[Conv(kernel)]] multicast fields */
  Ipmulti *multi;     /* multicast bindings for this interface */
  /*e: [[Conv(kernel)]] multicast fields */
  /*s: [[Conv(kernel)]] other fields */
  int restricted;   /* remote port is restricted */
  uint  ttl;      /* max time to live */
  uint  tos;      /* type of service */
  bool ignoreadvice;   /* don't terminate connection on icmp errors */

  int inuse;      /* opens of listen/data/ctl */
  int length;
  int state;

  int maxfragsize;    /* If set, used for fragmentation */

  /* udp specific */
  int headers;    /* data src/dst headers in udp */
  int reliable;   /* true if reliable udp */

  Conv* incall;     /* calls waiting to be listened for */
  Conv* next;

  Queue*  rq;     /* queued data waiting to be read */
  Queue*  wq;     /* queued data waiting to be written */
  Queue*  eq;     /* returned error packets */
  Queue*  sq;     /* snooping queue */
  Ref snoopers;   /* number of processes with snoop open */

  QLock car;
  Rendez  cr;
  char  cerr[ERRMAX];

  QLock listenq;
  Rendez  listenr;

  void* ptcl;     /* protocol specific stuff */
  /*e: [[Conv(kernel)]] other fields */

  // Extra
  QLock;
  /*s: [[Conv(kernel)]] extra fields */
  // ref<Proto> reverse of Proto.conv[this.x]
  Proto*  p;
  // index in Proto.conv[]
  int x;      /* conversation index */
  /*e: [[Conv(kernel)]] extra fields */
};
/*e: struct Conv (kernel) */

/*s: struct Medium (kernel) */
struct Medium
{
  char  *name;

  int hsize;    /* medium header size */
  int mintu;    /* default min mtu */
  int maxtu;    /* default max mtu */
  int maclen;   /* mac address length  */
 
  // the methods
  void  (*bind)(Ipifc*, int, char**);
  void  (*unbind)(Ipifc*);

  void  (*bwrite)(Ipifc *ifc, Block *b, int version, uchar *ip);

  /*s: [[Medium(kernel)]] multicast methods */
  /* for arming interfaces to receive multicast */
  void  (*addmulti)(Ipifc *ifc, uchar *a, uchar *ia);
  void  (*remmulti)(Ipifc *ifc, uchar *a, uchar *ia);

  /* for routing multicast groups */
  void  (*joinmulti)(Ipifc *ifc, uchar *a, uchar *ia);
  void  (*leavemulti)(Ipifc *ifc, uchar *a, uchar *ia);
  /*e: [[Medium(kernel)]] multicast methods */
  /*s: [[Medium(kernel)]] other methods */

  /* process packets written to 'data' */
  void  (*pktin)(Fs *f, Ipifc *ifc, Block *bp);

  /* routes for router boards */
  void  (*addroute)(Ipifc *ifc, int, uchar*, uchar*, uchar*, int);
  void  (*remroute)(Ipifc *ifc, int, uchar*, uchar*);
  void  (*flushroutes)(Ipifc *ifc);


  /* address resolution */
  void  (*ares)(Fs*, int, uchar*, uchar*, int, int);  /* resolve */
  void  (*areg)(Ipifc*, uchar*);      /* register */

  /* v6 address generation */
  void  (*pref2addr)(uchar *pref, uchar *ea);
  /*e: [[Medium(kernel)]] other methods */
  /*s: [[Medium(kernel)]] other fields */
  bool unbindonclose;  /* if non-zero, unbind on last close */
  /*e: [[Medium(kernel)]] other fields */

};
/*e: struct Medium (kernel) */

/*s: struct Iplifc (kernel) */
/* logical interface associated with a physical one */
struct Iplifc
{
  uchar local[IPaddrlen];
  uchar mask[IPaddrlen];
  uchar remote[IPaddrlen];
  uchar net[IPaddrlen];

  /*s: [[Iplifc(kernel)]] other fields */
  uchar tentative;  /* =1 => v6 dup disc on, =0 => confirmed unique */
  uchar onlink;   /* =1 => onlink, =0 offlink. */
  uchar autoflag; /* v6 autonomous flag */
  long  validlt;  /* v6 valid lifetime */
  long  preflt;   /* v6 preferred lifetime */
  long  origint;  /* time when addr was added */
  /*e: [[Iplifc(kernel)]] other fields */

  // Extra
  /*s: [[Iplifc(kernel)]] extra fields */
  // list<ref_own<Iplifc>>, head = Ipifc.lifc
  Iplifc  *next;
  /*x: [[Iplifc(kernel)]] extra fields */
  Iplink  *link;    /* addresses linked to this lifc */
  /*e: [[Iplifc(kernel)]] extra fields */
};
/*e: struct Iplifc (kernel) */

/*s: struct Iplink */
/* binding twixt Ipself and Iplifc */
struct Iplink
{
  Ipself  *self;
  Iplifc  *lifc;
  Iplink  *selflink;  /* next link for this local address */
  Iplink  *lifclink;  /* next link for this ifc */
  ulong expire;
  Iplink  *next;    /* free list */
  int ref;
};
/*e: struct Iplink */

/* rfc 2461, pp.40—43. */

/*s: struct Routerparams */
/* default values, one per stack */
struct Routerparams {
  int mflag;    /* flag: managed address configuration */
  int oflag;    /* flag: other stateful configuration */
  int   maxraint; /* max. router adv interval (ms) */
  int minraint; /* min. router adv interval (ms) */
  int linkmtu;  /* mtu options */
  int reachtime;  /* reachable time */
  int rxmitra;  /* retransmit interval */
  int ttl;    /* cur hop count limit */
  int routerlt; /* router lifetime */
};
/*e: struct Routerparams */

/*s: struct Hostparams */
struct Hostparams {
  int rxmithost;
};
/*e: struct Hostparams */

/*s: struct Ipifc (kernel) */
struct Ipifc
{
  char  dev[64];  /* device we're attached to */

  int maxtu;    /* Maximum transfer unit */
  int mintu;    /* Minumum tranfer unit */
  int mbps;   /* megabits per second */

  Medium  *m;   /* Media pointer */
  void  *arg;   /* medium specific */
  uchar mac[MAClen];  /* MAC address */

  // list<ref_own<Iplifc>>, next = Iplifc.next
  Iplifc  *lifc;    /* logical interfaces on this physical one */

  /*s: [[Ipifc(kernel)]] stat fields */
  ulong in, out;  /* message statistics */
  ulong inerr, outerr;  /* ... */
  /*e: [[Ipifc(kernel)]] stat fields */
  /*s: [[Ipifc(kernel)]] ipv6 fields */
  uchar sendra6;  /* flag: send router advs on this ifc */
  uchar recvra6;  /* flag: recv router advs on this ifc */
  /*e: [[Ipifc(kernel)]] ipv6 fields */
  /*s: [[Ipifc(kernel)]] other fields */
  Conv  *conv;    /* link to its conversation structure */
  bool reassemble; /* reassemble IP packets before forwarding */

  /* these are used so that we can unbind on the fly */
  Lock  idlock;
  uchar ifcid;    /* incremented each 'bind/unbind/add/remove' */
  int ref;    /* number of proc's using this ipifc */
  Rendez  wait;   /* where unbinder waits for ref == 0 */
  int unbinding;
  /*x: [[Ipifc(kernel)]] other fields */
  Routerparams rp;  /* router parameters as in RFC 2461, pp.40—43.
          used only if node is router */
  /*e: [[Ipifc(kernel)]] other fields */

  //Extra
  RWlock;

};
/*e: struct Ipifc (kernel) */

/*s: struct Ipmulti */
/*
 *  one per multicast-lifc pair used by a Conv
 */
struct Ipmulti
{
  uchar ma[IPaddrlen];
  uchar ia[IPaddrlen];
  Ipmulti *next;
};
/*e: struct Ipmulti */

/*s: enum _anon_ (kernel/network/ip/ip.h)4 */
/*
 *  hash table for 2 ip addresses + 2 ports
 */
enum
{
  Nipht=    521,  /* convenient prime */

  IPmatchexact= 0,  /* match on 4 tuple */
  IPmatchany,   /* *!* */
  IPmatchport,    /* *!port */
  IPmatchaddr,    /* addr!* */
  IPmatchpa,    /* addr!port */
};
/*e: enum _anon_ (kernel/network/ip/ip.h)4 */

/*s: struct Iphash */
struct Iphash
{
  Iphash  *next;
  Conv  *c;
  int match;
};
/*e: struct Iphash */
/*s: struct Ipht */
struct Ipht
{
  Lock;
  Iphash  *tab[Nipht];
};
/*e: struct Ipht */

void iphtadd(Ipht*, Conv*);
void iphtrem(Ipht*, Conv*);
Conv* iphtlook(Ipht *ht, uchar *sa, ushort sp, uchar *da, ushort dp);

/*s: struct Proto (kernel) */
/*
 *  one per multiplexed protocol
 */
struct Proto
{
  char*   name;   /* protocol name */

  /*s: [[Proto(kernel)]] methods */
  char*   (*connect)(Conv*, char**, int);
  char*   (*announce)(Conv*, char**, int);
  char*   (*bind)(Conv*, char**, int);
  int   (*state)(Conv*, char*, int);
  void    (*create)(Conv*);
  void    (*close)(Conv*);
  void    (*rcv)(Proto*, Ipifc*, Block*);
  char*   (*ctl)(Conv*, char**, int);
  void    (*advise)(Proto*, Block*, char*);
  int   (*stats)(Proto*, char*, int);
  int   (*local)(Conv*, char*, int);
  int   (*remote)(Conv*, char*, int);
  int   (*inuse)(Conv*);
  int   (*gc)(Proto*);  /* returns true if any conversations are freed */
  /*e: [[Proto(kernel)]] methods */

  // growing_array<ref_own<Proto>>, size = Proto.nc
  Conv    **conv;   /* array of conversations */
  int   nc;   /* number of conversations */
  int   ac; // number of active conversations??

  /*s: [[Proto(kernel)]] other fields */
  int   ipproto;  /* ip protocol type */


  int   ptclsize; /* size of per protocol ctl block */
  Qid   qid;    /* qid for protocol directory */
  ushort    nextrport;
  /*e: [[Proto(kernel)]] other fields */

  void    *priv;

  // Extra
  QLock;
  /*s: [[Proto(kernel)]] extra fields */
  // ref<Fs>, reverse of Fs.p[this.x]
  Fs    *f;   /* file system this proto is part of */
  // index in Fs.p[]
  int   x;    /* protocol index */
  /*e: [[Proto(kernel)]] extra fields */
};
/*e: struct Proto (kernel) */


/*s: struct Fs (kernel) */
/*
 *  one per IP protocol stack
 */
struct Fs
{
  // array<option<ref_own<Proto>>>, size is Fs.np
  Proto*  p[Maxproto+1];    /* list of supported protocols */
  int np;

  IP  *ip;
  Ipselftab *self;

  /*s: [[Fs(kernel)]] arp fields */
  Arp *arp;
  /*e: [[Fs(kernel)]] arp fields */
  /*s: [[Fs(kernel)]] routing fields */
  Route *v4root[1<<Lroot];  /* v4 routing forest */
  Route *queue;     /* used as temp when reinjecting routes */
  /*e: [[Fs(kernel)]] routing fields */
  /*s: [[Fs(kernel)]] ndb fields */
  char  ndb[1024];    /* an ndb entry for this interface */
  int ndbvers;
  long  ndbmtime;
  /*e: [[Fs(kernel)]] ndb fields */
  /*s: [[Fs(kernel)]] logging fields */
  Netlog  *alog;
  /*e: [[Fs(kernel)]] logging fields */
  /*s: [[Fs(kernel)]] ipv6 fields */
    v6params  *v6p;
    Route *v6root[1<<Lroot];  /* v6 routing forest */
  /*e: [[Fs(kernel)]] ipv6 fields */
  /*s: [[Fs(kernel)]] other fields */
  Proto*  t2p[256];   /* vector of all protocols */

  Proto*  ipifc;      /* kludge for ipifcremroute & ipifcaddroute */
  Proto*  ipmux;      /* kludge for finding an ip multiplexor */
  /*e: [[Fs(kernel)]] other fields */
 
  // Extra
  RWlock;
  /*s: [[Fs(kernel)]] extra fields */
  int dev; // idx in ipfs
  /*e: [[Fs(kernel)]] extra fields */
};
/*e: struct Fs (kernel) */

/*s: struct v6router */
/* one per default router known to host */
struct v6router {
  uchar inuse;
  Ipifc *ifc;
  int ifcid;
  uchar routeraddr[IPaddrlen];
  long  ltorigin;
  Routerparams  rp;
};
/*e: struct v6router */

/*s: struct v6params */
struct v6params
{
  Routerparams  rp;   /* v6 params, one copy per node now */
  Hostparams  hp;
  v6router  v6rlist[3]; /* max 3 default routers, currently */
  int   cdrouter; /* uses only v6rlist[cdrouter] if   */
          /* cdrouter >= 0. */
};
/*e: struct v6params */

int Fsconnected(Conv*, char*);
Conv* Fsnewcall(Conv*, uchar*, ushort, uchar*, ushort, uchar);
//int Fspcolstats(char*, int);
int Fsproto(Fs*, Proto*);
//int Fsbuiltinproto(Fs*, uchar);
//Conv* Fsprotoclone(Proto*, char*);
Proto*  Fsrcvpcol(Fs*, uchar);
Proto*  Fsrcvpcolx(Fs*, uchar);
char* Fsstdconnect(Conv*, char**, int);
char* Fsstdannounce(Conv*, char**, int);
//char* Fsstdbind(Conv*, char**, int);
ulong scalednconv(void);
//void  closeconv(Conv*);
/*s: enum _anon_ (kernel/network/ip/ip.h)5 */
/*
 *  logging
 */
enum
{
  Logip=    1<<1,
  Logtcp=   1<<2,
  Logfs=    1<<3,
  Logicmp=  1<<5,
  Logudp=   1<<6,
  Logcompress=  1<<7,
  Loggre=   1<<9,
  Logppp=   1<<10,
  Logtcprxmt= 1<<11,
  Logigmp=  1<<12,
  Logudpmsg=  1<<13,
  Logipmsg= 1<<14,
  Logrudp=  1<<15,
  Logrudpmsg= 1<<16,
  Logesp=   1<<17,
  Logtcpwin=  1<<18,
};
/*e: enum _anon_ (kernel/network/ip/ip.h)5 */

void  netloginit(Fs*);
void  netlogopen(Fs*);
void  netlogclose(Fs*);
void  netlogctl(Fs*, char*, int);
long  netlogread(Fs*, void*, ulong, long);
void  netlog(Fs*, int, char*, ...);
//void  ifcloginit(Fs*);
//long  ifclogread(Fs*, Chan *,void*, ulong, long);
//void  ifclog(Fs*, uchar *, int);
//void  ifclogopen(Fs*, Chan*);
//void  ifclogclose(Fs*, Chan*);

#pragma varargck argpos netlog  3

/*
 *  iproute.c
 */
typedef struct RouteTree RouteTree;
typedef struct Routewalk Routewalk;
typedef struct V4route V4route;
typedef struct V6route V6route;

/*s: enum _anon_ (kernel/network/ip/ip.h)6 */
enum
{

  /* type bits */
  Rv4=    (1<<0),   /* this is a version 4 route */
  Rifc=   (1<<1),   /* this route is a directly connected interface */
  Rptpt=    (1<<2),   /* this route is a pt to pt interface */
  Runi=   (1<<3),   /* a unicast self address */
  Rbcast=   (1<<4),   /* a broadcast self address */
  Rmulti=   (1<<5),   /* a multicast self address */
  Rproxy=   (1<<6),   /* this route should be proxied */
};
/*e: enum _anon_ (kernel/network/ip/ip.h)6 */

/*s: struct Routewalk */
struct Routewalk
{
  int o;
  int h;
  char* p;
  char* e;
  void* state;
  void  (*walk)(Route*, Routewalk*);
};
/*e: struct Routewalk */

/*s: struct RouteTree */
struct  RouteTree
{
  Route*  right;
  Route*  left;
  Route*  mid;
  uchar depth;
  uchar type;
  uchar ifcid;    /* must match ifc->id */
  Ipifc *ifc;
  char  tag[4];
  int ref;
};
/*e: struct RouteTree */

/*s: struct V4route */
struct V4route
{
  ulong address;
  ulong endaddress;
  uchar gate[IPv4addrlen];
};
/*e: struct V4route */

/*s: struct V6route */
struct V6route
{
  ulong address[IPllen];
  ulong endaddress[IPllen];
  uchar gate[IPaddrlen];
};
/*e: struct V6route */

/*s: struct Route */
struct Route
{
  RouteTree;

  union {
    V6route v6;
    V4route v4;
  };
};
/*e: struct Route */
extern void v4addroute(Fs *f, char *tag, uchar *a, uchar *mask, uchar *gate, int type);
extern void v6addroute(Fs *f, char *tag, uchar *a, uchar *mask, uchar *gate, int type);
extern void v4delroute(Fs *f, uchar *a, uchar *mask, int dolock);
extern void v6delroute(Fs *f, uchar *a, uchar *mask, int dolock);
extern Route* v4lookup(Fs *f, uchar *a, Conv *c);
extern Route* v6lookup(Fs *f, uchar *a, Conv *c);
extern long routeread(Fs *f, char*, ulong, int);
extern long routewrite(Fs *f, Chan*, char*, int);
extern void routetype(int, char*);
//extern void ipwalkroutes(Fs*, Routewalk*);
//extern void convroute(Route*, uchar*, uchar*, uchar*, char*, int*);

/*
 *  devip.c
 */

/*s: struct IPaux */
/*
 *  Hanging off every ip channel's ->aux is the following structure.
 *  It maintains the state used by devip and iproute.
 */
struct IPaux
{
  char  *owner;   /* the user that did the attach */
  char  tag[4];
};
/*e: struct IPaux */

extern IPaux* newipaux(char*, char*);

/*s: struct Arpent */
/*
 *  arp.c
 */
struct Arpent
{
  uchar ip[IPaddrlen];
  uchar mac[MAClen];
  Medium  *type;      /* media type */
  Arpent* hash;
  Block*  hold;
  Block*  last;
  uint  ctime;      /* time entry was created or refreshed */
  uint  utime;      /* time entry was last used */
  uchar state;
  Arpent  *nextrxt;   /* re-transmit chain */
  uint  rtime;      /* time for next retransmission */
  uchar rxtsrem;
  Ipifc *ifc;
  uchar ifcid;      /* must match ifc->id */
};
/*e: struct Arpent */

extern void arpinit(Fs*);
extern int  arpread(Arp*, char*, ulong, int);
extern int  arpwrite(Fs*, char*, int);
extern Arpent*  arpget(Arp*, Block *bp, int version, Ipifc *ifc, uchar *ip, uchar *h);
extern void arprelease(Arp*, Arpent *a);
extern Block* arpresolve(Arp*, Arpent *a, Medium *type, uchar *mac);
extern void arpenter(Fs*, int version, uchar *ip, uchar *mac, int len, int norefresh);

/*
 * ipaux.c
 */

//extern int  myetheraddr(uchar*, char*);
extern vlong  parseip(uchar*, char*);
extern vlong  parseipmask(uchar*, char*);
//extern char*  v4parseip(uchar*, char*);
extern void maskip(uchar *from, uchar *mask, uchar *to);
extern int  parsemac(uchar *to, char *from, int len);
extern uchar* defmask(uchar*);
extern int  isv4(uchar*);
extern void v4tov6(uchar *v6, uchar *v4);
extern int  v6tov4(uchar *v4, uchar *v6);
extern int  eipfmt(Fmt*);

/*s: macro ipmove (kernel/network/ip/ip.h) */
#define ipmove(x, y) memmove(x, y, IPaddrlen)
/*e: macro ipmove (kernel/network/ip/ip.h) */
/*s: macro ipcmp (kernel/network/ip/ip.h) */
#define ipcmp(x, y) ( (x)[IPaddrlen-1] != (y)[IPaddrlen-1] || memcmp(x, y, IPaddrlen) )
/*e: macro ipcmp (kernel/network/ip/ip.h) */

extern uchar IPv4bcast[IPaddrlen];
//extern uchar IPv4bcastobs[IPaddrlen];
//extern uchar IPv4allsys[IPaddrlen];
//extern uchar IPv4allrouter[IPaddrlen];
extern uchar IPnoaddr[IPaddrlen];
extern uchar v4prefix[IPaddrlen];
extern uchar IPallbits[IPaddrlen];

/*s: constant NOW */
#define NOW TK2MS(CPUS(0)->ticks)
/*e: constant NOW */

/*
 *  media
 */
//extern Medium ethermedium;
//extern Medium nullmedium;
//extern Medium pktmedium;

/*
 *  ipifc.c
 */
extern Medium*  ipfindmedium(char *name);
extern void addipmedium(Medium *med);
extern int  ipforme(Fs*, uchar *addr);
extern int  iptentative(Fs*, uchar *addr);
//extern int  ipisbm(uchar *);
extern int  ipismulticast(uchar *);
extern Ipifc* findipifc(Fs*, uchar *remote, int type);
extern void findlocalip(Fs*, uchar *local, uchar *remote);
extern int  ipv4local(Ipifc *ifc, uchar *addr);
extern int  ipv6local(Ipifc *ifc, uchar *addr);
extern int  ipv6anylocal(Ipifc *ifc, uchar *addr);
extern Iplifc*  iplocalonifc(Ipifc *ifc, uchar *ip);
extern int  ipproxyifc(Fs *f, Ipifc *ifc, uchar *ip);
extern int  ipismulticast(uchar *ip);
//extern int  ipisbooting(void);
//extern int  ipifccheckin(Ipifc *ifc, Medium *med);
//extern void ipifccheckout(Ipifc *ifc);
//extern int  ipifcgrab(Ipifc *ifc);
extern void ipifcaddroute(Fs*, int, uchar*, uchar*, uchar*, int);
extern void ipifcremroute(Fs*, int, uchar*, uchar*);
extern void ipifcremmulti(Conv *c, uchar *ma, uchar *ia);
extern void ipifcaddmulti(Conv *c, uchar *ma, uchar *ia);
//extern char*  ipifcrem(Ipifc *ifc, char **argv, int argc);
//extern char*  ipifcadd(Ipifc *ifc, char **argv, int argc, int tentative, Iplifc *lifcp);
extern long ipselftabread(Fs*, char *a, ulong offset, int n);
//extern char*  ipifcadd6(Ipifc *ifc, char**argv, int argc);

/*
 *  ip.c
 */
extern void iprouting(Fs*, int);
extern void icmpnoconv(Fs*, Block*);
extern void icmpcantfrag(Fs*, Block*, int);
extern void icmpttlexceeded(Fs*, uchar*, Block*);
extern ushort ipcsum(uchar*);
extern void ipiput4(Fs*, Ipifc*, Block*);
extern void ipiput6(Fs*, Ipifc*, Block*);
extern int  ipoput4(Fs*, Block*, int, int, int, Conv*);
extern int  ipoput6(Fs*, Block*, int, int, int, Conv*);
extern int  ipstats(Fs*, char*, int);
extern ushort ptclbsum(uchar*, int);
extern ushort ptclcsum(Block*, int, int);
extern void ip_init(Fs*);
//extern void update_mtucache(uchar*, ulong);
//extern ulong  restrict_mtu(uchar*, ulong);

/*
 * bootp.c
 */
extern int  bootpread(char*, ulong, int);

/*
 * chandial.c
 */
extern Chan*  chandial(char*, char*, char*, Chan**);

/*
 *  global to all of the stack
 */
//extern void (*igmpreportfn)(Ipifc*, uchar*);
/*e: kernel/network/ip/ip.h */
