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
int	fileexists(char*);
char*	findlib(char*);

typedef	struct	Adr	Adr;
typedef	struct	Sym	Sym;
typedef	struct	Autom	Auto;
typedef	struct	Prog	Prog;
typedef	struct	Optab	Optab;
typedef	struct	Oprang	Oprang;
typedef	uchar	Opcross[32][2][32];
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
    //enum<operand_kind> (D_NONE by default)
    byte	type;

    union
    {
        long	u0offset;
        char*	u0sval;
        Ieee*	u0ieee;
    } u0;

    union
    {
        Auto*	u1autom;
        Sym*	u1sym;
    } u1;

    /*s: [[Adr]] other fields */
    char	reg; // Abused for NOPROF and DUPOK
    char	name;
    char	class;
    /*e: [[Adr]] other fields */
};
/*e: struct Adr(arm) */

/*s: constant offset */
#define	offset	u0.u0offset
/*e: constant offset */
/*s: constant sval(arm) */
#define	sval	u0.u0sval
/*e: constant sval(arm) */
/*s: constant ieee */
#define	ieee	u0.u0ieee
/*e: constant ieee */

/*s: constant autom */
#define	autom	u1.u1autom
/*e: constant autom */
/*s: constant sym */
#define	sym	u1.u1sym
/*e: constant sym */

/*s: struct Prog(arm) */
struct	Prog
{
    //enum<opcode>
    byte	as;

    // operands
    Adr	from;
    Adr	to;

    /*s: [[Prog]] other fields */
    long	pc;
    long	line;
    /*x: [[Prog]] other fields */
    // enum<register>
    byte	reg;
    // enum<instr_cond>
    byte	scond;
    /*x: [[Prog]] other fields */
    union
    {
        long	u0regused;
        Prog*	u0forwd;
    } u0;
    /*x: [[Prog]] other fields */
    byte	mark;
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
/*s: constant regused(arm) */
#define	regused	u0.u0regused
/*e: constant regused(arm) */
/*s: constant forwd(arm) */
#define	forwd	u0.u0forwd
/*e: constant forwd(arm) */

/*s: struct Sym */
struct	Sym
{
    char	*name;
    short	version; // for static names, each sym has a different version

    //enum<sxxx>
    short	type;

    long	value; // e.g. pc for a TEXT procedure

    /*s: [[Sym]] other fields */
    short	become;
    short	frame;
    /*x: [[Sym]] other fields */
    // enum<section> too?
    byte	subtype;
    /*x: [[Sym]] other fields */
    // md5sum of the type of the symbol
    long	sig;
    /*x: [[Sym]] other fields */
    // idx in filen
    ushort	file;
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

/*s: struct Autom(arm) */
struct	Autom
{
    Sym*	asym;

    long	aoffset;
    short	type;

    // Extra
    Auto*	link;
};
/*e: struct Autom(arm) */
/*s: struct Optab(arm) */
struct	Optab
{
    // enum<opcode> from 5.out.h
    byte	as;

    char	a1;
    char	a2;
    char	a3;

    char	type;
    char	size;
    char	param;

    char	flag;
};
/*e: struct Optab(arm) */
/*s: struct Oprang(arm) */
struct	Oprang
{
    Optab*	start;
    Optab*	stop;
};
/*e: struct Oprang(arm) */
/*s: struct Count(arm) */
struct	Count
{
    long	count;
    long	outof;
};
/*e: struct Count(arm) */

/*s: enum sxxx(arm) */
enum sxxx
{
    SNONE,

    STEXT,
    SDATA,
    SBSS,

    SXREF,
    /*s: enum sxxx cases */
    SDATA1,
    SLEAF, // arm
    SFILE,
    SCONST,

    SSTRING, // arm
    SUNDEF,

    SIMPORT,
    SEXPORT,
    /*e: enum sxxx cases */
};
/*e: enum sxxx(arm) */
/*s: enum lxxx(arm) */
enum lxxx {
    LFROM	= 1<<0,
    LTO		= 1<<1,
    LPOOL	= 1<<2,

    V4		= 1<<3,	/* arm v4 arch */
    VFP		= 1<<4,	/* arm vfpv3 floating point */
};
/*e: enum lxxx(arm) */
/*s: enum cxxx(arm) */
enum cxxx {
    C_NONE		= 0,
    C_REG,
    C_REGREG,
    C_SHIFT,
    C_FREG,
    C_PSR,
    C_FCR,

    C_RCON,		/* 0xff rotated */
    C_NCON,		/* ~RCON */
    C_SCON,		/* 0xffff */
    C_LCON,
    C_FCON,

    C_RACON,
    C_LACON,

    C_RECON,
    C_LECON,

    C_SBRA,
    C_LBRA,

    C_HAUTO,	/* halfword insn offset (-0xff to 0xff) */
    C_FAUTO,	/* float insn offset (0 to 0x3fc, word aligned) */
    C_HFAUTO,	/* both H and F */
    C_SAUTO,	/* -0xfff to 0xfff */
    C_LAUTO,

    C_HEXT,
    C_FEXT,
    C_HFEXT,
    C_SEXT,
    C_LEXT,

    C_HOREG,
    C_FOREG,
    C_HFOREG,
    C_SOREG,
    C_ROREG,
    C_SROREG,	/* both S and R */
    C_LOREG,

    C_ADDR,		/* relocatable address */

    C_GOK,
};
/*e: enum cxxx(arm) */
/*s: enum mark(arm) */
/* mark flags */
enum mark {
    FOLL		= 1<<0,
    LABEL		= 1<<1,
    LEAF		= 1<<2,
};
/*e: enum mark(arm) */

/*s: enum misc_constant(arm) */
enum misc_constants {
    BIG		= (1<<12)-4,

    /*s: constant STRINGSZ */
    STRINGSZ	= 200,
    /*e: constant STRINGSZ */
    /*s: constant NHASH linker */
    NHASH		= 10007,
    /*e: constant NHASH linker */
    /*s: constant NHUNK linker */
    NHUNK		= 100000,
    /*e: constant NHUNK linker */

    MINSIZ		= 64,
    NENT		= 100,
    /*s: constant MAXIO */
    MAXIO		= 8192,
    /*e: constant MAXIO */
    MAXHIST		= 20,	/* limit of path elements for history symbols */
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
    } u;
    char	dbuf[1];
};
/*e: struct Buf */
extern union Buf buf;

/*s: constant cbuf */
#define	cbuf	u.obuf
/*e: constant cbuf */
/*s: constant xbuf */
#define	xbuf	u.ibuf
/*e: constant xbuf */

extern	long	HEADR;			/* length of header */
extern	long	HEADTYPE;		/* type of header */
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
extern	Opcross	opcross[8];
extern	Oprang	oprange[ALAST];
extern	char*	outfile;
extern	long	pc;
extern	uchar	repop[ALAST];
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
void	span(void);
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
