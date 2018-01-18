/*s: kernel/network/netif.h */

// todo: split in portdat_network.h? and portfns_network.h

typedef struct Etherpkt Etherpkt;
typedef struct Netaddr  Netaddr;
typedef struct Netfile  Netfile;
typedef struct Netif  Netif;

/*s: enum [[_anon_ (kernel/network/netif.h)]] */
enum
{
  Nmaxaddr= 64,
  Nmhash=   31,

  Ncloneqid=  1,
  Naddrqid,
  N2ndqid,
  N3rdqid,
  Ndataqid,
  Nctlqid,
  Nstatqid,
  Ntypeqid,
  Nifstatqid,
  Nmtuqid,
};
/*e: enum [[_anon_ (kernel/network/netif.h)]] */

/*s: macro [[NETTYPE]] */
/*
 *  Macros to manage Qid's used for multiplexed devices
 */
#define NETTYPE(x)  (((ulong)x)&0x1f)
/*e: macro [[NETTYPE]] */
/*s: macro [[NETID]] */
#define NETID(x)  ((((ulong)x))>>5)
/*e: macro [[NETID]] */
/*s: macro [[NETQID]] */
#define NETQID(i,t) ((((ulong)i)<<5)|(t))
/*e: macro [[NETQID]] */

/*s: struct [[Netfile]] */
/*
 *  one per multiplexed connection
 */
struct Netfile
{
  int inuse;
  ulong mode;
  char  owner[KNAMELEN];

  Queue *in;      /* input buffer */

  int type;     /* multiplexor type */
  int prom;     /* promiscuous mode */
  int scan;     /* base station scanning interval */
  int bridge;     /* bridge mode */
  int headersonly;    /* headers only - no data */

  uchar maddr[8];   /* bitmask of multicast addresses requested */
  int nmaddr;     /* number of multicast addresses */

  // Extra
  QLock;
};
/*e: struct [[Netfile]] */

/*s: struct [[Netaddr]] */
/*
 *  a network address
 */
struct Netaddr
{
  Netaddr *next;    /* allocation chain */
  Netaddr *hnext;
  uchar addr[Nmaxaddr];
  int ref;
};
/*e: struct [[Netaddr]] */

/*s: struct Netif (kernel) */
/*
 *  a network interface
 */
struct Netif
{
  /* multiplexing */
  char  name[KNAMELEN];   /* for top level directory */

  // growing_array?<option<ref_own<Netfile>>>, size = nfile?
  Netfile **f;
  int nfile;      /* max number of Netfiles */

  /* about net */
  int alen;     /* address length */
  int link;     /* link status */

  int limit;      /* flow control */

  int minmtu;
  int maxmtu;
  int mtu;
  int mbps;     /* megabits per sec */

  uchar addr[Nmaxaddr];
  uchar bcast[Nmaxaddr];

  int prom;     /* number of promiscuous opens */
  int scan;     /* number of base station scanners */
  int all;      /* number of -1 multiplexors */

  /*s: [[Netif(kernel)]] stat fields */
  /* statistics */
  int misses;
  uvlong  inpackets;
  uvlong  outpackets;
  int crcs;   /* input crc errors */
  int oerrs;    /* output errors */
  int frames;   /* framing errors */
  int overflows;  /* packet overflows */
  int buffs;    /* buffering errors */
  int soverflows; /* software overflow */
  /*e: [[Netif(kernel)]] stat fields */

  /* routines for touching the hardware */
  void  *arg;

  /*s: [[Netif(kernel)]] methods */
  void  (*promiscuous)(void*, int);
  int (*hwmtu)(void*, int); /* get/set mtu */
  void  (*scanbs)(void*, uint); /* scan for base stations */

  /*s: [[Netif(kernel)]] multicast fields */
  Netaddr *maddr;     /* known multicast addresses */
  int nmaddr;     /* number of known multicast addresses */
  Netaddr *mhash[Nmhash];   /* hash table of multicast addresses */
  /*e: [[Netif(kernel)]] multicast fields */
  /*s: [[Netif(kernel)]] multicast methods */
  void  (*multicast)(void*, uchar*, int);
  /*e: [[Netif(kernel)]] multicast methods */
  /*e: [[Netif(kernel)]] methods */

  // Extra
  QLock;

};
/*e: struct Netif (kernel) */

void  netifinit(Netif*, char*, int, ulong);
Walkqid*  netifwalk(Netif*, Chan*, Chan*, char **, int);
Chan* netifopen(Netif*, Chan*, int);
void  netifclose(Netif*, Chan*);
long  netifread(Netif*, Chan*, void*, long, ulong);
Block*  netifbread(Netif*, Chan*, long, ulong);
long  netifwrite(Netif*, Chan*, void*, long);
int netifwstat(Netif*, Chan*, uchar*, int);
int netifstat(Netif*, Chan*, uchar*, int);
int activemulti(Netif*, uchar*, int);

/*s: enum [[_anon_ (kernel/network/netif.h)2]] */
/*
 *  Ethernet specific
 */
enum
{
  /*s: constant [[Eaddrlen]] */
  Eaddrlen= 6,
  /*e: constant [[Eaddrlen]] */
  ETHERMINTU =  60,   /* minimum transmit size */
  ETHERMAXTU =  1514,   /* maximum transmit size */
  ETHERHDRSIZE =  14,   /* size of an ethernet header */

  /* ethernet packet types */
  ETARP   = 0x0806,
  ETIP4   = 0x0800,

  ETIP6   = 0x86DD,
};
/*e: enum [[_anon_ (kernel/network/netif.h)2]] */

/*s: typedef eaddr */
typedef uchar eaddr[Eaddrlen];
/*e: typedef eaddr */

/*s: struct [[Etherpkt]] */
struct Etherpkt
{
  eaddr d;
  eaddr s;
  uchar type[2];

  uchar data[1500];
};
/*e: struct [[Etherpkt]] */


/*e: kernel/network/netif.h */
