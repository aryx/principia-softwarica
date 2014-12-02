/*s: dat_buses.h */

/*
 *  a parsed plan9.ini line
 */
/*s: constant NISAOPT(x86) */
#define NISAOPT   8
/*e: constant NISAOPT(x86) */

/*s: struct ISAConf(x86) */
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
/*e: struct ISAConf(x86) */
/*e: dat_buses.h */
