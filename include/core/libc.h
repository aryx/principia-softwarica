/*s: include/libc.h */
#pragma lib "libc.a"
#pragma src "/sys/src/libc"

// --------------------------------------------
// pad's stuff
// --------------------------------------------
// More types! Types are good!
/*s: type bool */
typedef int bool;
enum _bool {
  false = 0,
  true = 1
};
/*e: type bool */

/*s: type byte */
typedef uchar byte;
/*e: type byte */

typedef uchar bool_byte;

//typedef char* string; // conflict
//typedef char* filename; // conflict in sam with function filename

/*s: constant STDxxx */
#define STDIN 0
#define STDOUT 1
#define STDERR 2
/*e: constant STDxxx */
/*s: type fdt */
typedef int fdt; // file descriptor type
/*e: type fdt */

/*s: constant OKxxx */
#define OK_0 0
#define OK_1 1
/*e: constant OKxxx */
/*s: constant ERRORxxx */
#define ERROR_0 0
#define ERROR_1 1
#define ERROR_NEG1 (-1)
/*e: constant ERRORxxx */
/*s: type errorxxx */
// later: unify all of that to be more consistent!
typedef int error0; // 0 is the error value
typedef int error1; // 1 is the error value
typedef int errorneg1; // -1 is the error value
typedef int errorn; // 1 or more means error
/*e: type errorxxx */

// --------------------------------------------
// end pad's stuff
// --------------------------------------------

/*s: function nelem */
#define nelem(x)    (sizeof(x)/sizeof((x)[0]))
/*e: function nelem */
/*s: function offsetof */
#define offsetof(s, m)  (ulong)(&(((s*)nil)->m))
/*e: function offsetof */

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

/*
 * mem routines
 */
extern  void*   memcpy(void*, void*, ulong);
extern  int     memcmp(void*, void*, ulong);
extern  void*   memset(void*, int, ulong);
extern  void*   memchr(void*, int, ulong);

extern  void*   memmove(void*, void*, ulong);
extern  void*   memccpy(void*, void*, int, ulong);

/*
 * string routines
 */
extern  char*   strcpy(char*, char*);
extern  int     strcmp(char*, char*);
extern  char*   strcat(char*, char*);
extern  char*   strchr(char*, int);
extern  char*   strdup(char*);
extern  long    strlen(char*);
extern  char*   strstr(char*, char*);

extern  char*   strecpy(char*, char*, char*);
extern  char*   strncat(char*, char*, long);
extern  char*   strncpy(char*, char*, long);
extern  int     strncmp(char*, char*, long);

extern  char*   strpbrk(char*, char*);

extern  char*   strrchr(char*, int);
extern  char*   strtok(char*, char*);
extern  long    strspn(char*, char*);
extern  long    strcspn(char*, char*);

extern  int     cistrncmp(char*, char*, int);
extern  int     cistrcmp(char*, char*);
extern  char*   cistrstr(char*, char*);

extern  int     tokenize(char*, char**, int);

enum
{
    /*s: constant UTFmax */
    UTFmax      = 4,        /* maximum bytes per rune */
    /*e: constant UTFmax */
    /*s: constant Runesync */
    Runesync    = 0x80,     /* cannot represent part of a UTF sequence (<) */
    /*e: constant Runesync */
    /*s: constant Runeself */
    Runeself    = 0x80,     /* rune and UTF sequences are the same (<) */
    /*e: constant Runeself */
    /*s: constant Runeerror */
    Runeerror   = 0xFFFD,   /* decoding error in UTF */
    /*e: constant Runeerror */
    /*s: constant Runemax */
    Runemax     = 0x10FFFF, /* 21-bit rune */
    /*e: constant Runemax */
    /*s: constant Runemask */
    Runemask    = 0x1FFFFF, /* bits used by runes (see grep) */
    /*e: constant Runemask */
};

/*
 * rune routines
 */
