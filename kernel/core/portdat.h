
// could be put in lib.h
#define MIN(a, b) ((a) < (b)? (a): (b))
//#define MAX(a, b) ((a) > (b)? (a): (b))
#define HOWMANY(x, y) (((x)+((y)-1))/(y))
#define ROUNDUP(x, y) (HOWMANY((x), (y))*(y)) /* ceiling */
//#define ROUNDDN(x, y) (((x)/(y))*(y))   /* floor */
#define ROUND(s, sz)  (((s)+(sz-1))&~(sz-1))
// BY2PG is defined in mem.h, which should always be included before "dat.h"!
#define PGROUND(s)  ROUNDUP(s, BY2PG)

// convenient constants
enum
{
  PRINTSIZE = 256,
  NUMSIZE = 12,   /* size of formatted number */
  MB =    (1024*1024),
  /* READSTR was 1000, which is way too small for usb's ctl file */
  READSTR = 4000,   /* temporary buffer size for device reads */
  //  MAXCRYPT =  127,
};

/*
 * For multi-bit fields use FIELD(v, o, w) where 'v' is the value
 * of the bit-field of width 'w' with LSb at bit offset 'o'.
 */
//#define FIELD(v, o, w)  (((v) & ((1<<(w))-1))<<(o))
//#define FCLR(d, o, w) ((d) & ~(((1<<(w))-1)<<(o)))
//#define FEXT(d, o, w) (((d)>>(o)) & ((1<<(w))-1))
//#define FINS(d, o, w, v) (FCLR((d), (o), (w))|FIELD((v), (o), (w)))
//#define FSET(d, o, w) ((d)|(((1<<(w))-1)<<(o)))
//#define FMASK(o, w) (((1<<(w))-1)<<(o))

//#define MAXBY2PG BY2PG    /* rounding for UTZERO in executables */


//enum
//{
//  NSMAX = 1000,
//  NSLOG = 7,
//  NSCACHE = (1<<NSLOG),
//};
//enum
//{
//  NCMDFIELD = 128
//};
/*
 *  action log
 */
//struct Log {
//  Lock;
//  int opens;
//  char* buf;
//  char  *end;
//  char  *rptr;
//  int len;
//  int nlog;
//  int minread;
//
//  int logmask;  /* mask of things to debug */
//
//  QLock readq;
//  Rendez  readr;
//};
//
//struct Logflag {
//  char* name;
//  int mask;
//};
//

#pragma varargck  type  "I" uchar*
#pragma varargck  type  "V" uchar*
#pragma varargck  type  "E" uchar*
#pragma varargck  type  "M" uchar*
