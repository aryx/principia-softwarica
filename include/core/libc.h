/*s: include/core/libc.h */
#pragma lib "libc.a"
#pragma src "/sys/src/libc"

typedef struct Fmt Fmt;
typedef struct Tm Tm;
typedef struct Lock Lock;
typedef struct QLp QLp;
typedef struct QLock QLock;
typedef struct RWLock RWLock;
typedef struct Rendez Rendez;
typedef struct NetConnInfo NetConnInfo;
typedef struct Qid Qid;
typedef struct Dir Dir;
typedef struct Waitmsg Waitmsg;
typedef struct IOchunk IOchunk;

//******************************************************************************
// Foundations
//******************************************************************************

//----------------------------------------------------------------------------
// Pad's core types
//----------------------------------------------------------------------------

// More types! Types are good!
/*s: type [[bool]] */
typedef int bool;
enum _bool {
  false = 0,
  true = 1
};
/*e: type [[bool]] */
/*s: type [[byte]] */
typedef uchar byte;
/*e: type [[byte]] */
typedef uchar bool_byte;

//typedef char* string; // conflict
//typedef char* filename; // conflict in sam with function filename

enum _ord {
  ORD__EQ = 0,
  ORD__INF = -1,
  ORD__SUP = 1,
};
typedef int ord;

/*s: constant [[STDxxx]] */
#define STDIN 0
#define STDOUT 1
#define STDERR 2
/*e: constant [[STDxxx]] */
/*s: type [[fdt]] */
typedef int fdt; // file descriptor type
/*e: type [[fdt]] */

/*s: constant [[OKxxx]] */
#define OK_0 0
#define OK_1 1
/*e: constant [[OKxxx]] */
/*s: constant [[ERRORxxx]] */
#define ERROR_0 0
#define ERROR_1 1
#define ERROR_NEG1 (-1)
/*e: constant [[ERRORxxx]] */
/*s: type [[errorxxx]] */
// later: unify all of that to be more consistent!
typedef int error0; // 0 is the error value
typedef int error1; // 1 is the error value
typedef int errorneg1; // -1 is the error value
typedef int errorn; // 1 or more means error
/*e: type [[errorxxx]] */

//----------------------------------------------------------------------------
// Macros (Array/Struct)
//----------------------------------------------------------------------------

/*s: function [[nelem]] */
#define nelem(x)    (sizeof(x)/sizeof((x)[0]))
/*e: function [[nelem]] */
/*s: function [[offsetof]] */
#define offsetof(s, m)  (ulong)(&(((s*)nil)->m))
/*e: function [[offsetof]] */

//----------------------------------------------------------------------------
// Exception
//----------------------------------------------------------------------------

extern  int     setjmp(jmp_buf);
extern  void    longjmp(jmp_buf, int);

// ??
extern  void    notejmp(void*, jmp_buf, int);


//----------------------------------------------------------------------------
// Malloc/free
//----------------------------------------------------------------------------

/*
 * malloc
 */
/*s: signatures memory management functions */
extern  void*   malloc(ulong);
extern  void    free(void*);
extern  void*   realloc(void*, ulong);

extern  void*   mallocz(ulong, bool);
extern  ulong   msize(void*);
/*e: signatures memory management functions */
// less useful
extern  void*   calloc(ulong, ulong);
extern  void*   mallocalign(ulong, ulong, long, ulong);

// internals
extern  void    setmalloctag(void*, ulong);
extern  void    setrealloctag(void*, ulong);
extern  ulong   getmalloctag(void*);
extern  ulong   getrealloctag(void*);

extern  void*   malloctopoolblock(void*);

//******************************************************************************
// Data processing
//******************************************************************************

//----------------------------------------------------------------------------
// Mem ops
//----------------------------------------------------------------------------

/*
 * mem routines
 */
