/*s: dat_buses.h */

/*
 *  a parsed plan9.ini line
 */
/*s: constant NISAOPT */
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
/*e: dat_buses.h */
