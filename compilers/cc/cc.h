/*s: cc/cc.h */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

#pragma	lib	"../cc/cc.a$O" //$

typedef	struct	Node	Node;
typedef	struct	Type	Type;
typedef	struct	Sym		Sym;
typedef	struct	Funct	Funct;
typedef	struct	Decl	Decl;
typedef	struct	Io		Io;
typedef	struct	Hist	Hist;
typedef	struct	Term	Term;
typedef	struct	Init	Init;
typedef	struct	Bits	Bits;

typedef	Rune	TRune;	/* target system type */

/*s: constant NHUNK */
#define	NHUNK		50000L
/*e: constant NHUNK */
/*s: constant BUFSIZ */
#define	BUFSIZ		8192
/*e: constant BUFSIZ */
/*s: constant NSYMB */
#define	NSYMB		1500
/*e: constant NSYMB */
/*s: constant NHASH */
#define	NHASH		1024
/*e: constant NHASH */
/*s: constant STRINGSZ */
#define	STRINGSZ	200
/*e: constant STRINGSZ */
/*s: constant HISTSZ */
#define	HISTSZ		20
/*e: constant HISTSZ */
/*s: constant YYMAXDEPTH */
//@Scheck: used in t.tab.c probably
#define YYMAXDEPTH	1500
/*e: constant YYMAXDEPTH */
/*s: constant NTERM */
#define	NTERM		10
/*e: constant NTERM */
/*s: constant MAXALIGN */
#define	MAXALIGN	7
/*e: constant MAXALIGN */

/*s: function SIGN */
#define	SIGN(n)		(1ULL<<(n-1))
/*e: function SIGN */
/*s: function MASK */
#define	MASK(n)		(SIGN(n)|(SIGN(n)-1))
/*e: function MASK */

/*s: constant BITS */
#define	BITS	5
/*e: constant BITS */
/*s: constant NVAR */
#define	NVAR	(BITS*sizeof(ulong)*8)
/*e: constant NVAR */
/*s: struct Bits */
struct	Bits
{
    ulong	b[BITS];
};
/*e: struct Bits */

/*s: struct Node */
struct	Node
{
    // enum<node_kind>
    char	op;

    // option<ref_own<Node>>
    Node*	left;
    // option<ref_own<Node>>
    Node*	right;

    long	lineno;

    /*s: [[Node]] other fields */
    Sym*	sym; 
    /*x: [[Node]] other fields */
    Type*	type;
    /*x: [[Node]] other fields */
    // enum<storage_class>
    char	class;
    /*x: [[Node]] other fields */
    void*	label;

    long	pc;
    int		reg;
    long	xoffset;

    // (ab)used as bool for marker of label def (true) vs use (false),
    // FNX special value, register allocation value, etc
    char	complex; 

    char	scale;
    /*x: [[Node]] other fields */
    // enum<type_kind>, inline of Node.type->etype?
    char	etype;
    /*x: [[Node]] other fields */
    // enum<qualifier>
    char	garb;
    /*x: [[Node]] other fields */
    vlong	vconst;		/* non fp const */ // abused in switch?
    /*x: [[Node]] other fields */
    double	fconst;		/* fp constant */
    /*x: [[Node]] other fields */
    char*	cstring;	/* character string */ // also used for ints
    /*x: [[Node]] other fields */
    TRune*	rstring;	/* rune string */
    /*x: [[Node]] other fields */
    bool 	xcast;
    /*x: [[Node]] other fields */
    // used as a bool to mark lvalues.
    // (ab)used as bool for marker of use of labels.
    // (ab)used by xcom to assign ``addressibility''.
    char	addable;
    /*x: [[Node]] other fields */
    // enum<node_kind>
    char	oldop;
    /*e: [[Node]] other fields */
};
/*e: struct Node */
/*s: constant Z */
#define	Z	((Node*)nil)
/*e: constant Z */