/*s: signatures of [[memxxx]] functions */
extern  void*   memset(void*, int, ulong);
extern  void*   memcpy(void*, void*, ulong);
extern  void*   memmove(void*, void*, ulong);
extern  int     memcmp(void*, void*, ulong);
extern  void*   memchr(void*, int, ulong);
/*e: signatures of [[memxxx]] functions */
// less useful?
extern  void*   memccpy(void*, void*, int, ulong);

//----------------------------------------------------------------------------
// Str ops
//----------------------------------------------------------------------------

/*
 * string routines
 */
// memxxx equivalent, but with special handling for '\0' (no need pass ulong)
/*s: signatures of [[strxxx]] functions */
extern  long    strlen(char*);
extern  int     strcmp(char*, char*);
extern  char*   strcpy(char*, char*);
extern  char*   strchr(char*, int);
extern  char*   strdup(char*);
extern  char*   strstr(char*, char*);
extern  char*   strcat(char*, char*);
/*e: signatures of [[strxxx]] functions */

extern  int tolower(int);
extern  int toupper(int);

// less useful?
extern  char*   strecpy(char*, char*, char*);
extern  char*   strncat(char*, char*, long);
extern  char*   strncpy(char*, char*, long);
extern  int     strncmp(char*, char*, long);
extern  char*   strrchr(char*, int);

extern  char*   strpbrk(char*, char*);

extern  long    strspn(char*, char*);
extern  long    strcspn(char*, char*);

extern  int     cistrncmp(char*, char*, int);
extern  int     cistrcmp(char*, char*);
extern  char*   cistrstr(char*, char*);

extern  char*   strtok(char*, char*);
extern  int     tokenize(char*, char**, int);

//----------------------------------------------------------------------------
// Runes
//----------------------------------------------------------------------------

enum
{
    /*s: constant [[UTFmax]] */
    UTFmax      = 4,        /* maximum bytes per rune */
    /*e: constant [[UTFmax]] */
    /*s: constant [[Runesync]] */
    Runesync    = 0x80,     /* cannot represent part of a UTF sequence (<) */
    /*e: constant [[Runesync]] */
    /*s: constant [[Runeself]] */
    Runeself    = 0x80,     /* rune and UTF sequences are the same (<) */
    /*e: constant [[Runeself]] */
    /*s: constant [[Runeerror]] */
    Runeerror   = 0xFFFD,   /* decoding error in UTF */
    /*e: constant [[Runeerror]] */
    /*s: constant [[Runemax]] */
    Runemax     = 0x10FFFF, /* 21-bit rune */
    /*e: constant [[Runemax]] */
    /*s: constant [[Runemask]] */
    Runemask    = 0x1FFFFF, /* bits used by runes (see grep) */
    /*e: constant [[Runemask]] */
};

/*
 * rune routines
 */

// strxxx equivalent for rune
/*s: signatures of [[runexxx]] functions */
extern  long    runestrlen(Rune*);
extern  int     runestrcmp(Rune*, Rune*);
extern  Rune*   runestrcpy(Rune*, Rune*);
extern  Rune*   runestrchr(Rune*, Rune);
extern  Rune*   runestrdup(Rune*);
extern  Rune*   runestrstr(Rune*, Rune*);
extern  Rune*   runestrcat(Rune*, Rune*);
/*e: signatures of [[runexxx]] functions */

// less useful
extern  Rune*   runestrrchr(Rune*, Rune);
extern  Rune*   runestrecpy(Rune*, Rune*, Rune*);
extern  Rune*   runestrncpy(Rune*, Rune*, long);
extern  Rune*   runestrncat(Rune*, Rune*, long);
extern  int     runestrncmp(Rune*, Rune*, long);

extern  Rune    tolowerrune(Rune);
extern  Rune    toupperrune(Rune);

extern  int isalpharune(Rune);
extern  int isdigitrune(Rune);
extern  int islowerrune(Rune);
extern  int isupperrune(Rune);
extern  int isspacerune(Rune);

// ??
extern  Rune    totitlerune(Rune);
extern  Rune    tobaserune(Rune);
extern  int istitlerune(Rune);
extern  int isbaserune(Rune);

