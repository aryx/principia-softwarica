/*s: dat_buses.h */

/*
 *  a parsed plan9.ini line
 */
#define NISAOPT   8

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
/*e: dat_buses.h */
