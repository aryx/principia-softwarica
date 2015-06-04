/*s: kernel/network/386/etherif.h */
/*s: enum _anon_ (kernel/network/386/etherif.h) */
enum {
  /*s: constant MaxEther */
  MaxEther  = 48,
  /*e: constant MaxEther */
  Ntypes    = 8,
};
/*e: enum _anon_ (kernel/network/386/etherif.h) */

typedef struct Ether Ether;
/*s: struct Ether (kernel) */
struct Ether {
  ISAConf;      /* hardware info */

  int ctlrno;
  int tbdf;     /* type+busno+devno+funcno */
  uchar ea[Eaddrlen];

  void  (*attach)(Ether*);  /* filled in by reset routine */
  void  (*detach)(Ether*);
  void  (*transmit)(Ether*);
  void  (*interrupt)(Ureg*, void*);
  long  (*ifstat)(Ether*, void*, long, ulong);
  long  (*ctl)(Ether*, void*, long); /* custom ctl messages */
  void  (*power)(Ether*, int);  /* power on/off */
  void  (*shutdown)(Ether*);  /* shutdown hardware before reboot */
  void  *ctlr;

  Queue*  oq;

  Netif;
};
/*e: struct Ether (kernel) */

extern Block* etheriq(Ether*, Block*, int);
extern void addethercard(char*, int(*)(Ether*));
//extern ulong ethercrc(uchar*, int);
//extern int parseether(uchar*, char*);

/*s: macro NEXT */
#define NEXT(x, l)  (((x)+1)%(l))
/*e: macro NEXT */
/*s: macro PREV */
#define PREV(x, l)  (((x) == 0) ? (l)-1: (x)-1)
/*e: macro PREV */
/*e: kernel/network/386/etherif.h */