// ?????
extern  int runelen(long);
extern  int runenlen(Rune*, int);
extern  int fullrune(char*, int);

//----------------------------------------------------------------------------
// UTF8
//----------------------------------------------------------------------------

// char <-> rune conversion
/*s: signatures rune conversion functions */
extern  int chartorune(Rune*, char*);
extern  int runetochar(char*, Rune*);
/*e: signatures rune conversion functions */

// strxxx equivalent for utf
/*s: signatures [[utfxxx]] functions */
extern  int     utflen(char*);
extern  char*   utfrune(char*, long);
extern  char*   utfrrune(char*, long);
extern  char*   utfutf(char*, char*);
/*e: signatures [[utfxxx]] functions */

// less useful?
extern  int utfnlen(char*, long);
extern  char*   utfecpy(char*, char*, char*);


//----------------------------------------------------------------------------
// Print
//----------------------------------------------------------------------------

/*
 * print routines
 */
/*s: type [[Fmt]] */
struct Fmt {
    uchar   runes;          /* output buffer is runes or chars? */

    void    *start;         /* of buffer */
    void    *to;            /* current place in the buffer */
    void    *stop;          /* end of the buffer; overwritten if flush fails */

    int     (*flush)(Fmt *);    /* called when to == stop */
    void    *farg;          /* to make flush a closure */
    int     nfmt;           /* num chars formatted so far */

    va_list args;           /* args passed to dofmt */

    int     r;          /* % format Rune */
    int     width;
    int     prec;
    ulong   flags;
};
/*e: type [[Fmt]] */

/*s: type [[Fmt_flag]] */
enum Fmt_flag {
    FmtWidth    = 1,
    FmtLeft     = FmtWidth << 1,
    FmtPrec     = FmtLeft << 1,
    FmtSharp    = FmtPrec << 1,
    FmtSpace    = FmtSharp << 1,
    FmtSign     = FmtSpace << 1,
    FmtZero     = FmtSign << 1,
    FmtUnsigned = FmtZero << 1,
    FmtShort    = FmtUnsigned << 1,
    FmtLong     = FmtShort << 1,
    FmtVLong    = FmtLong << 1,
    FmtComma    = FmtVLong << 1,
    FmtByte     = FmtComma << 1,

    FmtFlag     = FmtByte << 1
};
/*e: type [[Fmt_flag]] */

// pad: used to be just print() but for cg transformed in a pointer func
/*s: signatures [[xxxprint]] functions */
extern  int     (*print)(char*, ...);
extern  int     fprint(fdt, char*, ...);

#pragma varargck    argpos  print       1
#pragma varargck    argpos  fprint      2

extern  int     sprint(char*, char*, ...);
extern  char*   seprint(char*, char*, char*, ...);
extern  int     snprint(char*, int, char*, ...);

#pragma varargck    argpos  sprint      2
#pragma varargck    argpos  seprint     3
#pragma varargck    argpos  snprint     3

extern  int fmtinstall(int, int (*)(Fmt*));
/*e: signatures [[xxxprint]] functions */

extern  int     vfprint(fdt, char*, va_list);
extern  char*   vseprint(char*, char*, char*, va_list);

extern  int     runesprint(Rune*, char*, ...);

// less useful
extern  char*   smprint(char*, ...);

extern  int     vsnprint(char*, int, char*, va_list);
extern  char*   vsmprint(char*, va_list);

extern  Rune*   runeseprint(Rune*, Rune*, char*, ...);
extern  int     runesnprint(Rune*, int, char*, ...);
extern  Rune*   runesmprint(char*, ...);

extern  Rune*   runevseprint(Rune*, Rune*, char*, va_list);
extern  int     runevsnprint(Rune*, int, char*, va_list);
extern  Rune*   runevsmprint(char*, va_list);

extern  int     fmtfdinit(Fmt*, int, char*, int);

