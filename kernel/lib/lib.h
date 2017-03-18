/*s: lib.h */
// coupling: libc.h
// This file mostly exports code from lib_core/libc/ (linked with the kernel).
// Those functions are also exported in include/libc.h but they prefered to
// not include it and instead to reexport here a subset specific to the kernel.
// There are also the poolxxx() functions exported in include/pool.h
// (also part of libc) that are used in memory/alloc.c.
// Some functions in libc are also "overriden" (via linker abuse):
//  - print.c overrides _fmtlock(), _fmtunlock() that were in libc/fmt/
//  - devcons.s overrides the sysfatal() that was in libc/9sys/

// pad specific, could be in u.h
/*s: pad basic types */
typedef int bool;
enum _bool {
  false = 0,
  true = 1
};

typedef uchar byte;

typedef ushort bool_ushort; //TODO: delete

typedef int fdt; // file descriptor type

#define OK_0 0
#define OK_1 1
#define ERROR_0 0
#define ERROR_1 1
#define ERROR_NEG1 (-1)
typedef int error0;
typedef int error1;
typedef int errorneg1;
/*e: pad basic types */

//#define nil (void*)0 in u.h
#define nilptr 0

/*s: lib.h forward decl */
typedef struct Qid  Qid;
typedef struct DirEntry  DirEntry;
typedef struct DirEntry  Dir; // for fcall.h
typedef struct Waitmsg  Waitmsg;
typedef struct Fmt  Fmt;
/*e: lib.h forward decl */

/*
 * functions (mostly) linked in from libc.
 */
/*s: function nelem */
#define nelem(x)  (sizeof(x)/sizeof((x)[0]))
/*e: function nelem */
/*s: function offsetof */
//@Scheck: not dead, actually exanded in macros.h
#define offsetof(s, m)  (ulong)(&(((s*)0)->m))
/*e: function offsetof */
/*s: function assert */
#define assert(x) do { if(x) {}else _assert("x");}while(0)
/*e: function assert */

/*
 * mem routines
 */
/*s: lib.h mem functions decl */
extern  void* memccpy(void*, void*, int, ulong);
extern  void* memset(void*, int, ulong);
extern  int memcmp(void*, void*, ulong);
extern  void* memmove(void*, void*, ulong);
extern  void* memchr(void*, int, ulong);
/*e: lib.h mem functions decl */

/*
 * string routines
 */
/*s: lib.h string functions decl */
extern  char* strchr(char*, int);
extern  char* strrchr(char*, int);
extern  int strcmp(char*, char*);
extern  char* strcpy(char*, char*);
extern  char* strecpy(char*, char*, char*);
extern  char* strncpy(char*, char*, long);
extern  int strncmp(char*, char*, long);
extern  long  strlen(char*);
extern  char* strstr(char*, char*);
extern  int atoi(char*);
extern  int fullrune(char*, int);
/*e: lib.h string functions decl */

//redefined in the kernel
extern  int cistrcmp(char*, char*);
extern  int cistrncmp(char*, char*, int);

/*s: enum utf */
enum
{
  UTFmax    = 4,    /* maximum bytes per rune */
  Runeself  = 0x80,   /* rune and UTF sequences are the same (<) */
};
/*e: enum utf */

/*
 * rune routines
 */
/*s: lib.h rune functions decl */
extern  int runetochar(char*, Rune*);
extern  int chartorune(Rune*, char*);
extern  char* utfrune(char*, long);
extern  int utfnlen(char*, long);
/*e: lib.h rune functions decl */

extern  int abs(int);

/*
 * print routines
 */
typedef int (*Fmts)(Fmt*);
/*s: struct Fmt */
struct Fmt{
  byte runes;      /* output buffer is runes or chars? */
  void  *start;     /* of buffer */
  void  *to;      /* current place in the buffer */
  void  *stop;      /* end of the buffer; overwritten if flush fails */
  int (*flush)(Fmt *);  /* called when to == stop */
  void  *farg;      /* to make flush a closure */
  int nfmt;     /* num chars formatted so far */
  va_list args;     /* args passed to dofmt */
  int r;      /* % format Rune */
  int width;
  int prec;
  ulong flags;
};
/*e: struct Fmt */

