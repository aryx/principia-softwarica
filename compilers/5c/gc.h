/*s: 5c/gc.h */
#include	"../cc/cc.h"

#include	<common.out.h>
#include	"arm/5.out.h"

/*s: constant SZ_CHAR(arm) */
/*
 * 5c/arm
 * Arm
 */
#define	SZ_CHAR		1
/*e: constant SZ_CHAR(arm) */
/*s: constant SZ_SHORT(arm) */
#define	SZ_SHORT	2
/*e: constant SZ_SHORT(arm) */
/*s: constant SZ_INT(arm) */
#define	SZ_INT		4
/*e: constant SZ_INT(arm) */
/*s: constant SZ_LONG(arm) */
#define	SZ_LONG		4
/*e: constant SZ_LONG(arm) */
/*s: constant SZ_IND(arm) */
#define	SZ_IND		4
/*e: constant SZ_IND(arm) */
/*s: constant SZ_FLOAT(arm) */
#define	SZ_FLOAT	4
/*e: constant SZ_FLOAT(arm) */
/*s: constant SZ_VLONG(arm) */
#define	SZ_VLONG	8
/*e: constant SZ_VLONG(arm) */
/*s: constant SZ_DOUBLE(arm) */
#define	SZ_DOUBLE	8
/*e: constant SZ_DOUBLE(arm) */
/*s: constant FNX(arm) */
#define	FNX		100
/*e: constant FNX(arm) */
/*s: constant BTRUE(arm) */
#define	BTRUE		0x1000
/*e: constant BTRUE(arm) */

typedef	struct	Adr	Adr;
typedef	struct	Prog	Prog;
typedef	struct	Case	Case;
typedef	struct	C1	C1;
typedef	struct	Multab	Multab;
typedef	struct	Hintab	Hintab;
typedef	struct	Var	Var;
typedef	struct	Reg	Reg;
typedef	struct	Rgn	Rgn;


/*s: constant R0ISZERO(arm) */
#define	R0ISZERO	0
/*e: constant R0ISZERO(arm) */

/*s: struct Adr(arm) */
struct	Adr
{
    long	offset;
    double	dval;
    char	sval[NSNAME];
    Ieee	ieee;

    Sym*	sym;
    char	type;
    char	reg;
    char	name;
    char	etype;
};
/*e: struct Adr(arm) */
/*s: constant A(arm) */
#define	A	((Adr*)0)
/*e: constant A(arm) */

/*s: constant INDEXED(arm) */
#define	INDEXED	9
/*e: constant INDEXED(arm) */
/*s: struct Prog(arm) */
struct	Prog
{
    Adr	from;
    Adr	to;
    Prog*	link;
    long	lineno;
    char	as;
    char	reg;
    uchar	scond;
};
/*e: struct Prog(arm) */
/*s: constant P(arm) */
#define	P	((Prog*)0)
/*e: constant P(arm) */

/*s: struct Case(arm) */
struct	Case
{
    Case*	link;
    vlong	val;
    long	label;
    char	def;
    char isv;
};
/*e: struct Case(arm) */
/*s: constant C(arm) */
#define	C	((Case*)0)
/*e: constant C(arm) */

/*s: struct C1(arm) */
struct	C1
{
    vlong	val;
    long	label;
};
/*e: struct C1(arm) */

/*s: struct Multab(arm) */
struct	Multab
{
    long	val;
    char	code[20];
};
/*e: struct Multab(arm) */

/*s: struct Hintab(arm) */
struct	Hintab
{
    ushort	val;
    char	hint[10];
};
/*e: struct Hintab(arm) */

/*s: struct Var(arm) */
struct	Var
{
    long	offset;
    Sym*	sym;
    char	name;
    char	etype;
};
/*e: struct Var(arm) */

/*s: struct Reg(arm) */
struct	Reg
{
    long	pc;
    long	rpo;		/* reverse post ordering */

    Bits	set;
    Bits	use1;
    Bits	use2;

    Bits	refbehind;
    Bits	refahead;
    Bits	calbehind;
    Bits	calahead;
    Bits	regdiff;
    Bits	act;