extern  int runetochar(char*, Rune*);
extern  int chartorune(Rune*, char*);
extern  int runelen(long);

extern  int runenlen(Rune*, int);
extern  int fullrune(char*, int);
extern  int utflen(char*);
extern  int utfnlen(char*, long);
extern  char*   utfrune(char*, long);
extern  char*   utfrrune(char*, long);
extern  char*   utfutf(char*, char*);
extern  char*   utfecpy(char*, char*, char*);

extern  Rune*   runestrcat(Rune*, Rune*);
extern  Rune*   runestrchr(Rune*, Rune);
extern  int     runestrcmp(Rune*, Rune*);
extern  Rune*   runestrcpy(Rune*, Rune*);
extern  Rune*   runestrncpy(Rune*, Rune*, long);
extern  Rune*   runestrecpy(Rune*, Rune*, Rune*);
extern  Rune*   runestrdup(Rune*);
extern  Rune*   runestrncat(Rune*, Rune*, long);
extern  int     runestrncmp(Rune*, Rune*, long);
extern  Rune*   runestrrchr(Rune*, Rune);
extern  long    runestrlen(Rune*);
extern  Rune*   runestrstr(Rune*, Rune*);

extern  Rune    tolowerrune(Rune);
extern  Rune    totitlerune(Rune);
extern  Rune    toupperrune(Rune);
extern  Rune    tobaserune(Rune);
extern  int isalpharune(Rune);
extern  int isbaserune(Rune);
extern  int isdigitrune(Rune);
extern  int islowerrune(Rune);
extern  int isspacerune(Rune);
extern  int istitlerune(Rune);
extern  int isupperrune(Rune);

/*
 * malloc
 */
extern  void*   malloc(ulong);
extern  void*   mallocz(ulong, bool);
extern  void    free(void*);

extern  ulong   msize(void*);
extern  void*   mallocalign(ulong, ulong, long, ulong);
extern  void*   calloc(ulong, ulong);
extern  void*   realloc(void*, ulong);

extern  void    setmalloctag(void*, ulong);
extern  void    setrealloctag(void*, ulong);
extern  ulong   getmalloctag(void*);
extern  ulong   getrealloctag(void*);
extern  void*   malloctopoolblock(void*);

/*
 * print routines
 */
/*s: type Fmt */
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
/*e: type Fmt */

/*s: type Fmt_flag */
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
/*e: type Fmt_flag */

// pad: used to be just print()? but for cg transformed in a pointer func
extern  int     (*print)(char*, ...);
extern  char*   seprint(char*, char*, char*, ...);
extern  int     snprint(char*, int, char*, ...);
extern  char*   smprint(char*, ...);
extern  int     sprint(char*, char*, ...);
extern  int     fprint(int, char*, ...);

extern  char*   vseprint(char*, char*, char*, va_list);
extern  int     vsnprint(char*, int, char*, va_list);
extern  char*   vsmprint(char*, va_list);
extern  int     vfprint(int, char*, va_list);

extern  int     runesprint(Rune*, char*, ...);
extern  int     runesnprint(Rune*, int, char*, ...);
extern  Rune*   runeseprint(Rune*, Rune*, char*, ...);
extern  Rune*   runesmprint(char*, ...);

extern  int     runevsnprint(Rune*, int, char*, va_list);
extern  Rune*   runevseprint(Rune*, Rune*, char*, va_list);
extern  Rune*   runevsmprint(char*, va_list);

extern  int     fmtfdinit(Fmt*, int, char*, int);
extern  int     fmtfdflush(Fmt*);
extern  int     fmtstrinit(Fmt*);
extern  char*   fmtstrflush(Fmt*);
extern  int     runefmtstrinit(Fmt*);
extern  Rune*   runefmtstrflush(Fmt*);