/*s: struct Sym */
struct	Sym
{
    // for: 
    //  - keywords, 
    //  - identifiers (locals/params/globals/functions/typedefs),
    //  - tags (struct/union/enum) 
    //  - labels (for the goto)
    //  - macros
    char	*name;

    long	varlineno;

    /*s: [[Sym]] other fields */
    Type*	type;
    // enum<storage_class>
    char	class;
    /*x: [[Sym]] other fields */
    char*	macro;
    /*x: [[Sym]] other fields */
    ushort	lexical;
    /*x: [[Sym]] other fields */
    ushort	block;
    /*x: [[Sym]] other fields */
    long	offset;

    vlong	vconst;
    double	fconst;

    char	sym;
    /*x: [[Sym]] other fields */
    Node*	label;
    /*x: [[Sym]] other fields */
    Type*	suetag;
    /*x: [[Sym]] other fields */
    ushort	sueblock;
    /*x: [[Sym]] other fields */
    // ref<Type>
    Type*	tenum;
    /*x: [[Sym]] other fields */
    bool	aused;
    /*x: [[Sym]] other fields */
    // enum<signature>
    char	sig;
    /*e: [[Sym]] other fields */

    // Extra
    /*s: [[Sym]] extra fields */
    // list<ref<Sym>> (next = Sym.link) bucket of hashtbl 'hash'
    Sym*	link;
    /*e: [[Sym]] extra fields */
};
/*e: struct Sym */
/*s: constant S */
#define	S	((Sym*)nil)
/*e: constant S */

/*s: enum signature */
enum signature {
    SIGNONE = 0,
    SIGDONE = 1,
    SIGINTERN = 2,

    // ???
    SIGNINTERN = 1729*325*1729,
};
/*e: enum signature */

/*s: struct Decl */
struct	Decl
{
    Sym*	sym;
    // enum<dxxx>
    short	val;

    /*s: [[Decl]] sym copy fields */
    Type*	type;
    char	class;
    long	offset;
    ushort	block;
    long	varlineno;
    bool	aused;
    /*e: [[Decl]] sym copy fields */

    // Extra fields
    /*s: [[Decl]] extra fields */
    // list<ref_own<Decl> of dclstack
    Decl*	link;
    /*e: [[Decl]] extra fields */
};
/*e: struct Decl */
/*s: constant D */
#define	D	((Decl*)nil)
/*e: constant D */

/*s: struct Type */
struct	Type
{
    // enum<type_kind>  (the type cases only?)
    char	etype;

    // option<ref_own<Type>, e.g. for '*int' have TIND -link-> TINT
    Type*	link;

    // ??
    Type*	down;

    long	lineno;

    /*s: [[Type]] other fields */
    Sym*	sym;
    /*x: [[Type]] other fields */
    // enum<qualifier>
    char	garb;
    /*x: [[Type]] other fields */
    long	width; // ewidth[Type.etype]

    long	offset;
    schar	shift;
    char	nbits;
    /*x: [[Type]] other fields */
    Sym*	tag;
    /*x: [[Type]] other fields */
    Funct*	funct;
    /*e: [[Type]] other fields */
};
/*e: struct Type */
/*s: constant T */
#define	T	((Type*)nil)
/*e: constant T */
/*s: constant NODECL */
#define	NODECL	((void(*)(int, Type*, Sym*))nil)
/*e: constant NODECL */

/*s: struct Init */
struct	Init			/* general purpose initialization */
{
    int		code;
    ulong	value;
    char*	s;
};
/*e: struct Init */


/*s: struct Fi */
struct Fi
{
    char*	p;
    int	c;
};
/*e: struct Fi */
extern struct Fi fi;

/*s: struct Io */
struct	Io
{
    char	b[BUFSIZ];

    short	f;

    char*	p;
    short	c;

    Io*	link;
};
/*e: struct Io */
/*s: constant I */
#define	I	((Io*)nil)
/*e: constant I */

