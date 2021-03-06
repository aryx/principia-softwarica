/*s: linkers/5l/l.h */
#include    <u.h>
#include    <libc.h>
#include    <bio.h>

#include    <common.out.h>
#include    <5.out.h>

#include    "../8l/elf.h"

//----------------------------------------------------------------------------
// Data structures and constants
//----------------------------------------------------------------------------

// forward decls
typedef struct  Adr     Adr;
typedef struct  Auto    Auto;
typedef struct  Prog    Prog;
typedef struct  Sym     Sym;

/*s: struct [[Adr]](arm) */
struct  Adr
{
    // enum<Operand_kind> (D_NONE by default)
    short   type;

    // switch on Operand.type
    union {
        long    offset;
        Ieee*   ieee;
        char*   sval;
    };

    // option<enum<Register>> None = R_NONE
    short   reg; 

    /*s: [[Adr]] other fields */
    // enum<Sym_kind>
    short   symkind;
    // option<ref<Sym>> (owner = hash)
    Sym*    sym;
    /*x: [[Adr]] other fields */
    // option<enum<Operand_class>>, 0 means None, i means i-1 is the class you want
    short   class;
    /*x: [[Adr]] other fields */
    // list<ref_own<Auto> (next = Auto.link), only used by TEXT instruction
    Auto*   autom;
    /*e: [[Adr]] other fields */
};
/*e: struct [[Adr]](arm) */

/*s: struct [[Prog]](arm) */
struct  Prog
{
    //enum<Opcode>
    byte    as;

    // operands
    Adr from;
    Adr to;

    /*s: [[Prog]] other fields */
    // option<enum<Register>>, None = R_NONE
    short   reg;
    /*x: [[Prog]] other fields */
    // enum<Instr_cond>
    byte    scond;
    /*x: [[Prog]] other fields */
    long    line;
    /*x: [[Prog]] other fields */
    // virtual program counter, and then real program counter
    long    pc;
    /*x: [[Prog]] other fields */
    //bitset<enum<Mark>>
    short   mark;
    /*x: [[Prog]] other fields */
    Prog*   forwd;
    /*x: [[Prog]] other fields */
    // option<index in optab[] (1-based)>, 0 means None, i means optab[i-1]
    byte    optab;
    /*e: [[Prog]] other fields */

    // Extra
    /*s: [[Prog]] extra fields */
    // option<ref<Prog>> for branch instructions
    // (abused to list<ref<Prog>> (from = textp for TEXT instructions))
    // (abused also for instructions using large constants)
    Prog*   cond;
    /*x: [[Prog]] extra fields */
    // list<ref<Prog>> (from = firstp or datap)
    Prog*   link;
    /*e: [[Prog]] extra fields */
};
/*e: struct [[Prog]](arm) */
/*s: constant [[P]] */
#define P       ((Prog*)nil)
/*e: constant [[P]] */

/*s: struct [[Sym]] */
struct  Sym
{
    // The key
    // ref_own<string>
    char    *name;
    // 0 for global symbols, object file id for private symbols
    short   version; 

    // The generic value, 
    // e.g., virtual pc for a TEXT procedure, size for GLOBL
    long    value; 

    /*s: [[Sym]] section field */
    //enum<Section>
    short   type;
    /*e: [[Sym]] section field */

    /*s: [[Sym]] other fields */
    // for instance last 32 bits of md5sum of the type of the symbol
    ulong   sig;
    /*x: [[Sym]] other fields */
    // index in filen[]
    ushort  file;
    /*x: [[Sym]] other fields */
    // enum<Section> too?
    short   subtype;
    /*x: [[Sym]] other fields */
    // x86 only, dead? can remove though?
    short   become;
    /*x: [[Sym]] other fields */
    // x86 only
    short   frame;
    /*e: [[Sym]] other fields */
    // Extra
    /*s: [[Sym]] extra fields */
    // list<ref<Sym>> (from = hash)
    Sym*    link;
    /*e: [[Sym]] extra fields */
};
/*e: struct [[Sym]] */
/*s: constant [[S]] */
#define S       ((Sym*)nil)
/*e: constant [[S]] */

