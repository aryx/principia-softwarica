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

  eaddr ea;

  /*s: [[Ether]] methods */
  /*s: [[ether]] mounting methods */
  void  (*attach)(Ether*);  /* filled in by reset routine */
  void  (*detach)(Ether*);
  /*e: [[ether]] mounting methods */
  /*s: [[ether]] io methods */
  void  (*transmit)(Ether*);
  void  (*interrupt)(Ureg*, void*);
  /*e: [[ether]] io methods */
  /*s: [[ether]] other methods */
  long  (*ifstat)(Ether*, void*, long, ulong);
  long  (*ctl)(Ether*, void*, long); /* custom ctl messages */
  void  (*power)(Ether*, int);  /* power on/off */
  void  (*shutdown)(Ether*);  /* shutdown hardware before reboot */
  /*e: [[ether]] other methods */
  /*e: [[Ether]] methods */

  /*s: [[Ether]] priv fields */
  void  *ctlr;
  /*e: [[Ether]] priv fields */
  /*s: [[Ether]] other fields */
  int tbdf;     /* type+busno+devno+funcno */
  /*e: [[Ether]] other fields */

  Queue*  oq;

  ISAConf;      /* hardware info */
  Netif;

  // Extra
  /*s: [[Ether]] extra fields */
  int ctlrno;
  /*e: [[Ether]] extra fields */
};
/*e: struct Ether (kernel) */

extern Block* etheriq(Ether*, Block*, int);
extern void addethercard(char*, int(*)(Ether*));
extern ulong ethercrc(uchar*, int);
//extern int parseether(uchar*, char*);

/*s: macro NEXT */
#define NEXT(x, l)  (((x)+1)%(l))
/*e: macro NEXT */
/*s: macro PREV */
#define PREV(x, l)  (((x) == 0) ? (l)-1: (x)-1)
/*e: macro PREV */
/*e: kernel/network/386/etherif.h */