/*s: lib.h print decl */
// This used to be regular function, but to avoid backward deps in the kernel
// I made it into a pointer function (a bit ugly, and maybe unsafe)
extern  int (*print)(char*, ...);
/*e: lib.h print decl */

/*s: lib.h printf functions decl */
extern  char* seprint(char*, char*, char*, ...);
extern  char* vseprint(char*, char*, char*, va_list);
extern  int snprint(char*, int, char*, ...);
extern  int sprint(char*, char*, ...);
/*e: lib.h printf functions decl */

/*s: lib.h pragmas */
#pragma varargck  argpos  fmtprint  2
#pragma varargck  argpos  print   1
#pragma varargck  argpos  seprint   3
#pragma varargck  argpos  snprint   3
#pragma varargck  argpos  sprint    2

#pragma varargck  type  "lld" vlong
#pragma varargck  type  "llx" vlong
#pragma varargck  type  "lld" uvlong
#pragma varargck  type  "llx" uvlong
#pragma varargck  type  "ld"  long
#pragma varargck  type  "lx"  long
#pragma varargck  type  "ld"  ulong
#pragma varargck  type  "lx"  ulong
#pragma varargck  type  "d" int
#pragma varargck  type  "x" int
#pragma varargck  type  "c" int
#pragma varargck  type  "C" int
#pragma varargck  type  "d" uint
#pragma varargck  type  "x" uint
#pragma varargck  type  "c" uint
#pragma varargck  type  "C" uint
#pragma varargck  type  "s" char*
#pragma varargck  type  "q" char*
#pragma varargck  type  "S" Rune*
#pragma varargck  type  "%" void
#pragma varargck  type  "p" uintptr
#pragma varargck  type  "p" void*
#pragma varargck  flag  ','
/*e: lib.h pragmas */

/*s: lib.h fmt functions decl */
extern  int fmtstrinit(Fmt*);
extern  int fmtinstall(int, int (*)(Fmt*));
extern  void  quotefmtinstall(void);
extern  int fmtprint(Fmt*, char*, ...);
extern  int fmtstrcpy(Fmt*, char*);
extern  char* fmtstrflush(Fmt*);
/*e: lib.h fmt functions decl */

/*
 * one-of-a-kind
 */
/*s: lib.h strto functions decl */
extern  long  strtol(char*, char**, int);
extern  ulong strtoul(char*, char**, int);
extern  vlong strtoll(char*, char**, int);
extern  uvlong  strtoull(char*, char**, int);
/*e: lib.h strto functions decl */

//todo: should return a (void*)
extern  ulong getcallerpc(void*);
extern  char* cleanname(char*);
extern  int getfields(char*, char**, int, int, char*);
extern  int tokenize(char*, char**, int);
extern  void  qsort(void*, long, long, int (*)(void*, void*));

/*s: lib.h exxx decl */
extern  char  etext[];
//@Scheck: Assembly, not dead used by 386/l.s and arm/main.c
extern  char  edata[];
extern  char  end[];
/*e: lib.h exxx decl */


/*
 * Syscall data structures
 */
/*s: enum mount */
enum MountFlags {
    MREPL = 0x0000,  /* mount replaces object */
  
    MBEFORE = 0x0001,  /* mount goes before others in union directory */
    MAFTER = 0x0002,  /* mount goes after others in union directory */
  
    MCREATE = 0x0004,  /* permit creation in mounted directory */
    /*s: [[MountFlags]] cases */
    MCACHE = 0x0010,  /* cache some data */
    /*e: [[MountFlags]] cases */
  
    MORDERMASK =  0x0003,  /* mask for bits defining order of mounting */
    MMASK = 0x0017,  /* all bits on */
};
/*e: enum mount */

