/*s: linkers/5l/l.h */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

#include	<common.out.h>
#include	<arm/5.out.h>

#include	"../8l/elf.h"

/*s: macro DBG */
#define DBG if(debug['v']) mylog
/*e: macro DBG */

/*s: constant LIBNAMELEN */
#define	LIBNAMELEN	300
/*e: constant LIBNAMELEN */

void	addlibpath(char*);
int		fileexists(char*);
char*	findlib(char*);

typedef	struct	Adr	Adr;
typedef	struct	Sym	Sym;
typedef	struct	Auto	Auto;
typedef	struct	Prog	Prog;
typedef	struct	Optab	Optab;
typedef	struct	Oprang	Oprang;
typedef	struct	Count	Count;

/*s: constant P */
#define	P		((Prog*)nil)
/*e: constant P */
/*s: constant S */
#define	S		((Sym*)nil)
/*e: constant S */
/*s: constant TNAME(arm) */
#define	TNAME (curtext && curtext->from.sym ? curtext->from.sym->name : noname)
/*e: constant TNAME(arm) */

/*s: struct Adr(arm) */
struct	Adr
{
    // enum<operand_kind> (D_NONE by default)
    short	type;

    union {
        long	offset;
        Ieee*	ieee;
        char*	sval;
    };

    /*s: [[Adr]] other fields */
    // option<enum<registr>> None = R_NONE
    short	reg; 
    /*x: [[Adr]] other fields */
    union {
        Sym*	sym;
        Auto*	autom;
    };
    /*x: [[Adr]] other fields */
    // enum<sym_kind>
    short	symkind;
    /*x: [[Adr]] other fields */
    // enum<classx>
    short	class;
    /*e: [[Adr]] other fields */
};
/*e: struct Adr(arm) */

/*s: struct Prog(arm) */
struct	Prog
{
    //enum<opcode>
    byte	as;

    // operands
    Adr	from;
    Adr	to;

    /*s: [[Prog]] other fields */
    // option<enum<registr>>, None = R_NONE
    short	reg;
    /*x: [[Prog]] other fields */
    // enum<instr_cond>
    byte	scond;
    /*x: [[Prog]] other fields */
    long	line;
    /*x: [[Prog]] other fields */
    long	pc;
    /*x: [[Prog]] other fields */
    //bitset<enum<mark>>
    short	mark;
    /*x: [[Prog]] other fields */
    Prog*	forwd;
    /*x: [[Prog]] other fields */
    byte	optab;
    /*e: [[Prog]] other fields */

    // Extra
    /*s: [[Prog]] extra fields */
    // list<ref<Prog>> from firstp/lastp, or datap/edatap
    Prog*	link;
    /*x: [[Prog]] extra fields */
    // list<ref<Prog>> from textp/etextp
    Prog*	cond;
    /*e: [[Prog]] extra fields */
};
/*e: struct Prog(arm) */

/*s: struct Sym */
struct	Sym
{
    char	*name;
    short	version; // for static names, each sym has a different version

    //enum<section>
    short	type;
    // generic value, e.g. virtual pc for a TEXT procedure, size for GLOBL
    long	value; 

    /*s: [[Sym]] other fields */
    // md5sum of the type of the symbol
    long	sig;
    /*x: [[Sym]] other fields */
    // idx in filen
    ushort	file;
    /*x: [[Sym]] other fields */
    // enum<section> too?
    short	subtype;
    /*x: [[Sym]] other fields */
    short	become;
    /*x: [[Sym]] other fields */
    short	frame;
    /*e: [[Sym]] other fields */
    // Extra
    /*s: [[Sym]] extra fields */
    // hash<Sym.name * Sym.version, ref<Sym>> of hash
    Sym*	link;
    /*e: [[Sym]] extra fields */
};
/*e: struct Sym */

/*s: constant SIGNINTERN(arm) */
#define SIGNINTERN	(1729*325*1729)
/*e: constant SIGNINTERN(arm) */

