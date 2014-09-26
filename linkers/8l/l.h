/*s: linkers/8l/l.h */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

#include	<common.out.h>
#include	<386/8.out.h>
#include	"elf.h"

/*s: macro DBG */
#define DBG if(debug['v']) mylog
/*e: macro DBG */

/*s: constant P */
#define	P		((Prog*)nil)
/*e: constant P */
/*s: constant S */
#define	S		((Sym*)nil)
/*e: constant S */
/*s: constant TNAME */
#define	TNAME		(curtext ? curtext->from.sym->name : noname)
/*e: constant TNAME */

/*s: function cput */
#define	cput(c)\
    { *cbp++ = c;\
    if(--cbc <= 0)\
        cflush(); }
/*e: function cput */

/*s: constant LIBNAMELEN */
#define	LIBNAMELEN	300
/*e: constant LIBNAMELEN */

typedef	struct	Adr	Adr;
typedef	struct	Prog	Prog;
typedef	struct	Sym	Sym;
typedef	struct	Auto	Auto;
typedef	struct	Optab	Optab;

/*s: struct Adr */
struct	Adr
{
    union
    {
        long	u0offset;
        char	u0scon[8];
        Prog	*u0cond;	/* not used, but should be D_BRANCH */
        Ieee	u0ieee;
    } u0;

    union
    {
        Auto*	u1autom;
        Sym*	u1sym;
    } u1;

    //enum<dxxx>, D_NONE by default
    short	type;
    //enum<dxxx>? D_NONE by default
    uchar	index;

    // TODO: abused for NOPROF function attributes
    char	scale; // offset * scale give size of entity?
};
/*e: struct Adr */

/*s: constant offset */
#define	offset	u0.u0offset
/*e: constant offset */
/*s: constant scon */
#define	scon	u0.u0scon
/*e: constant scon */
/*s: constant cond */
#define	cond	u0.u0cond
/*e: constant cond */
/*s: constant ieee */
#define	ieee	u0.u0ieee
/*e: constant ieee */

/*s: constant autom */
#define	autom	u1.u1autom
/*e: constant autom */
/*s: constant sym */
#define	sym	u1.u1sym
/*e: constant sym */

/*s: struct Prog */
struct	Prog
{
    //enum<opcode>
    short	as;

    Adr	from;
    Adr	to;

    // [[Prog]] other fields
    // 2 by default in zprg, why?
    uchar	back;

    Prog*	forwd;
    long	pc;
    long	line;
    char	width;		/* fake for DATA */
    char	ft;		/* oclass cache */
    char	tt;
    uchar	mark;	/* work on these */

    // [[Prog]] Extra fields

    // list<ref<Prog>> from firstp/lastp, or datap/edatap
    Prog*	link;

    // list<ref<Prog>> from textp/etextp, to follow CALL xxx
    Prog*	pcond;	/* work on this */

};
/*e: struct Prog */
/*s: struct Auto */
struct	Auto
{
    Sym*	asym;

    long	aoffset;
    short	type;

    // Extra
    Auto*	link;
};
/*e: struct Auto */
/*s: struct Sym */
struct	Sym
{
    char	*name;
    short	version; // for static names, each sym has a different version

    //enum<section> ?
    short	type;

    // enum<section> too?
    byte	subtype;

    long	sig;
    long	value; // e.g. pc for a TEXT procedure

    // [[Sym]] other fields
    short	become;
    short	frame;
    ushort	file;

    // Extra

    // hash<Sym.name * Sym.version, ref<Sym>> of hash
    Sym*	link;
};
/*e: struct Sym */
/*s: struct Optab */
struct	Optab
{
    // enum<as> from 8.out.h
    short	as;

    uchar*	ytab;

    // enum<Pxxx>
    byte	prefix;
    // the actual x86 machine code for instruction optab.as
    byte	op[10];
};
/*e: struct Optab */

/*s: enum sxxx */
enum section
{
    STEXT		= 1,
    SDATA,
    SBSS,

    SDATA1,
    SXREF, // means not defined (yet)
    SFILE,
    SCONST,
    SUNDEF,

    SIMPORT,
    SEXPORT,
};
/*e: enum sxxx */
/*s: enum yxxx */
enum yxxx {
    Yxxx		= 0,
    Ynone,
    Yi0,
    Yi1,
    Yi8,
    Yi32,

    Yiauto,

    Yal,
    Ycl,
    Yax,
    Ycx,
    Yrb,
    Yrl,
    Yrf,
    Yf0,
    Yrx,
    Ymb,
    Yml,
    Ym,
    Ybr,
    Ycol,

    Ycs,	Yss,	Yds,	Yes,	Yfs,	Ygs,
    Ygdtr,	Yidtr,	Yldtr,	Ymsw,	Ytask,
    Ycr0,	Ycr1,	Ycr2,	Ycr3,	Ycr4,	Ycr5,	Ycr6,	Ycr7,
    Ydr0,	Ydr1,	Ydr2,	Ydr3,	Ydr4,	Ydr5,	Ydr6,	Ydr7,
    Ytr0,	Ytr1,	Ytr2,	Ytr3,	Ytr4,	Ytr5,	Ytr6,	Ytr7,

    Ymax,
};
/*e: enum yxxx */
/*s: enum zxxx */
enum zxxx {
    Zxxx		= 0,

