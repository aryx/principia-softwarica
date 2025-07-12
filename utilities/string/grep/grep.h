/*s: grep/grep.h */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include    <bio.h>

typedef struct  Re  Re;
typedef struct  Re2 Re2;
typedef struct  State   State;

/*s: struct [[State]](grep) */
struct  State
{
    int count;
    int match;
    // ??
    Re**    re;

    // extra
    State*  linkleft;
    State*  linkright;
    // ??
    State*  next[256];
};
/*e: struct [[State]](grep) */
/*s: struct [[Re2]](grep) */
struct  Re2
{
    // ref_own<Re>
    Re* beg;
    // ref_own<Re>
    Re* end;
};
/*e: struct [[Re2]](grep) */
/*s: struct [[Re]](grep) */
struct  Re
{
    // enum<Re_type>
    uchar   type;
    ushort  gen;
    union
    {
        Re* alt;      /* Talt */
        Re**  cases;  /* case */
        struct        /* class */
        {
            Rune    lo;
            Rune    hi;
        };  
        Rune    val;  /* char */
    };
    // ref_own ?
    Re* next;
};
/*e: struct [[Re]](grep) */

/*s: enum [[Re_type]] */
enum Re_type
{
    Talt        = 1,
    Tbegin,
    Tcase,
    Tclass,
    Tend,
    Tor,
};
/*e: enum [[Re_type]] */

enum
{
    Caselim     = 7,
    Nhunk       = 1<<16,
    Cbegin      = Runemax+1,
    Flshcnt     = (1<<9)-1,
};
/*s: enum [[grep_flags]] */
enum
{
    /* count */
    Cflag       = 1<<0,
    /* do not print file name in output */
    Hflag       = 1<<1,
    /* fold upper-lower */
    Iflag       = 1<<2,
    /* print only name of file if any match */
    Llflag      = 1<<3,
    /* print only name of file if any non match */
    LLflag      = 1<<4,
    /* count only */
    Nflag       = 1<<5,
    /* status only */
    Sflag       = 1<<6,
    /* inverse match */
    Vflag       = 1<<7,
    /* dont buffer output */
    Bflag       = 1<<8
};
/*e: enum [[grep_flags]] */

/*s: union [[U]](grep) */
union U
{
    char    string[16*1024];
    struct
    {
        /*
         * if a line requires multiple reads, we keep shifting
         * buf down into pre and then do another read into
         * buf.  so you'll get the last 16-32k of the matching line.
         * if pre were smaller than buf you'd get a suffix of the
         * line with a hole cut out.
         */
        uchar   pre[16*1024];   /* to save to previous '\n' */
        uchar   buf[16*1024];   /* input buffer */
    };
};
/*e: union [[U]](grep) */
extern union U u;

// defined in globals.c
extern  char    *filename;
extern  char    *pattern;
extern  Biobuf  bout;
extern  char    flags[256];
extern  Re**    follow;
extern  ushort  gen;
extern  char*   input;
extern  long    lineno;
extern  int literal;
extern  int matched;
extern  long    maxfollow;
extern  long    nfollow;
extern  int peekc;
extern  Biobuf* rein;
extern  State*  state0;
extern  Re2 topre;

// forward decls
extern  Re* addcase(Re*);
extern  void    appendnext(Re*, Re*);
extern  void    error(char*);
extern  int fcmp(void*, void*);     /* (Re**, Re**) */
extern  void    fol1(Re*, int);
extern  int getrec(void);
extern  void    increment(State*, int);
extern  State*  initstate_(Re*);
extern  void*   mal(int);
extern  void    patchnext(Re*, Re*);
extern  Re* ral(int);
extern  Re2 re2cat(Re2, Re2);
extern  Re2 re2class(char*);
extern  Re2 re2or(Re2, Re2);
extern  Re2 re2char(int, int);
extern  Re2 re2star(Re2);
extern  State*  sal(int);
extern  int search(char*, int);
extern  void    str2top(char*);
extern  int yyparse(void);
extern  void    reprint(char*, Re*);
extern  void    yyerror(char*, ...);
/*e: grep/grep.h */