/*s: enum [[Section]](arm) */
enum Section
{
    SNONE = 0,

    STEXT,
    SDATA,
    SBSS,

    SXREF,
    /*s: [[Section]] cases */
    SFILE,
    /*x: [[Section]] cases */
    SIMPORT,
    SEXPORT,
    /*x: [[Section]] cases */
    SUNDEF,
    /*x: [[Section]] cases */
    SDATA1,
    /*x: [[Section]] cases */
    SSTRING, // arm
    /*e: [[Section]] cases */
};
/*e: enum [[Section]](arm) */

/*s: enum [[Mark]](arm) */
/* mark flags */
enum Mark {
    /*s: [[Mark]] cases */
    LEAF        = 1<<2,
    /*x: [[Mark]] cases */
    FOLL        = 1<<0,
    /*e: [[Mark]] cases */
};
/*e: enum [[Mark]](arm) */

/*s: enum [[headtype]](arm) */
/*
 *  -H0                     no header
 *  -H2 -T4128 -R4096       is plan9 format
 *  -H7                     is elf
 */
enum Headtype {
     H_NOTHING = 0,
     H_PLAN9 = 2, // a.k.a H_AOUT
     H_ELF = 7,
};
/*e: enum [[headtype]](arm) */

/*s: struct [[Auto]](arm) */
struct  Auto
{
    // enum<Sym_kind> (N_LOCAL, N_PARAM, or N_FILE/N_LINE)
    short   type;

    // <ref<Sym>>
    Sym*    asym;
    long    aoffset;

    // Extra
    /*s: [[Auto]] extra fields */
    // list<ref<Auto> (head = curauto or Instr.to.autom of TEXT instruction)
    Auto*   link;
    /*e: [[Auto]] extra fields */
};
/*e: struct [[Auto]](arm) */

/*s: enum [[rxxx]] */
enum rxxx {
    Roffset = 22,       /* no. bits for offset in relocation address */
    Rindex  = 10,       /* no. bits for index in relocation address */
};
/*e: enum [[rxxx]] */
/*s: enum [[misc_constant]](arm) */
enum misc_constants {
    /*s: constant [[BIG]] */
    //BIG       = (1<<12)-4,
    BIG     = 0,
    /*e: constant [[BIG]] */

    /*s: constant [[STRINGSZ]] */
    STRINGSZ    = 200,
    /*e: constant [[STRINGSZ]] */
    /*s: constant NHASH linker */
    NHASH       = 10007,
    /*e: constant NHASH linker */
    /*s: constant NHUNK linker */
    NHUNK       = 100000,
    /*e: constant NHUNK linker */

    /*s: constant [[MINSIZ]] */
    MINSIZ      = 64,
    /*e: constant [[MINSIZ]] */
    /*s: constant [[MAXIO]] */
    MAXIO       = 8192,
    /*e: constant [[MAXIO]] */
    /*s: constant [[MAXHIST]] */
    MAXHIST     = 20,   /* limit of path elements for history symbols */
    /*e: constant [[MAXHIST]] */
};
/*e: enum [[misc_constant]](arm) */

/*s: constant [[SIGNINTERN]](arm) */
#define SIGNINTERN  (1729*325*1729)
/*e: constant [[SIGNINTERN]](arm) */

/*s: constant [[LIBNAMELEN]] */
#define LIBNAMELEN  300
/*e: constant [[LIBNAMELEN]] */

/*s: struct [[Buf]] */
union Buf
{
    struct
    {
        char    obuf[MAXIO];            /* output buffer */
        byte    ibuf[MAXIO];            /* input buffer */
    };
    char    dbuf[1]; // variable size
    //XxX: this cause bugs in kencc under Linux
};
/*e: struct [[Buf]] */

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

// io.c
extern union Buf buf;
extern  int     cbc;
extern  char*   cbp;

// globals.c

extern char	 thechar;
extern char* thestring;

// configuration
extern  short   HEADTYPE;       /* type of header */
extern  long    HEADR;          /* length of header */
extern  long    INITTEXT;       /* text location */
extern  long    INITRND;        /* data round above text location */
extern  long    INITDAT;        /* data location */
extern  char*   INITENTRY;      /* entry point */
extern  long    INITTEXTP;      /* text location (physical) */ // ELF