/*s: struct Hist */
struct	Hist
{
    char*	name;

    long	line;
    long	offset;

    Hist*	link;
};
/*e: struct Hist */
/*s: constant H */
#define	H	((Hist*)nil)
/*e: constant H */
extern Hist*	hist;

/*s: struct Term */
struct	Term
{
    vlong	mult;
    Node	*node;
};
/*e: struct Term */

/*s: enum os */
enum os				/* also in ../{8a,5a,0a}.h */
{
    Plan9	= 1<<0,
    Unix	= 1<<1,
    //Windows	= 1<<2,
};
/*e: enum os */
/*s: enum node_kind */
enum node_kind
{
    OXXX,

    // ----------------------------------------------------------------------
    // Names
    // ----------------------------------------------------------------------
    ONAME,

    // ----------------------------------------------------------------------
    // Expressions
    // ----------------------------------------------------------------------
    /*s: expression nodes */
    OCOMMA,
    /*x: expression nodes */
    OADD,
    OSUB,

    OMUL,
    ODIV,
    OMOD,

    OASHL,
    OASHR,

    OAND,
    OOR,
    OXOR,
    /*x: expression nodes */
    OPOS,
    ONEG,
    /*x: expression nodes */
    OANDAND,
    OOROR,
    /*x: expression nodes */
    OEQ,
    ONE,

    OLT,
    OGT,
    OLE,
    OGE,
    /*x: expression nodes */
    ONOT,
    OCOM,
    /*x: expression nodes */
    OAS,

    OASADD,
    OASSUB,

    OASMUL,
    OASMOD,
    OASDIV,

    OASASHL,
    OASASHR,

    OASAND,
    OASOR,
    OASXOR,
    /*x: expression nodes */
    OIND, // for uses (dereference) and also defs (and decls)
    OADDR,
    /*x: expression nodes */
    ODOT,
    /*x: expression nodes */
    OCONST,
    /*x: expression nodes */
    OSTRING,
    /*x: expression nodes */
    OLSTRING,
    /*x: expression nodes */
    OCAST,
    /*x: expression nodes */
    OCOND,
    /*x: expression nodes */
    OPREDEC,
    OPREINC,
    /*x: expression nodes */
    OPOSTINC,
    OPOSTDEC,
    /*x: expression nodes */
    OSIZE,
    /*x: expression nodes */
    OASI, // appears during parsing
    /*x: expression nodes */
    OINDREG, // after parsing only
    /*x: expression nodes */
    OASLMUL,
    OASLDIV,
    OASLMOD,
    OASLSHR,
    /*x: expression nodes */
    OLMUL,
    OLDIV,
    OLMOD,
    OLSHR,
    /*x: expression nodes */
    // after parsing only
    OHI,
    OHS,
    OLO,
    OLS,
    /*x: expression nodes */
    OSIGN,
    /*e: expression nodes */

    // ----------------------------------------------------------------------
    // Statements
    // ----------------------------------------------------------------------
    /*s: statement nodes */
    OIF,
    /*x: statement nodes */
    OFOR,
    OWHILE,
    ODWHILE,
    /*x: statement nodes */
    ORETURN,

    OBREAK,
    OCONTINUE,
    /*x: statement nodes */
    OSWITCH,
    OCASE, // for default too, in which case Node.left is null
    /*x: statement nodes */
    OLABEL,
    OGOTO,
    /*x: statement nodes */
    OUSED,
    OSET,
    /*e: statement nodes */

    // ----------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------
    /*s: variable declaration nodes */
    OINIT,
    /*x: variable declaration nodes */
    OELEM,  // field designator
    /*e: variable declaration nodes */

    // ----------------------------------------------------------------------
    // Declarations
    // ----------------------------------------------------------------------
    /*s: declaration nodes */
    OPROTO,
    /*e: declaration nodes */

