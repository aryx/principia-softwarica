/*s: linkers/5l/l.h */
#include    <u.h>
#include    <libc.h>
#include    <bio.h>

#include    <common.out.h>
#include    <5.out.h>
#include    "../8l/elf.h"

/*s: macro DBG */
#define DBG if(debug['v']) mylog
/*e: macro DBG */

/*s: constant LIBNAMELEN */
#define LIBNAMELEN  300
/*e: constant LIBNAMELEN */

typedef struct  Adr Adr;
typedef struct  Sym Sym;
typedef struct  Auto    Auto;
typedef struct  Prog    Prog;

/*s: struct Adr(arm) */
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
/*e: struct Adr(arm) */
/*s: enum Operand_class(arm) */
// order of entries for one kind matters! coupling with cmp() and ocmp()
enum Operand_class {
    C_NONE      = 0,
    C_REG,     // D_REG
    C_BRANCH,  // D_BRANCH

    // D_CONST
    /*s: [[Operand_class]] C_xCON cases */
    C_RCON,     /* 0xff rotated */ // [0..0xff] range, possibly rotated
    C_NCON,     /* ~RCON */
    C_LCON,
    /*e: [[Operand_class]] C_xCON cases */

    // D_OREG
    /*s: [[Operand_class]] C_xOREG cases */
    /*s: [[Operand_class]] cases, in C_xOREG, half case */
    C_HOREG,
    /*e: [[Operand_class]] cases, in C_xOREG, half case */
    /*s: [[Operand_class]] cases, in C_xOREG, float cases */
    C_FOREG,
    C_HFOREG,
    /*e: [[Operand_class]] cases, in C_xOREG, float cases */
    C_SOREG,
    C_LOREG,

    C_ROREG,
    C_SROREG,   /* both S and R */
    /*e: [[Operand_class]] C_xOREG cases */
    // D_OREG with symbol N_EXTERN (or N_INTERN)
    /*s: [[Operand_class]] C_xEXT cases */
    /*s: [[Operand_class]] cases, in C_xEXT, half case */
    C_HEXT,
    /*e: [[Operand_class]] cases, in C_xEXT, half case */
    /*s: [[Operand_class]] cases, in C_xEXT, float cases */
    C_FEXT,
    C_HFEXT,
    /*e: [[Operand_class]] cases, in C_xEXT, float cases */
    C_SEXT,
    C_LEXT,
    /*e: [[Operand_class]] C_xEXT cases */
    // D_OREG with symbol N_PARAM or N_LOCAL
    /*s: [[Operand_class]] C_xAUTO cases */
    /*s: [[Operand_class]] cases, in C_xAUTO, half case */
    C_HAUTO,    /* halfword insn offset (-0xff to 0xff) */
    /*e: [[Operand_class]] cases, in C_xAUTO, half case */
    /*s: [[Operand_class]] cases, in C_xAUTO, float cases */
    C_FAUTO,    /* float insn offset (0 to 0x3fc, word aligned) */
    C_HFAUTO,   /* both H and F */
    /*e: [[Operand_class]] cases, in C_xAUTO, float cases */
    C_SAUTO,    /* -0xfff to 0xfff */
    C_LAUTO,
    /*e: [[Operand_class]] C_xAUTO cases */

    // D_ADDR
    /*s: [[Operand_class]] C_xxCON cases */
    C_RECON,
    /*x: [[Operand_class]] C_xxCON cases */
    C_RACON,
    C_LACON,
    /*e: [[Operand_class]] C_xxCON cases */

    /*s: [[Operand_class]] cases */
    C_SHIFT,   // D_SHIFT
    /*x: [[Operand_class]] cases */
    C_ADDR,     /* relocatable address */
    /*x: [[Operand_class]] cases */
    C_FREG,
    C_FCON,
    C_FCR,
    /*x: [[Operand_class]] cases */
    C_REGREG,  // D_REGREG
    /*x: [[Operand_class]] cases */
    C_PSR,     // D_PSR
    /*e: [[Operand_class]] cases */
    C_GOK, // must be at the end e.g. for xcmp[] decl, or buildop loops
};
/*e: enum Operand_class(arm) */