extern  int     fmtfdflush(Fmt*);
extern  int     fmtstrinit(Fmt*);
extern  char*   fmtstrflush(Fmt*);
extern  int     runefmtstrinit(Fmt*);
extern  Rune*   runefmtstrflush(Fmt*);

#pragma varargck    argpos  fmtprint    2
#pragma varargck    argpos  runeseprint 3
#pragma varargck    argpos  runesmprint 1
#pragma varargck    argpos  runesnprint 3
#pragma varargck    argpos  runesprint  2
#pragma varargck    argpos  smprint     1

// %d = decimal, o = octal, x = hexa, b = binary?
#pragma varargck    type    "d" int
#pragma varargck    type    "o" int
#pragma varargck    type    "x" int
#pragma varargck    type    "b" int
// %f
#pragma varargck    type    "f" double
#pragma varargck    type    "e" double
#pragma varargck    type    "g" double
// %s
#pragma varargck    type    "s" char*
#pragma varargck    type    "q" char*
#pragma varargck    type    "S" Rune*
#pragma varargck    type    "Q" Rune*
#pragma varargck    type    "r" void
#pragma varargck    type    "%" void
#pragma varargck    type    "n" int*
#pragma varargck    type    "p" uintptr
#pragma varargck    type    "p" void*
#pragma varargck    type    "c" int
#pragma varargck    type    "C" int
#pragma varargck    type    "d" uint
#pragma varargck    type    "x" uint
#pragma varargck    type    "b" uint
#pragma varargck    type    "c" uint
#pragma varargck    type    "C" uint
#pragma varargck    type    "<" void*
#pragma varargck    type    "[" void*
#pragma varargck    type    "H" void*
#pragma varargck    type    "lH"    void*

#pragma varargck    type    "lld"   vlong
#pragma varargck    type    "llo"   vlong
#pragma varargck    type    "llx"   vlong
#pragma varargck    type    "llb"   vlong

#pragma varargck    type    "lld"   uvlong
#pragma varargck    type    "llo"   uvlong
#pragma varargck    type    "llx"   uvlong
#pragma varargck    type    "llb"   uvlong

#pragma varargck    type    "ld"    long
#pragma varargck    type    "lo"    long
#pragma varargck    type    "lx"    long
#pragma varargck    type    "lb"    long

#pragma varargck    type    "ld"    ulong
#pragma varargck    type    "lo"    ulong
#pragma varargck    type    "lx"    ulong
#pragma varargck    type    "lb"    ulong


#pragma varargck    flag    ','
#pragma varargck    flag    ' '
#pragma varargck    flag    'h'



extern  int dofmt(Fmt*, char*);
extern  int dorfmt(Fmt*, Rune*);
extern  int fmtprint(Fmt*, char*, ...);
extern  int fmtvprint(Fmt*, char*, va_list);
extern  int fmtrune(Fmt*, int);
extern  int fmtstrcpy(Fmt*, char*);
extern  int fmtrunestrcpy(Fmt*, Rune*);

/*
 * error string for %r
 * supplied on per os basis, not part of fmt library
 */
extern  int errfmt(Fmt *f);

//----------------------------------------------------------------------------
// Quoted strings
//----------------------------------------------------------------------------

/*
 * quoted strings
 */
extern  char*   unquotestrdup(char*);
extern  Rune*   unquoterunestrdup(Rune*);
extern  char*   quotestrdup(char*);
extern  Rune*   quoterunestrdup(Rune*);
extern  int     quotestrfmt(Fmt*);
extern  int     quoterunestrfmt(Fmt*);

extern  void    quotefmtinstall(void);

extern  int     (*doquote)(int);
extern  int     needsrcquote(int);

//******************************************************************************
// Mathematics
//******************************************************************************

//----------------------------------------------------------------------------
// Random
//----------------------------------------------------------------------------

/*
 * random number
 */
/*s: signatures [[xxxrand]] functions */
extern  int     rand(void);
extern  int     nrand(int);

extern  void    srand(long);
/*e: signatures [[xxxrand]] functions */