/*s: struct Auto(arm) */
struct	Auto
{
    // enum<name_kind> (but D_AUTO or D_PARAM only?)
    short	type;

    Sym*	asym;
    long	aoffset;

    // Extra
    /*s: [[Auto]] extra fields */
    Auto*	link;
    /*e: [[Auto]] extra fields */
};
/*e: struct Auto(arm) */
/*s: struct Optab(arm) */
struct	Optab
{
    // enum<opcode> from 5.out.h, but represents a range, see oprange[]
    byte	as;

    // enum<cxxx>, possible class for first operand
    short	a1;
    // enum<cxxx>, possible class for second operand
    short	a2;
    // enum<cxxx>, possible class for third operand
    short	a3;

    // idx for the code generator, see the giant switch in asmout()
    short	type; 

    // size of the corresponding machine code, should be a multiple of 4
    short	size; 

    /*s: [[Optab]] other fields */
    // 0 | REGSB | REGSP
    char	param;
    // enum<lxxx>
    char	flag;
    /*e: [[Optab]] other fields */
};
/*e: struct Optab(arm) */
/*s: struct Oprang(arm) */
struct	Oprang
{
    Optab*	start;
    Optab*	stop;
};
/*e: struct Oprang(arm) */

/*s: enum sxxx(arm) */
enum section
{
    SNONE = 0,

    STEXT,
    SDATA,
    SBSS,

    SXREF,
    /*s: enum sxxx cases */
    SFILE,
    /*x: enum sxxx cases */
    SLEAF, // arm
    /*x: enum sxxx cases */
    SDATA1,
    /*x: enum sxxx cases */
    SSTRING, // arm
    /*x: enum sxxx cases */
    SIMPORT,
    SEXPORT,
    /*x: enum sxxx cases */
    SUNDEF,
    /*x: enum sxxx cases */
    SCONST,
    /*e: enum sxxx cases */
};
/*e: enum sxxx(arm) */
/*s: enum lxxx(arm) */
enum lxxx {
    LFROM	= 1<<0,
    LTO		= 1<<1,
    LPOOL	= 1<<2,
    /*s: enum lxxx cases */
    V4		= 1<<3,	/* arm v4 arch */
    VFP		= 1<<4,	/* arm vfpv3 floating point */
    /*e: enum lxxx cases */
};
/*e: enum lxxx(arm) */
/*s: enum cxxx(arm) */
enum classx {
    C_NONE		= 0,

    C_REG,
    C_REGREG,
    C_SHIFT,
    C_PSR,

    C_BRANCH,

    /*s: cxxx(arm) cases */
    C_HEXT,
    C_SEXT,
    C_LEXT,

    C_FEXT,
    C_HFEXT,
    /*x: cxxx(arm) cases */
    C_HAUTO,	/* halfword insn offset (-0xff to 0xff) */
    C_SAUTO,	/* -0xfff to 0xfff */
    C_LAUTO,

    C_FAUTO,	/* float insn offset (0 to 0x3fc, word aligned) */
    C_HFAUTO,	/* both H and F */
    /*x: cxxx(arm) cases */
    C_HOREG,
    C_SOREG,
    C_LOREG,

    C_ROREG,
    C_SROREG,	/* both S and R */

    C_FOREG,
    C_HFOREG,
    /*x: cxxx(arm) cases */
    C_RCON,		/* 0xff rotated */
    C_NCON,		/* ~RCON */
    C_LCON,
    /*x: cxxx(arm) cases */
    C_RECON,
    /*x: cxxx(arm) cases */
    C_RACON,
    C_LACON,
    /*x: cxxx(arm) cases */
    C_ADDR,		/* relocatable address */
    /*x: cxxx(arm) cases */
    C_FREG,
    C_FCON,
    C_FCR,
    /*e: cxxx(arm) cases */