    // ----------------------------------------------------------------------
    // Definitions
    // ----------------------------------------------------------------------
    /*s: definition nodes */
    OSTRUCT,
    OUNION,
    /*x: definition nodes */
    OBIT,
    /*x: definition nodes */
    OREGISTER, // after parsing only?
    /*e: definition nodes */

    // ----------------------------------------------------------------------
    // Misc
    // ----------------------------------------------------------------------
    /*s: misc nodes */
    OLIST, // of stmts/labels/parameters/...  and also for pairs/triples/...
    /*x: misc nodes */
    ODOTDOT,
    /*x: misc nodes */
    OFUNC, // used for uses (calls) but also defs (and decls) :(
    /*x: misc nodes */
    OARRAY, // used for uses (including designator) and defs (and decl)
    /*x: misc nodes */
    OEXREG,
    /*x: misc nodes */
    OINDEX, // x86 only
    OREGPAIR, // x86 only, for 64 bits stuff
    /*e: misc nodes */

    OEND
};
/*e: enum node_kind */
/*s: enum type_kind */
enum type_kind
{
    TXXX,
    /*s: type cases */
    TCHAR,
    TUCHAR,
    TSHORT,
    TUSHORT,
    TINT,
    TUINT,
    TLONG,
    TULONG,
    TVLONG,
    TUVLONG,
    TFLOAT,
    TDOUBLE,
    /*x: type cases */
    TIND,
    TFUNC,
    TARRAY,
    TVOID,
    TSTRUCT,
    TUNION,
    TENUM,

    TDOT, // ... in function types
    /*e: type cases */
    NTYPE,
};
/*e: enum type_kind */
/*s: enum type_kind_bis */
enum type_kind_bis {
    // ----------------------------------------------------------------------
    // Storage (temporary, see CAUTO/CEXTERN/... for final storage)
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] storage cases */
    TAUTO	= NTYPE,
    TEXTERN,
    TSTATIC,
    TTYPEDEF, // ugly, not really a storage class
    TTYPESTR,
    TREGISTER,
    /*e: [[Type_kind_bis]] storage cases */

    // ----------------------------------------------------------------------
    // Qualifiers (aka garbage) (temporary, see GCONSTNT/GVOLATILE)
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] qualifier cases */
    TCONSTNT,
    TVOLATILE,
    /*e: [[Type_kind_bis]] qualifier cases */

    // ----------------------------------------------------------------------
    // Signs (temporary, see TUINT/TULONG/...)
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] sign cases */
    TUNSIGNED,
    TSIGNED,
    /*e: [[Type_kind_bis]] sign cases */

    // ----------------------------------------------------------------------
    // Misc
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] misc cases */
    TOLD,
    /*e: [[Type_kind_bis]] misc cases */

    NALLTYPES,

    /* adapt size of Rune to target system's size */
    TRUNE = sizeof(TRune)==4? TUINT: TUSHORT,
};
/*e: enum type_kind_bis */
/*s: enum align */
enum align
{
    Axxx,

    Ael1,
    Ael2,
    Asu2,
    Aarg0,
    Aarg1,
    Aarg2,
    Aaut3,

    NALIGN,
};
/*e: enum align */
/*s: enum dxxx */
enum dxxx
{
    DMARK, // special mark to help represent a stack of list??

    DAUTO, // locals/parameters/globals/typedefs/functions identifiers
    DSUE,  // struct/union/enum tags
    DLABEL,// labels, goto
};
/*e: enum dxxx */
/*s: enum storage_class */
enum storage_class
{
    CXXX, // nothing specified

    CAUTO,
    CEXTERN,
    CGLOBL,
    CSTATIC,

    CLOCAL,
    CPARAM,

    /*s: [[Storage_class]] cases */
    CTYPEDEF,
    CTYPESTR,

    CSELEM,
    CLABEL,
    /*x: [[Storage_class]] cases */
    CEXREG, // extern register, kencc ext (used in kernel for mips)
    /*e: [[Storage_class]] cases */