/*s: struct Prog(arm) */
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
/*e: struct Prog(arm) */
/*s: constant P */
#define P       ((Prog*)nil)
/*e: constant P */

/*s: struct Sym */
struct  Sym
{
    // The key
    // ref_own<string>
    char    *name;
    // 0 for global symbols, object file id for private symbols
    short   version; 

    // The generic value, 
    // e.g. virtual pc for a TEXT procedure, size for GLOBL
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
/*e: struct Sym */
/*s: constant S */
#define S       ((Sym*)nil)
/*e: constant S */

/*s: constant TNAME(arm) */
#define TNAME (curtext && curtext->from.sym ? curtext->from.sym->name : noname)
/*e: constant TNAME(arm) */

/*s: enum Section(arm) */
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
/*e: enum Section(arm) */

/*s: enum Mark(arm) */
/* mark flags */
enum Mark {
    /*s: [[Mark]] cases */
    LEAF        = 1<<2,
    /*x: [[Mark]] cases */
    FOLL        = 1<<0,
    /*e: [[Mark]] cases */
};
/*e: enum Mark(arm) */

/*s: enum headtype(arm) */
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
/*e: enum headtype(arm) */

/*s: struct Auto(arm) */
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
/*e: struct Auto(arm) */

/*s: struct Buf */
union Buf
{
    struct
    {
        char    obuf[MAXIO];            /* output buffer */
        byte    ibuf[MAXIO];            /* input buffer */
    };
    char    dbuf[1]; // variable size
};
/*e: struct Buf */

/*s: enum rxxx */
enum rxxx {
    Roffset = 22,       /* no. bits for offset in relocation address */
    Rindex  = 10,       /* no. bits for index in relocation address */
};
/*e: enum rxxx */
/*s: constant SIGNINTERN(arm) */
#define SIGNINTERN  (1729*325*1729)
/*e: constant SIGNINTERN(arm) */
/*s: enum misc_constant(arm) */
enum misc_constants {
    /*s: constant BIG */
    //BIG       = (1<<12)-4,
    BIG     = 0,
    /*e: constant BIG */

    /*s: constant STRINGSZ */
    STRINGSZ    = 200,
    /*e: constant STRINGSZ */
    /*s: constant NHASH linker */
    NHASH       = 10007,
    /*e: constant NHASH linker */
    /*s: constant NHUNK linker */
    NHUNK       = 100000,
    /*e: constant NHUNK linker */

