/*s: linkers/8l/l.h */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

#include	<common.out.h>
#include	"386/8.out.h"
#include	"../8l/elf.h"

/*s: constant P */
#define	P		((Prog*)0)
/*e: constant P */
/*s: constant S */
#define	S		((Sym*)0)
/*e: constant S */
/*s: constant TNAME */
#define	TNAME		(curtext?curtext->from.sym->name:noname)
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
typedef	struct	Sym8l	Sym;
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
    short	type;
    uchar	index;
    char	scale;
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
    Adr	from;
    Adr	to;
    Prog	*forwd;
    Prog*	link;
    Prog*	pcond;	/* work on this */
    long	pc;
    long	line;
    short	as;
    char	width;		/* fake for DATA */
    char	ft;		/* oclass cache */
    char	tt;
    uchar	mark;	/* work on these */
    uchar	back;
};
/*e: struct Prog */
/*s: struct Auto */
struct	Auto
{
    Sym*	asym;
    Auto*	link;
    long	aoffset;
    short	type;
};
/*e: struct Auto */
/*s: struct Sym8l */
struct	Sym8l
{
    char	*name;
    short	type;
    short	version;
    short	become;
    short	frame;
    uchar	subtype;
    ushort	file;
    long	value;
    long	sig;
    Sym*	link;
};
/*e: struct Sym8l */
/*s: struct Optab */
struct	Optab
{
    short	as;
    uchar*	ytab;
    uchar	prefix;
    uchar	op[10];
};
/*e: struct Optab */

/*s: enum _anon_ (linkers/8l/l.h) */
enum
{
    STEXT		= 1,
    SDATA,
    SBSS,
    SDATA1,
    SXREF,
    SFILE,
    SCONST,
    SUNDEF,

    SIMPORT,
    SEXPORT,

    NHASH		= 10007,
    NHUNK		= 100000,
    MINSIZ		= 4,
    STRINGSZ	= 200,
    MINLC		= 1,
    MAXIO		= 8192,
    MAXHIST		= 20,				/* limit of path elements for history symbols */

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

    Px		= 0,
    Pe		= 0x66,	/* operand escape */
    Pm		= 0x0f,	/* 2byte opcode escape */
    Pq		= 0xff,	/* both escape */
    Pb		= 0xfe,	/* byte operands */

    Roffset	= 22,		/* no. bits for offset in relocation address */
    Rindex	= 10,		/* no. bits for index in relocation address */
};
/*e: enum _anon_ (linkers/8l/l.h) */

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
extern	long	casepc;
extern	int	cbc;
extern	char*	cbp;
extern	char*	pcstr;
extern	int	cout;
extern	Auto*	curauto;
extern	Auto*	curhist;
extern	Prog*	curp;
extern	Prog*	curtext;
extern	Prog*	datap;
extern	Prog*	edatap;
extern	long	datsize;
extern	char	debug[128];
extern	char	literal[32];
extern	Prog*	etextp;
extern	Prog*	firstp;
extern	char	fnuxi8[8];
extern	char	fnuxi4[4];
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
extern	char	ycover[Ymax*Ymax];
extern	uchar*	andptr;
extern	uchar	and[30];
extern	char	reg[D_NONE];
extern	Prog*	lastp;
extern	long	lcsize;
extern	int	nerrors;
extern	long	nhunk;
extern	long	nsymbol;
extern	char*	noname;
extern	char*	outfile;
extern	long	pc;
extern	long	spsize;
extern	Sym*	symlist;
extern	long	symsize;
extern	Prog*	textp;
extern	long	textsize;
extern	long	thunk;
extern	int	version;
extern	Prog	zprg;
extern	int	dtype;

extern	Adr*	reloca;
extern	int	doexp, dlm;
extern	int	imports, nimports;
extern	int	exports, nexports, allexport;
extern	char*	EXPTAB;
extern	Prog	undefp;

/*s: constant UP */
#define	UP	(&undefp)
/*e: constant UP */

extern	Optab	optab[];
extern	char*	anames[];

int	Aconv(Fmt*);
int	Dconv(Fmt*);
int	Pconv(Fmt*);
int	Rconv(Fmt*);
int	Sconv(Fmt*);
void	addhist(long, int);
void	addlibpath(char*);
Prog*	appendp(Prog*);
void	asmb(void);
void	asmdyn(void);
void	asmins(Prog*);
void	asmlc(void);
void	asmsp(void);
void	asmsym(void);
long	atolwhex(char*);
Prog*	brchain(Prog*);
Prog*	brloop(Prog*);
void	cflush(void);
void	ckoff(Sym*, long);
Prog*	copyp(Prog*);
double	cputime(void);
void	datblk(long, long);
void	diag(char*, ...);
void	dodata(void);
void	doinit(void);
void	doprof1(void);
void	doprof2(void);
void	dostkoff(void);
void	dynreloc(Sym*, ulong, int);
long	entryvalue(void);
void	errorexit(void);
void	export(void);
int	fileexists(char*);
int	find1(long, int);
int	find2(long, int);
char*	findlib(char*);
void	follow(void);
void	gethunk(void);
void	histtoauto(void);
double	ieeedtod(Ieee*);
long	ieeedtof(Ieee*);
void	import(void);
void	ldobj(int, long, char*);
void	loadlib(void);
void	listinit(void);
Sym*	lookup(char*, int);
void	lput(long);
void	lputl(long);
void	llput(vlong v);
void	llputl(vlong v);
void	main(int, char*[]);
void	mkfwd(void);
void*	mysbrk(ulong);
void	nuxiinit(void);
void	objfile(char*);
int	opsize(Prog*);
void	patch(void);
Prog*	prg(void);
void	readundefs(char*, int);
int	relinv(int);
long	reuse(Prog*, Sym*);
long	rnd(long, long);
void	span(void);
void	strnput(char*, int);
void	undef(void);
void	undefsym(Sym*);
long	vaddr(Adr*);
void	wput(long);
void	wputl(long);
void	xdefine(char*, int, long);
void	xfol(Prog*);
int	zaddr(uchar*, Adr*, Sym*[]);
void	zerosig(char*);

#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"R"	int
#pragma	varargck	type	"A"	int
/*e: linkers/8l/l.h */