    NCTYPES,
};
/*e: enum storage_class */
/*s: enum qualifier */
enum qualifier
{
    GXXX		= 0,

    GCONSTNT	= 1<<0,
    GVOLATILE	= 1<<1,

    NGTYPES		= 1<<2,
    GINCOMPLETE	= 1<<2,
};
/*e: enum qualifier */
/*s: enum bxxx */
enum bxxx
{
    BCHAR		= 1L<<TCHAR,
    BUCHAR		= 1L<<TUCHAR,
    BSHORT		= 1L<<TSHORT,
    BUSHORT		= 1L<<TUSHORT,
    BINT		= 1L<<TINT,
    BUINT		= 1L<<TUINT,
    BLONG		= 1L<<TLONG,
    BULONG		= 1L<<TULONG,
    BVLONG		= 1L<<TVLONG,
    BUVLONG		= 1L<<TUVLONG,
    BFLOAT		= 1L<<TFLOAT,
    BDOUBLE		= 1L<<TDOUBLE,

    BIND		= 1L<<TIND,
    BFUNC		= 1L<<TFUNC,
    BARRAY		= 1L<<TARRAY,
    BVOID		= 1L<<TVOID,
    BSTRUCT		= 1L<<TSTRUCT,
    BUNION		= 1L<<TUNION,
    BENUM		= 1L<<TENUM,

    BDOT		= 1L<<TDOT,

    BCONSTNT	= 1L<<TCONSTNT,
    BVOLATILE	= 1L<<TVOLATILE,

    BUNSIGNED	= 1L<<TUNSIGNED,
    BSIGNED		= 1L<<TSIGNED,

    BAUTO		= 1L<<TAUTO,
    BEXTERN		= 1L<<TEXTERN,
    BSTATIC		= 1L<<TSTATIC,
    BTYPEDEF	= 1L<<TTYPEDEF,
    BTYPESTR	= 1L<<TTYPESTR,
    BREGISTER	= 1L<<TREGISTER,


    BINTEGER	= BCHAR|BUCHAR|BSHORT|BUSHORT|BINT|BUINT|
                BLONG|BULONG|BVLONG|BUVLONG,
    BNUMBER		= BINTEGER|BFLOAT|BDOUBLE,

/* these can be overloaded with complex types */

    BCLASS		= BAUTO|BEXTERN|BSTATIC|BTYPEDEF|BTYPESTR|BREGISTER,
    BGARB		= BCONSTNT|BVOLATILE,
};
/*e: enum bxxx */

/*s: struct Funct */
struct	Funct
{
    Sym*	sym[OEND];
    Sym*	castto[NTYPE];
    Sym*	castfr[NTYPE];
};
/*e: struct Funct */

/*s: struct En */
struct En
{
    Type*	tenum;		/* type of entire enum */

    Type*	cenum;		/* type of current enum run */

    vlong	lastenum;	/* value of current enum */
    double	floatenum;	/* value of current enum */ // for floats enums
};
/*e: struct En */
extern struct En en;

extern	int	autobn;
extern	long	autoffset;
extern	int	blockno;
extern	Decl*	dclstack;
extern	char	debug[256];
extern	Hist*	ehist;
extern	long	firstbit;
extern	Sym*	firstarg;
extern	Type*	firstargtype;
extern	Decl*	firstdcl;
extern	Sym*	hash[NHASH];
extern	int	hasdoubled;
extern	char*	hunk;
extern	char**	include;
extern	Io*	iofree;
extern	Io*	ionext;
extern	Io*	iostack;
extern	long	lastbit;
extern	char	lastclass;
extern	Type*	lastdcl;
extern	long	lastfield;
extern	Type*	lasttype;
extern	long	lineno;
extern	long	nearln;
extern	int	maxinclude;
extern	int	nerrors;
extern	bool	newflag;
extern	long	nhunk;
extern	int	ninclude;
extern	Node*	nodproto;
extern	Node*	nodcast;
extern	Biobuf	outbuf;
extern	Biobuf	diagbuf;
extern	char*	outfile;
extern	char*	pathname;
extern	int	peekc;
extern	long	stkoff;
extern	Type*	strf;
extern	Type*	strl;
extern	char	symb[NSYMB];
extern	Sym*	symstring;
extern	int	taggen;
extern	Type*	tfield;
extern	Type*	tufield;
extern	int	thechar;
extern	char*	thestring;
extern	Type*	thisfn;
extern	long	thunk;
extern	Type*	types[NTYPE];