    C_GOK, // must be at the end e.g. for xcmp[] decl
};
/*e: enum cxxx(arm) */
/*s: enum mark(arm) */
/* mark flags */
enum mark {
    /*s: enum mark cases */
    LEAF		= 1<<2,
    /*x: enum mark cases */
    FOLL		= 1<<0,
    /*e: enum mark cases */
};
/*e: enum mark(arm) */

/*s: enum misc_constant(arm) */
enum misc_constants {
    /*s: constant BIG */
    BIG		= (1<<12)-4,
    /*e: constant BIG */

    /*s: constant STRINGSZ */
    STRINGSZ	= 200,
    /*e: constant STRINGSZ */
    /*s: constant NHASH linker */
    NHASH		= 10007,
    /*e: constant NHASH linker */
    /*s: constant NHUNK linker */
    NHUNK		= 100000,
    /*e: constant NHUNK linker */

    /*s: constant MINSIZ */
    MINSIZ		= 64,
    /*e: constant MINSIZ */
    NENT		= 100,
    /*s: constant MAXIO */
    MAXIO		= 8192,
    /*e: constant MAXIO */
    /*s: constant MAXHIST */
    MAXHIST		= 20,	/* limit of path elements for history symbols */
    /*e: constant MAXHIST */
};
/*e: enum misc_constant(arm) */

/*s: enum rxxx */
enum rxxx {
    Roffset	= 22,		/* no. bits for offset in relocation address */
    Rindex	= 10,		/* no. bits for index in relocation address */
};
/*e: enum rxxx */

/*s: enum headtype(arm) */
/*
 *	-H0				      no header
 *	-H2 -T4128 -R4096	  is plan9 format
 *	-H7				      is elf
 */
enum headtype {
     H_NOTHING = 0,
     H_PLAN9 = 2,
     H_ELF = 7,
};
/*e: enum headtype(arm) */

/*s: struct Buf */
union Buf
{
    struct
    {
        char	obuf[MAXIO];			/* output buffer */
        byte	ibuf[MAXIO];			/* input buffer */
    };
    char	dbuf[1];
};
/*e: struct Buf */
extern union Buf buf;

extern	long	HEADR;			/* length of header */
extern	short	HEADTYPE;		/* type of header */
extern	long	INITDAT;		/* data location */
extern	long	INITRND;		/* data round above text location */
extern	long	INITTEXT;		/* text location */
extern	long	INITTEXTP;		/* text location (physical) */
extern	char*	INITENTRY;		/* entry point */
extern	long	autosize;
extern	Biobuf	bso;
extern	long	bsssize;
extern	int	cbc;
extern	char*	cbp;
extern	int	cout;
extern	Auto*	curauto;
extern	Auto*	curhist;
extern	Prog*	curp;
extern	Prog*	curtext;
extern	Prog*	datap;
extern	long	datsize;
extern	bool	debug[128];
extern	Prog*	etextp;
extern	Prog*	firstp;
extern	char	fnuxi4[4];
extern	char	fnuxi8[8];
extern	char*	noname;
extern	Sym*	hash[NHASH];
extern	Sym*	histfrog[MAXHIST];
extern	int	histfrogp;
extern	int	histgen;
extern	char*	library[50];
extern	char*	libraryobj[50];
extern	int	libraryp;
extern	int	xrefresolv;
extern	char*	hunk;
extern	char	inuxi1[1];
extern	char	inuxi2[2];
extern	char	inuxi4[4];
extern	Prog*	lastp;
extern	long	lcsize;
extern	char	literal[32];
extern	int	nerrors;
extern	long	nhunk;
extern long	nsymbol;
extern	long	instoffset;
extern	Oprang	oprange[ALAST];
extern	char*	outfile;
extern	long	pc;
extern	long	symsize;
extern	Prog*	textp;
extern	long	textsize;
extern	long	thunk;
extern	int	version;
extern	char	xcmp[C_GOK+1][C_GOK+1];
extern	Prog	zprg;
extern	int	dtype;
extern	int	armv4;
extern	int vfp;