#pragma varargck    argpos  fmtprint    2
#pragma varargck    argpos  fprint      2
#pragma varargck    argpos  print       1
#pragma varargck    argpos  runeseprint 3
#pragma varargck    argpos  runesmprint 1
#pragma varargck    argpos  runesnprint 3
#pragma varargck    argpos  runesprint  2
#pragma varargck    argpos  seprint     3
#pragma varargck    argpos  smprint     1
#pragma varargck    argpos  snprint     3
#pragma varargck    argpos  sprint      2

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


extern  int fmtinstall(int, int (*)(Fmt*));
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

/*
 * random number
 */
extern  void    srand(long);
extern  int     rand(void);
extern  int     nrand(int);
extern  long    lrand(void);
extern  long    lnrand(long);
extern  double  frand(void);
extern  ulong   truerand(void);         /* uses /dev/random */
extern  ulong   ntruerand(ulong);       /* uses /dev/random */

/*
 * math
 */
extern  int     abs(int);
//extern  long    labs(long);
extern  double  frexp(double, int*);
extern  double  ldexp(double, int);
extern  double  modf(double, double*);
extern  double  pow10(int);


extern  double  NaN(void);
extern  double  Inf(int);
extern  int     isNaN(double);
extern  int     isInf(double, int);

extern  double  fabs(double);
extern  double  floor(double);
extern  double  ceil(double);

extern  double  pow(double, double);
extern  double  log(double);
extern  double  log10(double);
extern  double  exp(double);
extern  double  sqrt(double);
extern  double  hypot(double, double);
extern  double  fmod(double, double);

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

extern  ulong   getfcr(void);
extern  void    setfsr(ulong);
extern  ulong   getfsr(void);
extern  void    setfcr(ulong);

extern  ulong   umuldiv(ulong, ulong, ulong);
extern  long    muldiv(long, long, long);


#define HUGE    3.4028234e38
#define PIO2    1.570796326794896619231e0
#define PI  (PIO2+PIO2)

/*
 * Time-of-day
 */
/*s: type Tm */
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
/*e: type Tm */

extern  Tm*     gmtime(long);
extern  Tm*     localtime(long);
extern  char*   asctime(Tm*);
extern  char*   ctime(long);
extern  double  cputime(void);
extern  long    times(long*);
extern  long    tm2sec(Tm*);
extern  vlong   nsec(void);

extern  void    cycles(uvlong*);    /* 64-bit value of the cycle counter if there is one, 0 if there isn't */

extern  long    time(long*);


/*
 * one-of-a-kind
 */
enum
{
    PNPROC      = 1,
    PNGROUP     = 2,
};

// debugging tools
/*s: macro assert */
#define assert(x)   do{ if(x) {} else _assert("x"); }while(0)
/*e: macro assert */
extern  void    (*_assert)(char*);
extern  void    perror(char*);
extern  void    sysfatal(char*, ...);
extern  void    syslog(int, char*, char*, ...);

extern  uintptr getcallerpc(void*);

#pragma varargck    argpos  sysfatal    1
#pragma varargck    argpos  syslog  3

// concurrency
extern  int     setjmp(jmp_buf);
extern  void    longjmp(jmp_buf, int);
extern  void    notejmp(void*, jmp_buf, int);

// IPC
extern  int     postnote(int, int, char *);
extern  int     atexit(void(*)(void));
extern  void    atexitdont(void(*)(void));
extern  int     atnotify(int(*)(void*, char*), int);

// conversion
extern  double  atof(char*);
extern  int     atoi(char*);
extern  long    atol(char*);
extern  vlong   atoll(char*);

extern  double  strtod(char*, char**);
extern  long    strtol(char*, char**, int);
extern  ulong   strtoul(char*, char**, int);
extern  vlong   strtoll(char*, char**, int);
extern  uvlong  strtoull(char*, char**, int);

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


extern  int tolower(int);
extern  int toupper(int);

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


/*
 *  profiling
 */
/*s: type Prof */
enum Profiling {
    Profoff,        /* No profiling */