extern	Type*	fntypes[NTYPE];
extern	Node*	initlist;
extern	Term	term[NTERM];
extern	int	nterm;
extern	int	packflg;
extern	int	fproundflg;
extern	bool	profileflg;
extern	int	ncontin;
extern	bool	newvlongcode;
extern	int	canreach;
extern	int	warnreach;
extern	Bits	zbits;

extern	char	*onames[], *tnames[], *gnames[];
extern	char	*cnames[], *qnames[], *bnames[];

extern	char	comrel[], invrel[], logrel[];
extern	long	ncast[], tadd[], tand[];
extern	long	targ[], tasadd[], tasign[], tcast[];
extern	long	tdot[], tfunct[], tindir[], tmul[];
extern	long	tnot[], trel[], tsub[];

extern	char	typeaf[];
extern	char	typefd[];
extern	char	typei[];
extern	char	typesu[];
extern	char	typesuv[];
extern	char	typeu[];
extern	char	typev[];
extern	char	typeil[];
extern	char	typeilp[];
extern	char	typechl[];
extern	char	typechlv[];

extern	char	typechlp[];
extern	char	typechlpfd[];

extern	char*	typeswitch;
extern	char*	typeword;
extern	char*	typecmplx;

extern	ulong	thash1;
extern	ulong	thash2;
extern	ulong	thash3;
extern	ulong	thash[];

/*
 *	compat.c/unix.c/windows.c
 */
int	mywait(int*);
int	mycreat(char*, int);
int	myaccess(char*);
int	pathchar(void);
bool	systemtype(int);

// utils.c
void	gethunk(void);
void*	allocn(void*, long, long);
void*	alloc(long);
void	errorexit(void);
void	yyerror(char*, ...);

/*
 *	parser
 */
//@Scheck: def in y.tab.c from cc.y
int	yyparse(void);

/*
 *	lex.c
 */
int	filbuf(void);
int	getc(void);
int	getnsc(void);
Sym*	lookup(void);
void	newfile(char*, int);
void	newio(void);
void	pushio(void);
Sym*	slookup(char*);
void	unget(int);
long	yylex(void);

//!!!! (hmmm in lex.c, as well as cinit(), compile())
void	main(int, char*[]);


/*
 * mac.c
 */
void	dodefine(char*);
void	domacro(void);
void	linehist(char*, int);
void	macexpand(Sym*, char*);

/*
 * dcl.c
 */
//@Scheck: useful, used by cc.y
Type*	tcopy(Type*);
//@Scheck: useful, used by cc.y
Node*	doinit(Sym*, Type*, long, Node*);
//@Scheck: useful, used by cc.y
void	adecl(int, Type*, Sym*);
void	argmark(Node*, int);
Node*	dcllabel(Sym*, int);
Node*	dodecl(void(*)(int, Type*, Sym*), int, Type*, Node*);
//@Scheck: useful, used by cc.y
Sym*	mkstatic(Sym*);
void	doenum(Sym*, Node*);
void	snap(Type*);
Type*	dotag(Sym*, int, int);
void	edecl(int, Type*, Sym*);
void	markdcl(void);
//@Scheck: useful, used by cc.y
void	pdecl(int, Type*, Sym*);
Node*	revertdcl(void);
long	round(long, int);
int	sametype(Type*, Type*);
ulong	sign(Sym*);
ulong	signature(Type*);
void	sualign(Type*);
void	xdecl(int, Type*, Sym*);
Node*	contig(Sym*, Node*, long);