    long	regu;
    long	loop;		/* could be shorter */

    
    Reg*	log5;
    long	active;

    Reg*	p1;
    Reg*	p2;
    Reg*	p2link;
    Reg*	s1;
    Reg*	s2;
    Reg*	link;
    Prog*	prog;
};
/*e: struct Reg(arm) */
/*s: constant R(arm) */
#define	R	((Reg*)0)
/*e: constant R(arm) */

/*s: constant NRGN(arm) */
#define	NRGN	1000		/* was 600; raised for paranoia.c */
/*e: constant NRGN(arm) */
/*s: struct Rgn(arm) */
struct	Rgn
{
    Reg*	enter;
    short	cost;
    short	varno;
    short	regno;
};
/*e: struct Rgn(arm) */

extern	long	breakpc;
extern	long	nbreak;
extern	Case*	cases;
extern	Node	constnode;
extern	Node	fconstnode;
extern	long	continpc;
extern	long	curarg;
extern	long	cursafe;
extern	Prog*	firstp;
extern	Prog*	lastp;
extern	long	maxargsafe;
extern	int	mnstring;
extern	Multab	multab[20];
extern	int	hintabsize;
extern	Node*	nodrat;
extern	Node*	nodret;
extern	Node*	nodsafe;
extern	long	nrathole;
extern	long	nstring;
extern	Prog*	p;
extern	long	pc;
extern	Node	regnode;
extern	char	string[NSNAME];
extern	Sym*	symrathole;
extern	Node	znode;
extern	Prog	zprog;
extern	char	reg[NREG+NFREG];
extern	long	exregoffset;
extern	long	exfregoffset;
extern	int	suppress;

/*s: function BLOAD(arm) */
#define	BLOAD(r)	band(bnot(r->refbehind), r->refahead)
/*e: function BLOAD(arm) */
/*s: function BSTORE(arm) */
#define	BSTORE(r)	band(bnot(r->calbehind), r->calahead)
/*e: function BSTORE(arm) */
/*s: function LOAD(arm) */
#define	LOAD(r)		(~r->refbehind.b[z] & r->refahead.b[z])
/*e: function LOAD(arm) */
/*s: function STORE(arm) */
#define	STORE(r)	(~r->calbehind.b[z] & r->calahead.b[z])
/*e: function STORE(arm) */

/*s: function bset(arm) */
#define	bset(a,n)	((a).b[(n)/32]&(1L<<(n)%32))
/*e: function bset(arm) */

/*s: constant CLOAD(arm) */
#define	CLOAD	4
/*e: constant CLOAD(arm) */
/*s: constant CREF(arm) */
#define	CREF	5
/*e: constant CREF(arm) */
/*s: constant CINF(arm) */
#define	CINF	1000
/*e: constant CINF(arm) */
/*s: constant LOOP(arm) */
#define	LOOP	3
/*e: constant LOOP(arm) */

extern	Rgn	region[NRGN];
extern	Rgn*	rgp;
extern	int	nregion;
extern	int	nvar;

extern	Bits	externs;
extern	Bits	params;
extern	Bits	consts;
extern	Bits	addrs;

extern	long	regbits;
extern	long	exregbits;

extern	int	change;

extern	Reg*	firstr;
extern	Reg*	lastr;
extern	Reg	zreg;
extern	Reg*	freer;
extern	Var	var[NVAR];
extern	long*	idom;
extern	Reg**	rpo2r;
extern	long	maxnr;

extern	char*	anames[];
extern	Hintab	hintab[];

/*
 * sgen.c
 */
void	codgen(Node*, Node*);
void	gen(Node*);
void	noretval(int);
void	usedset(Node*, int);
void	xcom(Node*);
int	bcomplex(Node*, Node*);

/*
 * cgen.c
 */
void	cgen(Node*, Node*);
void	cgenrel(Node*, Node*, int);
void	reglcgen(Node*, Node*, Node*);
void	lcgen(Node*, Node*);
void	bcgen(Node*, int);
void	boolgen(Node*, int, Node*);
void	sugen(Node*, Node*, long);
void	layout(Node*, Node*, int, int, Node*);