    Profuser,       /* Measure user time only (default) */
    Profkernel,     /* Measure user + kernel time */
    Proftime,       /* Measure total time */
    Profsample,     /* Use clock interrupt to sample (default when there is no cycle counter) */
}; /* what */
/*e: type Prof */

extern  void    prof(void (*fn)(void*), void *arg, int entries, int what);

/*
 * atomic
 */
extern long    ainc(long*);
extern long    adec(long*);
extern int     cas32(u32int*, u32int, u32int);
extern int     casp(void**, void*, void*);
extern int     casl(ulong*, ulong, ulong);

/*
 *  synchronization
 */
/*s: type Lock */
struct Lock {
    long    key;
    long    sem;
};
/*e: type Lock */

extern int  _tas(int*);

extern  void    lock(Lock*);
extern  void    unlock(Lock*);
extern  int     canlock(Lock*);

/*s: type QLp */
struct QLp {
    int inuse;
    QLp *next;
    char    state;
};
/*e: type QLp */

/*s: type QLock */
struct QLock {
    Lock    lock;
    int locked;
    QLp *head;
    QLp     *tail;
};
/*e: type QLock */


extern  void    qlock(QLock*);
extern  void    qunlock(QLock*);
extern  int     canqlock(QLock*);
extern  void    _qlockinit(void* (*)(void*, void*));    /* called only by the thread library */

/*s: type RWLock */
struct RWLock {
    Lock    lock;
    int readers;    /* number of readers */
    int writer;     /* number of writers */
    QLp *head;      /* list of waiting processes */
    QLp *tail;
};
/*e: type RWLock */


extern  void    rlock(RWLock*);
extern  void    runlock(RWLock*);
extern  int     canrlock(RWLock*);
extern  void    wlock(RWLock*);
extern  void    wunlock(RWLock*);
extern  int     canwlock(RWLock*);

/*s: type Rendez */
struct Rendez {
    QLock   *l;
    QLp *head;
    QLp *tail;
};
/*e: type Rendez */

extern  void    rsleep(Rendez*);    /* unlocks r->l, sleeps, locks r->l again */
extern  int     rwakeup(Rendez*);
extern  int     rwakeupall(Rendez*);
extern  void**  privalloc(void);
extern  void    privfree(void**);

/*
 *  network dialing
 */
#define NETPATHLEN 40
extern  int     accept(int, char*);
extern  int     announce(char*, char*);
extern  int     dial(char*, char*, char*, int*);
extern  void    setnetmtpt(char*, int, char*);
extern  int     hangup(int);
extern  int     listen(char*, char*);
extern  char*   netmkaddr(char*, char*, char*);
extern  int     reject(int, char*, char*);

/*
 *  encryption
 */
extern  int pushssl(int, char*, char*, char*, int*);
extern  int pushtls(int, char*, char*, int, char*, char*);

/*
 *  network services
 */
/*s: type NetConnInfo */
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
/*e: type NetConnInfo */
extern  NetConnInfo*    getnetconninfo(char*, int);
extern  void            freenetconninfo(NetConnInfo*);

/*
 * system calls
 *
 */
#include <syscall.h>



// getopt like macros
/*s: signature global argv0 */
extern char *argv0;
/*e: signature global argv0 */
/*s: macro ARGBEGIN */
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
/*e: macro ARGBEGIN */
/*s: macro ARGEND */
#define ARGEND      SET(_argt);USED(_argt,_argc,_args);}USED(argv, argc);
/*e: macro ARGEND */
/*s: macro ARGF */
#define ARGF()      (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): 0))
/*e: macro ARGF */
/*s: macro EARGF */
#define EARGF(x)    (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))
/*e: macro EARGF */
/*s: macro ARGC */
#define ARGC()      _argc
/*e: macro ARGC */


/* this is used by sbrk and brk,  it's a really bad idea to redefine it */
extern  char    end[];

/*e: include/libc.h */