/*
 * com.c
 */
void	ccom(Node*);
void	complex(Node*);
int	tcom(Node*);
int	tcoma(Node*, Node*, Type*, int);
int	tcomo(Node*, int);
void	constas(Node*, Type*, Type*);
Node*	uncomma(Node*);

/*
 * con.c
 */
void	acom(Node*);
void	evconst(Node*);

/*
 * funct.c
 */
int	isfunct(Node*);
void	dclfunct(Type*, Sym*);

/*
 * sub.c
 */
void	arith(Node*, int);
int	deadheads(Node*);
Type*	dotsearch(Sym*, Type*, Node*, long*);
Node*	invert(Node*);
int	bitno(long);
void	makedot(Node*, Type*, long);
int	mixedasop(Type*, Type*);
Node*	new(int, Node*, Node*);
Node*	new1(int, Node*, Node*);
int	nilcast(Type*, Type*);
int	nocast(Type*, Type*);
void	prtree(Node*, char*);
void	prtree1(Node*, int, int);
void	relcon(Node*, Node*);
int	relindex(int);
//@Scheck: useful, used by cc.y
int	simpleg(long);
Type*	garbt(Type*, long);
int	simplec(long);
Type*	simplet(long);
int	stcompat(Node*, Type*, Type*, long[]);
int	tcompat(Node*, Type*, Type*, long[]);
void	tinit(void);
Type*	typ(int, Type*);
Type*	copytyp(Type*);
void	typeext(Type*, Node*);
void	typeext1(Type*, Node*);
int	side(Node*);
int	vconst(Node*);
int	log2(uvlong);
int	vlog(Node*);
int	topbit(ulong);
void	simplifyshift(Node*);
long	typebitor(long, long);
void	diag(Node*, char*, ...);
void	warn(Node*, char*, ...);
void	fatal(Node*, char*, ...);

/*
 * acid.c
 */
void	acidtype(Type*);
void	acidvar(Sym*);

/*
 * pickle.c
 */
void	pickletype(Type*);

/*
 * bits.c
 */
Bits	bor(Bits, Bits);
//Bits	band(Bits, Bits);
//Bits	bnot(Bits);
int	bany(Bits*);
int	bnum(Bits);
Bits	blsh(uint);
int	beq(Bits, Bits);
int	bset(Bits, uint);

/*
 * dpchk.c
 */
void	dpcheck(Node*);
void	arginit(void);
//void	pragvararg(void);
//void	pragpack(void);
//void	pragfpround(void);
//void  pragprofile(void);
//void	pragincomplete(void);

/*
 * calls to machine depend part
 */
//todo: could define an interface instantiated by each xc
void	codgen(Node*, Node*);
void	gclean(void);
void	gextern(Sym*, Node*, long, long);
void	ginit(void);
long	outstring(char*, long);
long	outlstring(TRune*, long);
void	xcom(Node*);
long	exreg(Type*);
long	align(long, Type*, int);
long	maxround(long, long);

extern	schar	ewidth[];

/*
 * com64
 */
int	com64(Node*);
void	com64init(void);
void	bool64(Node*);
//double	convvtof(vlong);
//vlong		convftov(double);
//double	convftox(double, int);
vlong	convvtox(vlong, int);

/*
 * machcap
 */
int	machcap(Node*);

#pragma	varargck	argpos	warn	2
#pragma	varargck	argpos	diag	2
#pragma	varargck	argpos	yyerror	1

#pragma	varargck	type	"F"	Node*
#pragma	varargck	type	"L"	long
#pragma	varargck	type	"Q"	long
#pragma	varargck	type	"O"	int
#pragma	varargck	type	"T"	Type*
#pragma	varargck	type	"|"	int
/*e: cc/cc.h */