// less useful
extern  double  frand(void);
extern  ulong   truerand(void);         /* uses /dev/random */
extern  long    lrand(void);
extern  long    lnrand(long);
extern  ulong   ntruerand(ulong);       /* uses /dev/random */

//----------------------------------------------------------------------------
// Basic math
//----------------------------------------------------------------------------

/*
 * math
 */
/*s: signatures basic and rounding functions */
extern  int     abs(int);
extern  double  fabs(double);

extern  double  floor(double);
extern  double  ceil(double);
extern  double  fmod(double, double);
/*e: signatures basic and rounding functions */
//extern  long    labs(long);

extern  double  frexp(double, int*);
extern  double  ldexp(double, int);
extern  double  modf(double, double*);

#define HUGE    3.4028234e38
/*s: signatures special float functions */
extern  double  NaN(void);
extern  double  Inf(int);

extern  int     isNaN(double);
extern  int     isInf(double, int);
/*e: signatures special float functions */

/*s: signatures exponential and powers functions */
extern  double  exp(double);
extern  double  log(double);
extern  double  log10(double);
extern  double  pow(double, double);
extern  double  pow10(int);
extern  double  sqrt(double);
/*e: signatures exponential and powers functions */

extern  double  hypot(double, double);

//----------------------------------------------------------------------------
// Trigonometry
//----------------------------------------------------------------------------

#define PIO2    1.570796326794896619231e0
#define PI  (PIO2+PIO2)

/*s: signatures trigonometric functions */
extern  double  sin(double);
extern  double  cos(double);
extern  double  tan(double);

extern  double  asin(double);
extern  double  acos(double);
extern  double  atan(double);
extern  double  atan2(double, double);

extern  double  sinh(double);
extern  double  cosh(double);
extern  double  tanh(double);
/*e: signatures trigonometric functions */

//----------------------------------------------------------------------------
// Conversion
//----------------------------------------------------------------------------

/*s: signatures number parsing functions */
extern  int     atoi(char*);
extern  double  atof(char*);
extern  long    atol(char*);

extern  double  strtod(char*, char**);
extern  long    strtol(char*, char**, int);
/*e: signatures number parsing functions */

extern  vlong   atoll(char*);

extern  ulong   strtoul(char*, char**, int);
extern  vlong   strtoll(char*, char**, int);
extern  uvlong  strtoull(char*, char**, int);

//----------------------------------------------------------------------------
// Misc
//----------------------------------------------------------------------------

// internals?
extern  ulong   getfcr(void);
extern  void    setfsr(ulong);
extern  ulong   getfsr(void);
extern  void    setfcr(ulong);

extern  ulong   umuldiv(ulong, ulong, ulong);
extern  long    muldiv(long, long, long);

//******************************************************************************
// Misc
//******************************************************************************

//----------------------------------------------------------------------------
// Time
//----------------------------------------------------------------------------

/*
 * Time-of-day
 */
/*s: type [[Tm]] */
struct Tm {
    int sec;
    int min;
    int hour;

    int mday;
    int mon;
    int year;
    int wday;
    int yday;

    char    zone[4];
    int     tzoff;
};
/*e: type [[Tm]] */

/*s: signatures time functions */
extern  long    time(long*);
extern  vlong   nsec(void);

extern  Tm*     gmtime(long);
extern  Tm*     localtime(long);
/*e: signatures time functions */

extern  double  cputime(void);

extern  long    tm2sec(Tm*);

// less useful?
extern  char*   asctime(Tm*);
extern  char*   ctime(long);
extern  long    times(long*);

extern  void    cycles(uvlong*);    /* 64-bit value of the cycle counter if there is one, 0 if there isn't */


//----------------------------------------------------------------------------
// Misc
//----------------------------------------------------------------------------

/*
 * one-of-a-kind
 */

// misc
extern  double  charstod(int(*)(void*), void*);

// modified in place, so type should really be void cleanname(INOUT char*);
extern  char*   cleanname(char*);
extern  int     encodefmt(Fmt*);