// output
extern  char*   outfile;
extern  fdt     cout;
extern  Biobuf  bso;

// core algorithm
extern  Sym*    hash[NHASH];
extern  long    pc;
extern  Prog    zprg;

extern  Prog*   firstp;
extern  Prog*   lastp;
extern  Prog*   datap;
extern  Prog*   textp;
extern  Prog*   etextp;

extern  Prog*   curtext;
extern  Auto*   curauto;
extern  Auto*   curhist;
extern  Prog*   curp;
extern  long    autosize;

// sections size
extern  long    textsize;
extern  long    datsize;
extern  long    bsssize;
extern  long    symsize;
extern  long    lcsize;

extern  char*   noname;
/*s: constant [[TNAME]](arm) */
#define TNAME (curtext && curtext->from.sym ? curtext->from.sym->name : noname)
/*e: constant [[TNAME]](arm) */


// debugging support
extern  Sym*    histfrog[MAXHIST];
extern  int     histfrogp;
extern  int     histgen;

// library
extern  int xrefresolv;

// advanced topics
extern  int armv4;
extern  int vfp;
extern  bool doexp;
extern  bool dlm;
extern  char*   EXPTAB;

extern  Prog    undefp;
/*s: constant [[UP]] */
#define UP  (&undefp)
/*e: constant [[UP]] */

// debugging
extern  bool    debug[128];
extern  char*   anames[];

// utils (for statistics)
extern  long    thunk;
extern long nsymbol;


/*s: pragmas varargck type */
#pragma varargck    type    "A" int
#pragma varargck    type    "A" uint
#pragma varargck    type    "C" int
#pragma varargck    type    "D" Adr*
#pragma varargck    type    "N" Adr*
#pragma varargck    type    "P" Prog*
#pragma varargck    type    "S" char*
/*e: pragmas varargck type */
/*s: pragmas varargck argpos */
#pragma varargck    argpos  diag 1
/*e: pragmas varargck argpos */

//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

// obj.c
int     isobjfile(char *f);
void    objfile(char*);

// lib.c
void    loadlib(void);
void    addlibpath(char*);
char*   findlib(char *file);
void    addlib(char *obj);

// pass.c
void    patch(void);
void    follow(void);

// noops.c
void    noops(void);
void    divsig(void);
void    initdiv(void);
void    nocache(Prog*);

// layout.c
void    dodata(void);
void    dotext(void);

// span.c
void    buildop(void);
int     aclass(Adr*);
long    immrot(ulong);
long    immaddr(long);
// oplook() in m.h

long    regoff(Adr*); // for float

// datagen.c
void    nuxiinit(void);
void    datblk(long s, long n, bool sstring);

// codegen.c
// asmout() in m.h

// asm.c
void    asmb(void);



// hist.c
void    addhist(long, int);
void    histtoauto(void);

// debugging.c
void asmsym(void);
void asmlc(void);

// profile.c
void    doprof1(void);
void    doprof2(void);

// float.c
double  ieeedtod(Ieee*);
long    ieeedtof(Ieee*);
int chipfloat(Ieee*);

// dynamic.c
void    zerosig(char*);
void    readundefs(char*, int);
void    dynreloc(Sym*, long, int);
void    asmdyn(void);
void    import(void);
void    export(void);
void    ckoff(Sym*, long);


// io.c
void    cput(int);
void    lput(long);
void    lputl(long l);
void    wput(long);
void    wputl(long);
void    cflush(void);
byte* readsome(fdt f, byte *buf, byte *good, byte *stop, int max);

// error.c
void    diag(char*, ...);
void  errorexit(void);

// utils.c
Sym*  lookup(char*, int);
Prog* prg(void);
// and malloc/free/setmalloctag overwrite
long    atolwhex(char*);
long    rnd(long, long);
int     fileexists(char*);
void  mylog(char*, ...);
/*s: macro [[DBG]] */
#define DBG if(debug['v']) mylog
/*e: macro [[DBG]] */

// fmt.c (dumpers)
void listinit(void);
void    prasm(Prog*);

/*e: linkers/5l/l.h */