    Zlit,
    Z_rp,
    Zbr,
    Zcall,
    Zib_,
    Zib_rp,
    Zibo_m,
    Zil_,
    Zil_rp,
    Zilo_m,
    Zjmp,
    Zloop,
    Zm_o,
    Zm_r,
    Zaut_r,
    Zo_m,
    Zpseudo,
    Zr_m,
    Zrp_,
    Z_ib,
    Z_il,
    Zm_ibo,
    Zm_ilo,
    Zib_rr,
    Zil_rr,
    Zclr,
    Zbyte,
    Zmov,
    Zmax,
};
/*e: enum zxxx */
/*s: enum pxxx */
enum pxxx {
    Px		= 0,
    Pe		= 0x66,	/* operand escape */
    Pm		= 0x0f,	/* 2byte opcode escape */
    Pq		= 0xff,	/* both escape */
    Pb		= 0xfe,	/* byte operands */
};
/*e: enum pxxx */
/*s: enum rxxx */
enum rxxx {
    Roffset	= 22,		/* no. bits for offset in relocation address */
    Rindex	= 10,		/* no. bits for index in relocation address */
};
/*e: enum rxxx */

/*s: enum misc1 */
enum misc1 {
    /*s: constant NHASH 8l.h */
    NHASH		= 10007,
    /*e: constant NHASH 8l.h */
    /*s: constant NHUNK */
    NHUNK		= 100000,
    /*e: constant NHUNK */

    MINSIZ		= 4,
    STRINGSZ	= 200,
    MINLC		= 1,
    /*s: constant MAXIO */
    MAXIO		= 8192,
    /*e: constant MAXIO */
    MAXHIST		= 20, /* limit of path elements for history symbols */
};
/*e: enum misc1 */

/*s: enum headtype */
/*
 *	-H0 -T0x40004C -D0x10000000	is garbage unix
 *	-H1 -T0xd0 -R4			is unix coff
 *	-H2 -T4128 -R4096		is plan9 format
 *	-H3 -Tx -Rx			is MS-DOS .COM
 *	-H4 -Tx -Rx			is fake MS-DOS .EXE
 *	-H5 -T0x80100020 -R4096		is ELF
 */
enum headtype {
    H_GARBAGE = 0,
    H_COFF = 1,
    H_PLAN9 = 2, // default
    H_COM = 3,
    H_EXE = 4,
    H_ELF = 5,
};
/*e: enum headtype */

/*s: struct Buf */
union Buf
{
    struct
    {
        char	obuf[MAXIO];			/* output buffer */
        uchar	ibuf[MAXIO];			/* input buffer */
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

#pragma	varargck	type	"A"	int
#pragma	varargck	type	"A"	uint
#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"R"	int
#pragma	varargck	type	"R"	uint
#pragma	varargck	type	"S"	char*

#pragma	varargck	argpos	diag 1

extern	long	HEADR;
extern	long	HEADTYPE;
extern	long	INITDAT;
extern	long	INITRND;
extern	long	INITTEXT;
extern	long	INITTEXTP;
extern	char*	INITENTRY;		/* entry point */

extern	Biobuf	bso;
extern	long	bsssize;
extern	int	cbc;
extern	char*	cbp;
extern	char*	pcstr;
extern	int	cout;
extern	Prog*	curp;
extern	Prog*	curtext;
extern	Prog*	datap;
extern	Prog*	edatap;
extern	long	datsize;
extern	bool	debug[128];
extern	Prog*	firstp;
extern	char	fnuxi8[8];
extern	char	fnuxi4[4];
extern	Sym*	hash[NHASH];
extern	char*	hunk;
extern	char	inuxi1[1];
extern	char	inuxi2[2];
extern	char	inuxi4[4];
extern	char	ycover[Ymax*Ymax];
extern	uchar*	andptr;
extern	uchar	and[30];
extern	char	reg[D_NONE];
extern	Prog*	lastp;
extern	long	lcsize;
extern	int	nerrors;
extern	long	nhunk;
extern	long	nsymbol;
//@Scheck: used by TName, not useless
extern	char*	noname;
extern	char*	outfile;
extern	long	pc;
extern	long	symsize;
extern	Prog*	textp;
extern	long	textsize;
extern	long	thunk;
extern	Prog	zprg;
extern	int	dtype;

extern	Adr*	reloca;
extern	bool	dlm;
extern	int	imports, nimports;
extern	int	exports, nexports;
bool allexport;
extern	char*	EXPTAB;
extern	Prog	undefp;

/*s: constant UP */
#define	UP	(&undefp)
/*e: constant UP */

extern	Optab	optab[];
//@Scheck: defined in ../8c/enam.c
extern	char*	anames[];


Prog*	appendp(Prog*);
void	asmb(void);
void	asmdyn(void);
void	asmins(Prog*);
void	asmlc(void);

void	asmsym(void);
long	atolwhex(char*);

void	cflush(void);
void	ckoff(Sym*, long);
Prog*	copyp(Prog*);

double	cputime(void); //?


void	diag(char*, ...);
void	dodata(void);
void	doinit(void);
void	dostkoff(void);
void	dynreloc(Sym*, ulong, int);

void	errorexit(void);
void	export(void);
int	fileexists(char*);


void	follow(void);
void	gethunk(void);
long	ieeedtof(Ieee*);
void	import(void);

void	listinit(void);
Sym*	lookup(char*, int);
void	lput(long);
void	lputl(long);
void	llput(vlong v);
void	llputl(vlong v);
void	main(int, char*[]);

void	patch(void);
Prog*	prg(void);


long	rnd(long, long);
void	span(void);
void	strnput(char*, int);
void	undef(void);
void	undefsym(Sym*);

void	wput(long);
void	wputl(long);
void	xdefine(char*, int, long);

void mylog(char*, ...);


#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"R"	int
#pragma	varargck	type	"A"	int
/*e: linkers/8l/l.h */