extern  int     getfields(char*, char**, int, int, char*);
extern  int     gettokens(char *, char **, int, char *);

extern  int     iounit(fdt);

// ugly redefined by user code? see statusbar.c
extern  void    qsort(void*, long, long, int (*)(void*, void*));

//******************************************************************************
// Debugging and profiling
//******************************************************************************

//----------------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------------

/*
 | debugging tools 
 */

/*s: macro [[assert]] */
#define assert(x)   do{ if(x) {} else _assert("x"); }while(0)
/*e: macro [[assert]] */

extern  void    (*_assert)(char*);

/*s: signatures error and logging functions */
extern  void    perror(char*);
extern  void    sysfatal(char*, ...);
extern  void    syslog(int, char*, char*, ...);

#pragma varargck    argpos  sysfatal    1
#pragma varargck    argpos  syslog  3
/*e: signatures error and logging functions */

// useful for stack trace?
extern  uintptr getcallerpc(void*);

//----------------------------------------------------------------------------
// Profiling
//----------------------------------------------------------------------------

/*
 *  profiling
 */

/*s: type [[Prof]] */
enum Profiling {
    Profoff,        /* No profiling */

    Profuser,       /* Measure user time only (default) */
    Profkernel,     /* Measure user + kernel time */
    Proftime,       /* Measure total time */
    Profsample,     /* Use clock interrupt to sample (default when there is no cycle counter) */
}; /* what */
/*e: type [[Prof]] */

/*s: signatures profiling functions */
extern  void    prof(void (*fn)(void*), void *arg, int entries, int what);
/*e: signatures profiling functions */

//******************************************************************************
// Concurrency and communication
//******************************************************************************

/*s: type [[PostnoteKind]] */
enum
{
    PNPROC      = 1,
    PNGROUP     = 2,
};
/*e: type [[PostnoteKind]] */

//----------------------------------------------------------------------------
// Concurrency
//----------------------------------------------------------------------------

/*
 * atomic
 */
/*s: signatures atomic functions */
extern long    ainc(long*);
extern long    adec(long*);
/*e: signatures atomic functions */

extern int     cas32(u32int*, u32int, u32int);
extern int     casp(void**, void*, void*);
extern int     casl(ulong*, ulong, ulong);

/*
 *  synchronization
 */
/*s: type [[Lock]] */
struct Lock {
    long    key;
    long    sem;
};
/*e: type [[Lock]] */

extern int  _tas(int*);

/*s: signatures [[Lock]] functions */
extern  void    lock(Lock*);
extern  void    unlock(Lock*);
extern  int     canlock(Lock*);
/*e: signatures [[Lock]] functions */

/*s: type [[QLp]] */
struct QLp {
    char    state;
    int inuse;
    // Extra
    QLp *next;
};
/*e: type [[QLp]] */

/*s: type [[QLock]] */
struct QLock {
    Lock    lock;
    int locked;

    QLp *head;
    QLp     *tail;
};
/*e: type [[QLock]] */

/*s: signatures [[QLock]] functions */
extern  void    qlock(QLock*);
extern  void    qunlock(QLock*);
extern  int     canqlock(QLock*);
/*e: signatures [[QLock]] functions */

extern  void    _qlockinit(void* (*)(void*, void*));    /* called only by the thread library */

/*s: type [[RWLock]] */
struct RWLock {
    Lock    lock;
    int readers;    /* number of readers */
    int writer;     /* number of writers */

    QLp *head;      /* list of waiting processes */
    QLp *tail;
};
/*e: type [[RWLock]] */

/*s: signatures [[RWLock]] functions */
extern  void    rlock(RWLock*);
extern  void    runlock(RWLock*);
extern  int     canrlock(RWLock*);
extern  void    wlock(RWLock*);
extern  void    wunlock(RWLock*);
extern  int     canwlock(RWLock*);
/*e: signatures [[RWLock]] functions */

/*s: type [[Rendez]] */
struct Rendez {
    QLock   *l;

