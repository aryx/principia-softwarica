/*s: grep/grep.h */
#include    <u.h>
#include    <libc.h>
#include    <bio.h>

typedef struct  Re  Re;
typedef struct  Re2 Re2;
typedef struct  State   State;

struct  State
{
    int count;
    int match;
    Re**    re;
    State*  linkleft;
    State*  linkright;
    State*  next[256];
};
struct  Re2
{
    Re* beg;
    Re* end;
};
struct  Re
{
    uchar   type;
    ushort  gen;
    union
    {
        Re* alt;    /* Talt */
        Re**    cases;  /* case */
        struct      /* class */
        {
            Rune    lo;
            Rune    hi;
        };  
        Rune    val;    /* char */
    };
    Re* next;
};

enum
{
    Talt        = 1,
    Tbegin,
    Tcase,
    Tclass,
    Tend,
    Tor,

    Caselim     = 7,
    Nhunk       = 1<<16,
    Cbegin      = Runemax+1,
    Flshcnt     = (1<<9)-1,

    Cflag       = 1<<0,
    Hflag       = 1<<1,
    Iflag       = 1<<2,
    Llflag      = 1<<3,
    LLflag      = 1<<4,
    Nflag       = 1<<5,
    Sflag       = 1<<6,
    Vflag       = 1<<7,
    Bflag       = 1<<8
};

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
extern union U u;

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

extern  Re* addcase(Re*);
extern  void    appendnext(Re*, Re*);
extern  void    error(char*);
extern  int fcmp(void*, void*);     /* (Re**, Re**) */
extern  void    fol1(Re*, int);
extern  int getrec(void);
extern  void    increment(State*, int);
extern  State*  initstate(Re*);
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
