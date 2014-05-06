/*s: portdat.h */

// could be put in lib.h
/*s: portdat.h macros */
#define MIN(a, b) ((a) < (b)? (a): (b))
#define HOWMANY(x, y) (((x)+((y)-1))/(y))
#define ROUNDUP(x, y) (HOWMANY((x), (y))*(y)) /* ceiling */
#define ROUND(s, sz)  (((s)+(sz-1))&~(sz-1))
// BY2PG is defined in mem.h, which should always be included before "dat.h"!
#define PGROUND(s)  ROUNDUP(s, BY2PG)
/*e: portdat.h macros */

// convenient constants
/*s: enum miscsize_portdat */
enum miscsize_portdat
{
  PRINTSIZE = 256,
  NUMSIZE = 12,   /* size of formatted number */
  MB =    (1024*1024),
  /* READSTR was 1000, which is way too small for usb's ctl file */
  READSTR = 4000,   /* temporary buffer size for device reads */
};
/*e: enum miscsize_portdat */

/*s: portdat.h pragmas */
#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
/*e: portdat.h pragmas */
/*e: portdat.h */
