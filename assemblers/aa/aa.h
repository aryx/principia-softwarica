/*s: assemblers/aa/aa.h */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

// The content of this file was originally copy pasted in 8a/a.h, 5a/a.h, etc.
// It was almost always the same in all archi so I factorized things in aa.h.
// aa.h  is the generic part, for the specific part do #include XXX/Y.out.h
// in Ya/a.h, e.g.:
//#include "386/8.out.h"
//#include "arm/5.out.h"

#include <common.out.h>

#pragma lib "../aa/aa.a$O"

typedef struct  Sym Sym;
typedef struct  Io Io;
typedef struct  Hist Hist;

/*s: constant MAXALIGN */
#define MAXALIGN 7
/*e: constant MAXALIGN */
/*s: constant NSYMB */
#define NSYMB       500
/*e: constant NSYMB */
/*s: constant BUFSIZ */
#define BUFSIZ      8192
/*e: constant BUFSIZ */
/*s: constant HISTSZ */
#define HISTSZ      20
/*e: constant HISTSZ */
/*s: constant NINCLUDE */
#define NINCLUDE    10
/*e: constant NINCLUDE */
/*s: constant NHUNK */
#define NHUNK       10000
/*e: constant NHUNK */
/*s: constant EOF */
#define EOF     (-1)
/*e: constant EOF */
/*s: constant IGN */
#define IGN     (-2)
/*e: constant IGN */
/*s: function GETC */
/// main -> assemble -> yyparse -> yylex -> <>
#define GETC()      ((--fi.c < 0) ? filbuf() : *fi.p++ & 0xff)
/*e: function GETC */
/*s: constant NHASH */
#define NHASH       503
/*e: constant NHASH */
/*s: constant STRINGSZ */
#define STRINGSZ    200
/*e: constant STRINGSZ */


/*s: struct Sym */
struct  Sym
{
    // Sym is (ab)used to represent many things in the assembler:
    //   actual symbols, labels, but also macros,
    //   tokens for opcodes and registers, etc

    // ---------------------------------------------------------
    // The "key"
    // ---------------------------------------------------------
    // ref_own<string>
    char    *name;

    // ---------------------------------------------------------
    // The generic "value"
    // ---------------------------------------------------------
    long    value; 

    /*s: [[Sym]] identifier fields */
    // index in h when the Sym is a symbol, 0 otherwise
    int symidx;
    /*e: [[Sym]] identifier fields */
    /*s: [[Sym]] macro fields */
    //option<string>  for '#define FOO xxx' expansion
    char*   macro;
    /*e: [[Sym]] macro fields */
    /*s: [[Sym]] token fields */
    //enum<token_code> (e.g. LLAB, LNAME, LVAR, LARITH, etc)
    ushort  type;
    /*e: [[Sym]] token fields */

    // ---------------------------------------------------------
    // Extra
    // ---------------------------------------------------------
    /*s: [[Sym]] extra fields */
    // list<ref<Sym>> (from = hash)
    Sym*    link;
    /*e: [[Sym]] extra fields */
};
/*e: struct Sym */
/*s: constant S */
#define S   ((Sym*)nil)
/*e: constant S */

/*s: struct Fi */
struct Fi
{
    // ref<char> (target = Io.b)
    char*   p;
    // remaining characters in Io.b to read
    int     c;
};
/*e: struct Fi */
extern struct Fi fi;

/*s: struct Io */
struct  Io
{
    // option<fdt> (None = FD_NONE)
    fdt f;
    /*s: [[Io]] buffer fields */
    char    b[BUFSIZ];
    /*x: [[Io]] buffer fields */
    // like Fi, saved pointers in Io.b
    char*   p;
    short   c;
    /*e: [[Io]] buffer fields */
    // Extra
    /*s: [[Io]] extra fields */
    // list<ref_own<Io>> (from = iostack or iofree)
    Io* link;
    /*e: [[Io]] extra fields */
};
/*e: struct Io */
/*s: constant I */
#define I   ((Io*)nil)
/*e: constant I */

/*s: constant FD_NONE */
#define FD_NONE (-1)
/*e: constant FD_NONE */

/*s: struct Htab */
struct Htab
{
    // option<ref<Sym>>>
    Sym*    sym;
    //enum<Sym_kind>
    short   symkind;
};
/*e: struct Htab */
extern struct Htab h[NSYM];

/*s: struct Hist */
struct  Hist
{
    char*   filename; // nil for a ``pop''

    // global line of this Hist
    long    global_line;       
    // 0 for #include, +n for #line, -1 for #pragma lib (ugly)
    long    local_line; 

    // Extra
    /*s: [[Hist]] extra fields */
    // list<ref<Hist>> (from = hist)
    Hist*   link;
    /*e: [[Hist]] extra fields */
};
/*e: struct Hist */
/*s: constant H */
#define H   ((Hist*)nil)
/*e: constant H */

// core algorithm
extern  Sym*    hash[NHASH];
extern  int pass;
extern  Biobuf  obuf;
extern  char*   outfile;
extern  char*   pathname;
extern  long    pc;
extern  int symcounter;
extern  int thechar;
extern  char*   thestring;

// input files (used by Ya/lex.c)
extern  Io* iostack;
extern  Io* iofree;
extern  Io* ionext;

// cpp
extern  char*   include[NINCLUDE];
extern  int ninclude;
extern  char*   Dlist[30];
extern  int nDlist;

// lexer
extern  char    symb[NSYMB];
extern  int peekc;

// debugging support
extern  Hist*   hist;
extern  Hist*   ehist;
extern  long    lineno;

// debugging
extern  bool    debug[256];

// error managments
extern  int nerrors;

// utils (used by mac.c)
extern  char*   hunk;
extern  long    nhunk;
extern  long    thunk;

// compact.c
int systemtype(int);
int pathchar(void);
int mywait(int*);
int mycreat(char*, int);

// utils.c
void* alloc(long n);
void* allocn(void *p, long on, long n);
void  gethunk(void);

// lookup.c
Sym*    slookup(char*);
Sym*    lookup(void);
// this actually must be defined in Ya/lex.c, depends on LNAME
void    syminit(Sym*);

// lexbody.c (used by Ya/lex.c)
int escchar(int);



// for lexbody
void    setinclude(char*);
void    pinit(char*);
void    dodefine(char*);
void    domacro(void);
void    macund(void);
void    macdef(void);
void    macexpand(Sym*, char*);
void    macinc(void);
void    maclin(void);
void    macprag(void);
void    macif(int);
void    macend(void);
int getnsc(void);
void    yyerror(char*, ...);
void    linehist(char*, int);
int filbuf(void);

// for macbody
int getc(void);
void    unget(int);

// for macbody, was in lexbody
void pragpack(void);
void pragfpround(void);
void pragprofile(void);
void pragvararg(void);
void pragincomplete(void);


void    pushio(void);
void    newio(void);
void    newfile(char*, int);

void    errorexit(void);

// from lexbody.c
void ieeedtod(Ieee *ieee, double native);

/*s: enum _anon_ (assemblers/aa/aa.h) */
/*
 *  system-dependent stuff from ../cc/compat.c
 */
enum    /* keep in synch with ../cc/cc.h */
{
    Plan9   = 1<<0,
    Unix    = 1<<1,
};
/*e: enum _anon_ (assemblers/aa/aa.h) */
/*e: assemblers/aa/aa.h */