/*
 * txt.c
 */
void	ginit(void);
void	gclean(void);
void	nextpc(void);
void	gargs(Node*, Node*, Node*);
void	garg1(Node*, Node*, Node*, int, Node**);
Node*	nodconst(long);
Node*	nod32const(vlong);
Node*	nodfconst(double);
void	nodreg(Node*, Node*, int);
void	regret(Node*, Node*);
int	tmpreg(void);
void	regalloc(Node*, Node*, Node*);
void	regfree(Node*);
void	regialloc(Node*, Node*, Node*);
void	regsalloc(Node*, Node*);
void	regaalloc1(Node*, Node*);
void	regaalloc(Node*, Node*);
void	regind(Node*, Node*);
void	gprep(Node*, Node*);
void	raddr(Node*, Prog*);
void	naddr(Node*, Adr*);
void	gmovm(Node*, Node*, int);
void	gmove(Node*, Node*);
void	gmover(Node*, Node*);
void	gins(int a, Node*, Node*);
void	gopcode(int, Node*, Node*, Node*);
int	samaddr(Node*, Node*);
void	gbranch(int);
void	patch(Prog*, long);
int	sconst(Node*);
int	sval(long);
void	gpseudo(int, Sym*, Node*);

/*
 * swt.c
 */
int	swcmp(const void*, const void*);
void	doswit(Node*);
void	swit1(C1*, int, long, Node*);
void	swit2(C1*, int, long, Node*, Node*);
void	casf(void);
void	bitload(Node*, Node*, Node*, Node*, Node*);
void	bitstore(Node*, Node*, Node*, Node*, Node*);
long	outstring(char*, long);
int	mulcon(Node*, Node*);
Multab*	mulcon0(long);
void	nullwarn(Node*, Node*);
void	gextern(Sym*, Node*, long, long);
void	outcode(void);
void	ieeedtod(Ieee*, double);

/*
 * list
 */
void	listinit(void);
int	Pconv(Fmt*);
int	Aconv(Fmt*);
int	Dconv(Fmt*);
int	Sconv(Fmt*);
int	Nconv(Fmt*);
int	Bconv(Fmt*);
int	Rconv(Fmt*);

/*
 * reg.c
 */
Reg*	rega(void);
int	rcmp(const void*, const void*);
void	regopt(Prog*);
void	addmove(Reg*, int, int, int);
Bits	mkvar(Adr*, int);
void	prop(Reg*, Bits, Bits);
void	loopit(Reg*, long);
void	synch(Reg*, Bits);
ulong	allreg(ulong, Rgn*);
void	paint1(Reg*, int);
ulong	paint2(Reg*, int);
void	paint3(Reg*, int, long, int);
void	addreg(Adr*, int);

/*
 * peep.c
 */
void	peep(void);
void	excise(Reg*);
Reg*	uniqp(Reg*);
Reg*	uniqs(Reg*);
int	regtyp(Adr*);
int	regzer(Adr*);
int	anyvar(Adr*);
int	subprop(Reg*);
int	copyprop(Reg*);
int	shiftprop(Reg*);
void	constprop(Adr*, Adr*, Reg*);
int	copy1(Adr*, Adr*, Reg*, int);
int	copyu(Prog*, Adr*, Adr*);

int	copyas(Adr*, Adr*);
int	copyau(Adr*, Adr*);
int	copyau1(Prog*, Adr*);
int	copysub(Adr*, Adr*, Adr*, int);
int	copysub1(Prog*, Adr*, Adr*, int);

long	RtoB(int);
long	FtoB(int);
int	BtoR(long);
int	BtoF(long);

void	predicate(void); 
int	isbranch(Prog *); 
int	predicable(Prog *p); 
int	modifiescpsr(Prog *p); 

#pragma	varargck	type	"A"	int
#pragma	varargck	type	"B"	Bits
#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"N"	Adr*
#pragma	varargck	type	"R"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"S"	char*
/*e: 5c/gc.h */
