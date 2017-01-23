/*s: portdat_buses.h */

/*s: constant NISAOPT */
/*
 *  a parsed plan9.ini line
 */
#define NISAOPT   8
/*e: constant NISAOPT */

/*s: struct ISAConf */
struct ISAConf {
  char  *type;
  ulong port;
  int irq;
  ulong dma;
  ulong mem;
  ulong size;
  ulong freq;

  int nopt;
  char  *opt[NISAOPT];
};
/*e: struct ISAConf */

/*
 *  routines to access UART hardware
 */
struct PhysUart
{
  char* name;
  Uart* (*pnp)(void);
  void  (*enable)(Uart*, int);
  void  (*disable)(Uart*);
  void  (*kick)(Uart*);
  void  (*dobreak)(Uart*, int);
  int (*baud)(Uart*, int);
  int (*bits)(Uart*, int);
  int (*stop)(Uart*, int);
  int (*parity)(Uart*, int);
  void  (*modemctl)(Uart*, int);
  void  (*rts)(Uart*, int);
  void  (*dtr)(Uart*, int);
  long  (*status)(Uart*, void*, long, long);
  void  (*fifo)(Uart*, int);
  void  (*power)(Uart*, int);
  int (*getc)(Uart*); /* polling versions, for iprint, rdb */
  void  (*putc)(Uart*, int);
};

/*
 *  software UART
 */
struct Uart
{
  void* regs;     /* hardware stuff */
  void* saveregs;   /* place to put registers on power down */
  char* name;     /* internal name */
  ulong freq;     /* clock frequency */
  int bits;     /* bits per character */
  int stop;     /* stop bits */
  int parity;     /* even, odd or no parity */
  int baud;     /* baud rate */
  PhysUart*phys;
  bool console;    /* used as a serial console */
  int special;    /* internal kernel device */
  Uart* next;     /* list of allocated uarts */

  QLock;
  int type;     /* ?? */
  int dev;
  int opens;

  int enabled;
  Uart  *elist;     /* next enabled interface */

  int perr;     /* parity errors */
  int ferr;     /* framing errors */
  int oerr;     /* rcvr overruns */
  int berr;     /* no input buffers */
  int serr;     /* input queue overflow */

  /* buffers */
  int (*putc)(Queue*, int);
  Queue *iq;
  Queue *oq;

  Lock  rlock;
  uchar istage[STAGESIZE];
  uchar *iw;
  uchar *ir;
  uchar *ie;

  Lock  tlock;      /* transmit */
  uchar ostage[STAGESIZE];
  uchar *op;
  uchar *oe;
  int drain;

  int modem;      /* hardware flow control on */
  int xonoff;     /* software flow control on */
  int blocked;
  int cts, dsr, dcd;  /* keep track of modem status */ 
  int ctsbackoff;
  int hup_dsr, hup_dcd; /* send hangup upstream? */
  int dohup;

  Rendez  r;
};

extern  Uart* consuart;

extern Dev uartdevtab;
extern PhysUart* physuart[];

//void (*lprint)(char *, int);
/*e: portdat_buses.h */