    /*s: constant MINSIZ */
    MINSIZ      = 64,
    /*e: constant MINSIZ */
    /*s: constant MAXIO */
    MAXIO       = 8192,
    /*e: constant MAXIO */
    /*s: constant MAXHIST */
    MAXHIST     = 20,   /* limit of path elements for history symbols */
    /*e: constant MAXHIST */
};
/*e: enum misc_constant(arm) */

// globals.c

extern  short   HEADTYPE;       /* type of header */
extern  long    HEADR;          /* length of header */
extern  long    INITTEXT;       /* text location */
extern  long    INITRND;        /* data round above text location */
extern  long    INITDAT;        /* data location */
extern  char*   INITENTRY;      /* entry point */

extern  long    INITTEXTP;      /* text location (physical) */ // ELF

extern union Buf buf;
extern  int cbc;
extern  char*   cbp;

extern  Sym*    hash[NHASH];
extern  long    pc;
extern  Prog    zprg;

extern  Prog*   curtext;
extern  Auto*   curauto;
extern  Auto*   curhist;
extern  Prog*   curp;
extern  long    autosize;

extern  long    textsize;
extern  long    datsize;
extern  long    bsssize;
extern  long    symsize;
extern  long    lcsize;

extern  char*   outfile;
extern  fdt cout;
extern  Biobuf  bso;

extern  Prog*   firstp;
extern  Prog*   lastp;
extern  Prog*   datap;
extern  Prog*   textp;
extern  Prog*   etextp;

extern  char    inuxi1[1];
extern  char    inuxi2[2];
extern  char    inuxi4[4];
extern  char    fnuxi4[4];
extern  char    fnuxi8[8];

extern  char*   noname;

extern  Sym*    histfrog[MAXHIST];
extern  int histfrogp;
extern  int histgen;

extern  char*   library[50];
extern  char*   libraryobj[50];
extern  int libraryp;
extern  int xrefresolv;


extern  long    instoffset;
extern  char    literal[32];
extern  Optab   optab[];
extern  Oprange oprange[ALAST];
extern  bool    xcmp[C_GOK+1][C_GOK+1];
extern  Prog*   blitrl;
extern  Prog*   elitrl;


extern  int dtype;
extern  int armv4;
extern  int vfp;
extern  int doexp, dlm;
extern  int imports, nimports;
extern  int exports, nexports;
extern  char*   EXPTAB;
extern  Prog    undefp;

extern  Prog*   prog_div;
extern  Prog*   prog_divu;
extern  Prog*   prog_mod;
extern  Prog*   prog_modu;

extern  bool    debug[128];
extern  char*   anames[];

extern  int nerrors;

extern  char*   hunk;
extern  long    nhunk;
extern  long    thunk;

// ???
extern long nsymbol;
extern  int version;

/*s: constant UP */
#define UP  (&undefp)
/*e: constant UP */


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

int Aconv(Fmt*);
int Cconv(Fmt*);
int Dconv(Fmt*);
int Nconv(Fmt*);
int Pconv(Fmt*);
int Sconv(Fmt*);

void    addpool(Prog*, Adr*);
void    initdiv(void);
void    addlibpath(char*);
int     fileexists(char*);
char*   findlib(char*);

int aclass(Adr*);
void    addhist(long, int);
void    addlibpath(char*);
void    append(Prog*, Prog*);
void    asmb(void);
void    asmdyn(void);
void    asmlc(void);
void    asmout(Prog*, Optab*);
void    asmsym(void);
long    atolwhex(char*);
Prog*   brloop(Prog*);
void    buildop(void);
void    buildrep(int, int);
void    cflush(void);
void    ckoff(Sym*, long);
int chipfloat(Ieee*);
int cmp(int, int);
int compound(Prog*);
double  cputime(void);
void    datblk(long, long, int);
void    diag(char*, ...);
void    divsig(void);
void    dodata(void);
void    doprof1(void);
void    doprof2(void);
void    dynreloc(Sym*, long, int);
long    entryvalue(void);
void    errorexit(void);
void    exchange(Prog*);
void    export(void);
int fileexists(char*);
int find1(long, int);
char*   findlib(char*);
void    follow(void);
void    gethunk(void);
void    histtoauto(void);
double  ieeedtod(Ieee*);
long    ieeedtof(Ieee*);
void    import(void);
int isnop(Prog*);
void    ldobj(int, long, char*);
void    loadlib(void);
void    listinit(void);
Sym*    lookup(char*, int);
void    cput(int);
void    llput(vlong);
void    llputl(vlong);
void    lput(long);
void    lputl(long);
void    mkfwd(void);
void*   mysbrk(ulong);
void    names(void);
void    nocache(Prog*);
void    nuxiinit(void);
void    objfile(char*);
int ocmp(const void*, const void*);
long    opirr(int);
Optab*  oplook(Prog*);

long    oprrr(int, int);

long    omvl(Prog*, Adr*, int);

long    olr(int, int, long, int, int);
long    olrr(int, int, int, int, int);

long    osr(int, int, int, long, int);
long    osrr(int, int, int, int, int);

long    olhr(long, int, int, int);
long    olhrr(int, int, int, int);
long    oshr(int, long, int, int);
long    oshrr(int, int, int, int);

long    opvfprrr(int, int);
long    ofsr(int, int, long, int, int, Prog*);


void    patch(void);
void    prasm(Prog*);
void    prepend(Prog*, Prog*);
Prog*   prg(void);
int pseudo(Prog*);
void    putsymb(char*, int, long, int);
void    readundefs(char*, int);
long    regoff(Adr*);
int relinv(int);
long    rnd(long, long);
void    dotext(void);
void    strnput(char*, int);
void    undef(void);
void    undefsym(Sym*);
void    wput(long);
void    wputl(long);
void    xdefine(char*, int, long);
void    xfol(Prog*);
void    zerosig(char*);
void    noops(void);
long    immrot(ulong);
long    immaddr(long);
long    opbra(int, int);

void mylog(char*, ...);

/*e: linkers/5l/l.h */