extern	int	doexp, dlm;
extern	int	imports, nimports;
extern	int	exports, nexports;
extern	char*	EXPTAB;
extern	Prog	undefp;

/*s: constant UP */
#define	UP	(&undefp)
/*e: constant UP */

extern	char*	anames[];
extern	Optab	optab[];

void	addpool(Prog*, Adr*);
extern	Prog*	blitrl;
extern	Prog*	elitrl;

void	initdiv(void);
extern	Prog*	prog_div;
extern	Prog*	prog_divu;
extern	Prog*	prog_mod;
extern	Prog*	prog_modu;

#pragma	varargck	type	"A"	int
#pragma	varargck	type	"A"	uint
#pragma	varargck	type	"C"	int
#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"N"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"S"	char*

#pragma	varargck	argpos	diag 1

int	Aconv(Fmt*);
int	Cconv(Fmt*);
int	Dconv(Fmt*);
int	Nconv(Fmt*);
int	Pconv(Fmt*);
int	Sconv(Fmt*);

int	aclass(Adr*);
void	addhist(long, int);
void	addlibpath(char*);
void	append(Prog*, Prog*);
void	asmb(void);
void	asmdyn(void);
void	asmlc(void);
void	asmout(Prog*, Optab*);
void	asmsym(void);
long	atolwhex(char*);
Prog*	brloop(Prog*);
void	buildop(void);
void	buildrep(int, int);
void	cflush(void);
void	ckoff(Sym*, long);
int	chipfloat(Ieee*);
int	cmp(int, int);
int	compound(Prog*);
double	cputime(void);
void	datblk(long, long, int);
void	diag(char*, ...);
void	divsig(void);
void	dodata(void);
void	doprof1(void);
void	doprof2(void);
void	dynreloc(Sym*, long, int);
long	entryvalue(void);
void	errorexit(void);
void	exchange(Prog*);
void	export(void);
int	fileexists(char*);
int	find1(long, int);
char*	findlib(char*);
void	follow(void);
void	gethunk(void);
void	histtoauto(void);
double	ieeedtod(Ieee*);
long	ieeedtof(Ieee*);
void	import(void);
int	isnop(Prog*);
void	ldobj(int, long, char*);
void	loadlib(void);
void	listinit(void);
Sym*	lookup(char*, int);
void	cput(int);
void	llput(vlong);
void	llputl(vlong);
void	lput(long);
void	lputl(long);
void	mkfwd(void);
void*	mysbrk(ulong);
void	names(void);
void	nocache(Prog*);
void	nuxiinit(void);
void	objfile(char*);
int	ocmp(const void*, const void*);
long	opirr(int);
Optab*	oplook(Prog*);
long	oprrr(int, int);
long	opvfprrr(int, int);
long	olr(long, int, int, int);
long	olhr(long, int, int, int);
long	olrr(int, int, int, int);
long	olhrr(int, int, int, int);
long	osr(int, int, long, int, int);
long	oshr(int, long, int, int);
long	ofsr(int, int, long, int, int, Prog*);
long	osrr(int, int, int, int);
long	oshrr(int, int, int, int);
long	omvl(Prog*, Adr*, int);
void	patch(void);
void	prasm(Prog*);
void	prepend(Prog*, Prog*);
Prog*	prg(void);
int	pseudo(Prog*);
void	putsymb(char*, int, long, int);
void	readundefs(char*, int);
long	regoff(Adr*);
int	relinv(int);
long	rnd(long, long);
void	dotext(void);
void	strnput(char*, int);
void	undef(void);
void	undefsym(Sym*);
void	wput(long);
void	wputl(long);
void	xdefine(char*, int, long);
void	xfol(Prog*);
void	zerosig(char*);
void	noops(void);
long	immrot(ulong);
long	immaddr(long);
long	opbra(int, int);

void mylog(char*, ...);

/*e: linkers/5l/l.h */