    QLp *head;
    QLp *tail;
};
/*e: type [[Rendez]] */

/*s: signatures [[Rendez]] functions */
extern  void    rsleep(Rendez*);    /* unlocks r->l, sleeps, locks r->l again */
extern  int     rwakeup(Rendez*);
/*e: signatures [[Rendez]] functions */

extern  int     rwakeupall(Rendez*);

// per-process private vars
/*s: signatures private vars functions */
extern  void**  privalloc(void);
extern  void    privfree(void**);
/*e: signatures private vars functions */

//----------------------------------------------------------------------------
// Network (/net wrappers)
//----------------------------------------------------------------------------

/*
 *  network dialing
 */
#define NETPATHLEN 40
/*s: signatures networking functions */
extern  int     dial(char*, char*, char*, int*);
extern  int     accept(int, char*);
extern  int     announce(char*, char*);
extern  int     listen(char*, char*);
/*e: signatures networking functions */

extern  void    setnetmtpt(char*, int, char*);
extern  int     hangup(int);
extern  char*   netmkaddr(char*, char*, char*);
extern  int     reject(int, char*, char*);

/*
 *  network services
 */
/*s: type [[NetConnInfo]] */
struct NetConnInfo {
    char    *dir;       /* connection directory */
    char    *root;      /* network root */
    char    *spec;      /* binding spec */
    char    *lsys;      /* local system */
    char    *lserv;     /* local service */
    char    *rsys;      /* remote system */
    char    *rserv;     /* remote service */
    char    *laddr;     /* local address */
    char    *raddr;     /* remote address */
};
/*e: type [[NetConnInfo]] */
extern  NetConnInfo*    getnetconninfo(char*, int);
extern  void            freenetconninfo(NetConnInfo*);


//----------------------------------------------------------------------------
// Crypto
//----------------------------------------------------------------------------

/*
 *  encryption
 */
extern  int pushssl(int, char*, char*, char*, int*);
extern  int pushtls(int, char*, char*, int, char*, char*);

// encryption
extern  int decrypt(void*, void*, int);
extern  int encrypt(void*, void*, int);
extern  int netcrypt(void*, void*);

extern  int dec64(uchar*, int, char*, int);
extern  int enc64(char*, int, uchar*, int);
extern  int dec32(uchar*, int, char*, int);
extern  int enc32(char*, int, uchar*, int);
extern  int dec16(uchar*, int, char*, int);
extern  int enc16(char*, int, uchar*, int);

//******************************************************************************
// Syscalls
//******************************************************************************

/*
 * system calls
 *
 */
#include <syscall.h>


//******************************************************************************
// ARGXXX
//******************************************************************************

// getopt like macros
/*s: signature global argv0 */
extern char *argv0;
/*e: signature global argv0 */
/*s: macro [[ARGBEGIN]] */
#define ARGBEGIN    for((argv0||(argv0=*argv)),argv++,argc--;\
                argv[0] && argv[0][0]=='-' && argv[0][1];\
                argc--, argv++) {\
                char *_args, *_argt;\
                Rune _argc;\
                _args = &argv[0][1];\
                if(_args[0]=='-' && _args[1]==0){\
                    argc--; argv++; break;\
                }\
                _argc = 0;\
                while(*_args && (_args += chartorune(&_argc, _args)))\
                switch(_argc)
/*e: macro [[ARGBEGIN]] */
/*s: macro [[ARGEND]] */
#define ARGEND      SET(_argt);USED(_argt,_argc,_args);}USED(argv, argc);
/*e: macro [[ARGEND]] */
/*s: macro [[ARGF]] */
#define ARGF()      (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): 0))
/*e: macro [[ARGF]] */
/*s: macro [[EARGF]] */
#define EARGF(x)    (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))
/*e: macro [[EARGF]] */
/*s: macro [[ARGC]] */
#define ARGC()      _argc
/*e: macro [[ARGC]] */

/*e: include/core/libc.h */
