/*s: 8c/gc.h */
#include	"../cc/cc.h"

#include	<common.out.h>
#include	<8.out.h>

/*
 * 8c/386
 * Intel 386
 */

/*s: constant [[SZ_CHAR]](x86) */
#define	SZ_CHAR		1
/*e: constant [[SZ_CHAR]](x86) */
/*s: constant [[SZ_SHORT]](x86) */
#define	SZ_SHORT	2
/*e: constant [[SZ_SHORT]](x86) */
/*s: constant [[SZ_INT]](x86) */
#define	SZ_INT		4
/*e: constant [[SZ_INT]](x86) */
/*s: constant [[SZ_LONG]](x86) */
#define	SZ_LONG		4
/*e: constant [[SZ_LONG]](x86) */
/*s: constant [[SZ_IND]](x86) */
#define	SZ_IND		4
/*e: constant [[SZ_IND]](x86) */
/*s: constant [[SZ_FLOAT]](x86) */
#define	SZ_FLOAT	4
/*e: constant [[SZ_FLOAT]](x86) */
/*s: constant [[SZ_VLONG]](x86) */
#define	SZ_VLONG	8
/*e: constant [[SZ_VLONG]](x86) */
/*s: constant [[SZ_DOUBLE]](x86) */
#define	SZ_DOUBLE	8
/*e: constant [[SZ_DOUBLE]](x86) */
/*s: constant FNX (8c/gc.h) */
#define	FNX		100
/*e: constant FNX (8c/gc.h) */

typedef	struct	Adr	Adr;
typedef	struct	Prog	Prog;
typedef	struct	Case	Case;
typedef	struct	C1	C1;
typedef	struct	Var	Var;
typedef	struct	Reg	Reg;
typedef	struct	Rgn	Rgn;

/*s: struct [[Idx]](x86) */
struct Idx
{
    Node*	regtree;
    Node*	basetree;
    short	scale;
    short	reg;
    short	ptr;
};
/*e: struct [[Idx]](x86) */
extern struct Idx idx;

/*s: struct [[Adr]](x86) */
struct	Adr
{
    long	offset;
    double	dval;
    char	sval[NSNAME];
    Sym*	sym;

    uchar	type;
    uchar	index;
    uchar	etype;
    uchar	scale;	/* doubles as width in DATA op */
};
/*e: struct [[Adr]](x86) */
/*s: constant [[A]] */
#define	A	((Adr*)nil)
/*e: constant [[A]] */

/*s: constant [[INDEXED]](x86) */
#define	INDEXED	9
/*e: constant [[INDEXED]](x86) */
/*s: struct [[Prog]](x86) */
struct	Prog
{
    // enum<opcode>, from 8.out.h
    short	as;

    Adr	from;
    Adr	to;

    long	lineno;

    // Extra
    /*s: [[Prog]] extra fields(x86) */
    Prog*	link;
    /*e: [[Prog]] extra fields(x86) */
};
/*e: struct [[Prog]](x86) */
/*s: constant [[P]] */
#define	P	((Prog*)nil)
/*e: constant [[P]] */

/*s: struct [[Case]] */
struct	Case
{
    vlong	val;
    long	label;
    char	def;
    char 	isv;

    Case*	link;
};
/*e: struct [[Case]] */
/*s: constant [[C]] */
#define	C	((Case*)nil)
/*e: constant [[C]] */

/*s: struct [[C1]] */
struct	C1
{
    vlong	val;
    long	label;
};
/*e: struct [[C1]] */

/*s: struct [[Var]] */
struct	Var
{
    Sym*	sym;
    //enum<sym_kind>
    char	symkind;
    //enum<type_kind>
    char	etype;
    long	offset;
};
/*e: struct [[Var]] */

/*s: struct [[Reg]] */
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
/*e: struct [[Reg]] */
/*s: constant [[R]] */
#define	R	((Reg*)nil)
/*e: constant [[R]] */

/*s: constant [[NRGN]](x86) */
#define	NRGN	600
/*e: constant [[NRGN]](x86) */
/*s: struct [[Rgn]] */
struct	Rgn
{
    Reg*	enter;
    short	cost;
    short	varno;
    short	regno;
};
/*e: struct [[Rgn]] */

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
extern	Node*	nodrat;
extern	Node*	nodret;
extern	Node*	nodsafe;
extern	long	nrathole;
extern	long	nstring;
extern	Prog*	p;
extern	long	pc;
extern	Node	regnode;
extern	Node	fregnode0;
extern	Node	fregnode1;
extern	char	string[NSNAME];
extern	Sym*	symrathole;
extern	Node	znode;
extern	Prog	zprog;
extern	int	reg[D_NONE];
extern	long	exregoffset;
extern	long	exfregoffset;