/*s: enum open */
enum OpenFlags {
    OREAD = 0, /* open for read */
    OWRITE = 1, /* write */
    ORDWR = 2, /* read and write */
    /*s: [[OpenFlags]] cases */
    OEXEC = 3, /* execute, == read but check execute permission */
    /*x: [[OpenFlags]] cases */
    OCEXEC = 32,  /* or'ed in, close on exec */
    /*x: [[OpenFlags]] cases */
    ORCLOSE = 64,  /* or'ed in, remove on close */
    /*x: [[OpenFlags]] cases */
    OTRUNC = 16,  /* or'ed in (except for exec), truncate file first */
    /*x: [[OpenFlags]] cases */
    OEXCL = 0x1000,  /* or'ed in, exclusive create */
    /*e: [[OpenFlags]] cases */
};
/*e: enum open */

/*s: enum note */
enum NoteHook {
  NCONT = 0, /* continue after note */
  NDFLT = 1, /* terminate after note */
  NSAVE = 2, /* clear note but hold state */
  NRSTR = 3, /* restore saved state */
};
/*e: enum note */

/*s: enum miscsize */
enum Miscsize {  
    /*s: constant ERRMAX */
    ERRMAX = 128, /* max length of error string */
    /*e: constant ERRMAX */
    /*s: constant KNAMELEN */
    KNAMELEN = 28,  /* max length of name held in kernel */
    /*e: constant KNAMELEN */
};
/*e: enum miscsize */



/*s: enum qidtype */
/* bits in Qid.type */
enum Qidtype {
  QTFILE = 0x00,    /* plain file */
  QTDIR = 0x80,    /* type bit for directories */
  /*s: [[Qidtype]] cases */
  QTMOUNT = 0x10,    /* type bit for mounted channel */
  /*x: [[Qidtype]] cases */
  QTAUTH = 0x08,    /* type bit for authentication file */
  QTAPPEND = 0x40,    /* type bit for append only files */
  QTEXCL = 0x20,    /* type bit for exclusive use files */
  /*e: [[Qidtype]] cases */
};
/*e: enum qidtype */

/*s: enum dirmode */
/* bits in DirEntry.mode */
enum Dirmode {
    DMDIR = 0x80000000,  /* mode bit for directories */

    DMREAD = 0x4,   /* mode bit for read permission */
    DMWRITE = 0x2,   /* mode bit for write permission */
    DMEXEC = 0x1,   /* mode bit for execute permission */
    /*s: [[Dirmode]] cases */
    DMMOUNT = 0x10000000,  /* mode bit for mounted channel */
    /*x: [[Dirmode]] cases */
    DMEXCL = 0x20000000,  /* mode bit for exclusive use files */
    /*e: [[Dirmode]] cases */
};
/*e: enum dirmode */

/*s: struct Qid */
struct Qid
{
  // note that this is not a string, but an int! it's kind of an inode
  uvlong  path;
  // for cache invalidation
  ulong vers;
  // enum<Qidtype>
  byte type;
};
/*e: struct Qid */

/*s: struct DirEntry */
struct DirEntry {
  /* system-modified data */
  ushort  type; /* server type */
  uint  dev;  /* server subtype */

  /* file data */
  Qid qid;  /* unique id from server */
  // bitset<enum<dirmode>>
  ulong mode; /* permissions */

  ulong atime;  /* last read time */
  ulong mtime;  /* last write time */

  vlong length; /* file length: see <u.h> */
  char  *name;  /* last element of path */

  char  *uid; /* owner name */
  char  *gid; /* group name */
  char  *muid;  /* last modifier name */
};
/*e: struct DirEntry */


/*s: struct Waitmsg */
struct Waitmsg
{
  int pid;    /* of loved one */ // pid of the child
  /*s: [[Waitmsg]] time field */
  ulong time[3];  /* of loved one and descendants */
  /*e: [[Waitmsg]] time field */
  char  msg[ERRMAX];  /* actually variable-size in user mode */
};
/*e: struct Waitmsg */

/*e: lib.h */