/*s: function [[LOAD]](x86) */
#define	LOAD(r)		(~r->refbehind.b[z] & r->refahead.b[z])
/*e: function [[LOAD]](x86) */
/*s: function [[STORE]](x86) */
#define	STORE(r)	(~r->calbehind.b[z] & r->calahead.b[z])
/*e: function [[STORE]](x86) */

/*s: macro [[bset]](x86) */
//@Scheck: maybe dead, dupe with bits.c function
#define	bset(a,n)	((a).b[(n)/32]&(1L<<(n)%32))
/*e: macro [[bset]](x86) */

/*s: constant [[CLOAD]](x86) */
#define	CLOAD	5
/*e: constant [[CLOAD]](x86) */
/*s: constant [[CREF]](x86) */
#define	CREF	5
/*e: constant [[CREF]](x86) */
/*s: constant [[CINF]](x86) */
#define	CINF	1000
/*e: constant [[CINF]](x86) */
/*s: constant [[LOOP]](x86) */
#define	LOOP	3
/*e: constant [[LOOP]](x86) */

extern	Rgn	region[NRGN];
extern	Rgn*	rgp;
extern	int	nregion;
extern	int	nvar;

extern	Bits	externs;
extern	Bits	params;
extern	Bits	consts;
extern	Bits	addrs;

extern	long	regbits;
//extern	long	exregbits;

extern	int	change;
extern	int	suppress;

extern	Reg*	firstr;
extern	Reg*	lastr;
extern	Reg	zreg;
extern	Reg*	freer;
extern	Var	var[NVAR];
extern	long*	idom;
extern	Reg**	rpo2r;
extern	long	maxnr;

extern	char*	anames[];

/*
 * sgen.c
 */
void	codgen(Node*, Node*);
void	noretval(int);
void	xcom(Node*);

/*
 * cgen.c
 */
void	zeroregm(Node*);
void	cgen(Node*, Node*);
void	reglcgen(Node*, Node*, Node*);
void	lcgen(Node*, Node*);
void	boolgen(Node*, int, Node*);
void	sugen(Node*, Node*, long);
//int	needreg(Node*, int);

/*
 * cgen64.c
 */
int	vaddr(Node*, int);
void	loadpair(Node*, Node*);
int	cgen64(Node*, Node*);
void	testv(Node*, int);
Node*	hi64(Node*);
Node*	lo64(Node*);

/*
 * txt.c
 */
void	ginit(void);
void	gclean(void);
void	gargs(Node*, Node*, Node*);
Node*	nodconst(long);
int	nareg(int);
int	nodreg(Node*, Node*, int);
int	isreg(Node*, int);
void	regret(Node*, Node*);
void	regalloc(Node*, Node*, Node*);
void	regfree(Node*);
void	regialloc(Node*, Node*, Node*);
void	regsalloc(Node*, Node*);
void	regind(Node*, Node*);
void	gmove(Node*, Node*);
void	gins(int a, Node*, Node*);
void	fgopcode(int, Node*, Node*, int, int);
void	gopcode(int, Type*, Node*, Node*);
void	gbranch(int);
void	patch(Prog*, long);
int	sconst(Node*);
void	gpseudo(int, Sym*, Node*);
//void	gprep(Node*, Node*);

/*
 * swt.c
 */
void	doswit(Node*);
void	swit1(C1*, int, long, Node*);
void	casf(void);
void	bitload(Node*, Node*, Node*, Node*, Node*);
void	bitstore(Node*, Node*, Node*, Node*, Node*);
long	outstring(char*, long);
void	nullwarn(Node*, Node*);
void	gextern(Sym*, Node*, long, long);
void	outcode(void);
void	ieeedtod(Ieee*, double);

/*
 * list
 */
void	listinit(void);

/*
 * reg.c
 */
Reg*	rega(void);
void	regopt(Prog*);


/*
 * peep.c
 */
void	peep(void);
void	excise(Reg*);
int	copyu(Prog*, Adr*, Adr*);



/*s: constant [[D_HI]](x86) */
//#define	D_HI	D_NONE
/*e: constant [[D_HI]](x86) */
/*s: constant [[D_LO]](x86) */
//#define	D_LO	D_NONE
/*e: constant [[D_LO]](x86) */

/*
 * com64
 */
int	cond(int);
int	com64(Node*);
void	com64init(void);
void	bool64(Node*);

/*
 * div/mul
 */
void	sdivgen(Node*, Node*, Node*, Node*);
void	udivgen(Node*, Node*, Node*, Node*);
void	sdiv2(long, int, Node*, Node*);
void	smod2(long, int, Node*, Node*);
void	mulgen(Type*, Node*, Node*);
void	genmuladd(Node*, Node*, int, Node*);
void	shiftit(Type*, Node*, Node*);

#pragma	varargck	type	"A"	int
#pragma	varargck	type	"B"	Bits
#pragma	varargck	type	"D"	Adr*
#pragma	varargck	type	"P"	Prog*
#pragma	varargck	type	"R"	int
#pragma	varargck	type	"S"	char*

/*e: 8c/gc.h */
