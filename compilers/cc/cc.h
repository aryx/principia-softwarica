/*s: cc/cc.h */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

#pragma	lib	"../cc/cc.a$O" //$

typedef	struct	Node	Node;
typedef	struct	Type	Type;
typedef	struct	Sym		Sym;
typedef	struct	Decl	Decl;
typedef	struct	Funct	Funct;
typedef	struct	Io		Io;
typedef	struct	Hist	Hist;
typedef	struct	Term	Term;
typedef	struct	Init	Init;
typedef	struct	Bits	Bits;

typedef	Rune	TRune;	/* target system type */

/*s: constant [[NHUNK]] */
#define	NHUNK		50000L
/*e: constant [[NHUNK]] */
/*s: constant [[BUFSIZ]] */
#define	BUFSIZ		8192
/*e: constant [[BUFSIZ]] */
/*s: constant [[NSYMB]] */
#define	NSYMB		1500
/*e: constant [[NSYMB]] */
/*s: constant [[NHASH]] */
#define	NHASH		1024
/*e: constant [[NHASH]] */
/*s: constant [[STRINGSZ]] */
#define	STRINGSZ	200
/*e: constant [[STRINGSZ]] */
/*s: constant [[HISTSZ]] */
#define	HISTSZ		20
/*e: constant [[HISTSZ]] */
/*s: constant [[YYMAXDEPTH]] */
#define YYMAXDEPTH	1500
/*e: constant [[YYMAXDEPTH]] */
/*s: constant [[NTERM]] */
#define	NTERM		10
/*e: constant [[NTERM]] */
/*s: constant [[MAXALIGN]] */
#define	MAXALIGN	7
/*e: constant [[MAXALIGN]] */

/*s: function [[SIGN]] */
#define	SIGN(n)		(1ULL<<(n-1))
/*e: function [[SIGN]] */
/*s: function [[MASK]] */
#define	MASK(n)		(SIGN(n)|(SIGN(n)-1))
/*e: function [[MASK]] */

/*s: constant [[BITS]] */
#define	BITS	5
/*e: constant [[BITS]] */
/*s: constant [[NVAR]] */
#define	NVAR	(BITS*sizeof(ulong)*8)
/*e: constant [[NVAR]] */
/*s: struct [[Bits]] */
struct	Bits
{
    ulong	b[BITS];
};
/*e: struct [[Bits]] */

/*s: struct [[Node]] */
struct	Node
{
    // enum<node_kind>
    char	op;

    // option<ref_own<Node>>
    Node*	left;
    // option<ref_own<Node>>
    Node*	right;

    // after preprocessing, global lineno
    long	lineno; 

    /*s: [[Node]] value fields */
    Sym*	sym; // for ONAME, ODOT/OELEM, OXXX of OLABEL/OGOTO
    /*x: [[Node]] value fields */
    vlong	vconst; /* non fp const */ // for OCONST
    /*x: [[Node]] value fields */
    double	fconst;		/* fp constant */ // for OCONST
    /*x: [[Node]] value fields */
    char*	cstring;	/* character string */ // for OSTRING (and OCONST)
    /*x: [[Node]] value fields */
    TRune*	rstring;	/* rune string */ // for OLSTRING
    /*x: [[Node]] value fields */
    // option<enum<registr>>
    int		reg; // for OREGISTER
    /*e: [[Node]] value fields */

    // ----------------------------------------------------------------------
    // Post parsing annotations
    // ----------------------------------------------------------------------
    /*s: [[Node]] type and storage fields */
    Type*	type;
    /*x: [[Node]] type and storage fields */
    // enum<type_kind>, inline of Node.type->etype?
    char	etype;
    /*x: [[Node]] type and storage fields */
    // enum<storage_class>
    char	class;
    /*e: [[Node]] type and storage fields */
    /*s: [[Node]] code generation fields */
    // address-able, used as a bool to mark lvalues, if can assign you can take
    // the address of. used by xcom() to assign ``addressibility''.
    // (also (ab)used as a bool to mark label uses (true = used in a goto))
    char	addable;
    /*x: [[Node]] code generation fields */
    // complexity in number of registers. for register allocation?
    // (also (ab)used as FNX special value)
    // (also (ab)used as a bool to mark label definitions (true = already defined))
    char	complex; 
    /*x: [[Node]] code generation fields */
    long	xoffset;
    /*x: [[Node]] code generation fields */
    long	pc;
    /*x: [[Node]] code generation fields */
    // ref<Prog>, but use 'void*' to be archi independent
    void*	label;
    /*x: [[Node]] code generation fields */
    char	scale; // x86 only
    /*e: [[Node]] code generation fields */
    /*s: [[Node]] origin tracking fields */
    bool 	xcast;
    /*x: [[Node]] origin tracking fields */
    // enum<node_kind>
    char	oldop;
    /*e: [[Node]] origin tracking fields */

    // ----------------------------------------------------------------------
    // Misc
    // ----------------------------------------------------------------------
    /*s: [[Node]] parsing helper fields */
    // enum<qualifier>
    char	nodegarb;
    /*e: [[Node]] parsing helper fields */
};
/*e: struct [[Node]] */
/*s: constant [[Z]] */
#define	Z	((Node*)nil)
/*e: constant [[Z]] */

/*s: struct [[Sym]] */
struct	Sym
{
    // Symbolic names are used for: 
    //  - identifiers (locals/params/globals, functions, typedefs
    //    and also enum constants)
    //  - tags (struct/union/enum) 
    //  - labels (for the goto)
    // and also during parsing for:
    //  - macros (the #define)
    //  - keywords lexeme (abuse)

    // ----------------------------------------------------------------------
    // The "key"
    // ----------------------------------------------------------------------
    char	*name;

    // ----------------------------------------------------------------------
    // The "value" for the different "namespaces"
    // ----------------------------------------------------------------------
    /*s: [[Sym]] identifier value fields */
    /*s: [[Sym]] identifier value, type and storage fields */
    // ref<Type>
    Type*	type;
    // enum<Storage_class>
    char	class;
    /*e: [[Sym]] identifier value, type and storage fields */
    /*s: [[Sym]] identifier value, scope fields */
    ushort	block;
    /*e: [[Sym]] identifier value, scope fields */
    /*s: [[Sym]] identifier value, checking fields */
    bool	aused;
    /*e: [[Sym]] identifier value, checking fields */
    /*s: [[Sym]] identifier value, code generation fields */
    long	offset;
    /*x: [[Sym]] identifier value, code generation fields */
    // index in h when the Sym is really a symbol, 0 when not a symbol
    char	symidx;
    /*x: [[Sym]] identifier value, code generation fields */
    // enum<signature>
    char	sig;
    /*e: [[Sym]] identifier value, code generation fields */
    /*x: [[Sym]] identifier value fields */
    long	varlineno;
    /*e: [[Sym]] identifier value fields */
    /*s: [[Sym]] enum value fields */
    // ref<Type>
    Type*	tenum;
    /*x: [[Sym]] enum value fields */
    vlong	vconst;
    double	fconst;
    /*e: [[Sym]] enum value fields */
    /*s: [[Sym]] tag value fields */
    Type*	suetag;
    /*x: [[Sym]] tag value fields */
    ushort	sueblock;
    /*e: [[Sym]] tag value fields */
    /*s: [[Sym]] label value fields */
    Node*	label;
    /*e: [[Sym]] label value fields */

    /*s: [[Sym]] macro value fields */
    char*	macro;
    /*e: [[Sym]] macro value fields */
    /*s: [[Sym]] lexeme value fields */
    // enum<lexeme>
    ushort	lexical;
    /*e: [[Sym]] lexeme value fields */

    // ----------------------------------------------------------------------
    // Extra
    // ----------------------------------------------------------------------
    /*s: [[Sym]] extra fields */
    // list<ref<Sym>> (next = Sym.link) bucket of hashtbl 'hash'
    Sym*	link;
    /*e: [[Sym]] extra fields */
};
/*e: struct [[Sym]] */
/*s: constant [[S]] */
#define	S	((Sym*)nil)
/*e: constant [[S]] */

/*s: enum [[signature]] */
enum signature {
    SIGNONE = 0,
    SIGDONE = 1,
    SIGINTERN = 2,

    // ???
    SIGNINTERN = 1729*325*1729,
};
/*e: enum [[signature]] */

/*s: struct [[Decl]] */
struct	Decl
{
    Sym*	sym;
    // enum<Namespace>
    short	val;

    /*s: [[Decl]] sym copy fields */
    Type*	type;  // for Sym.type and Sym.suetag
    ushort	block; // for Sym.block and Sym.sueblock and autobn
    /*x: [[Decl]] sym copy fields */
    long	offset; // for Sym.offset and autoffset
    /*x: [[Decl]] sym copy fields */
    char	class;
    long	varlineno;
    /*x: [[Decl]] sym copy fields */
    bool	aused;
    /*e: [[Decl]] sym copy fields */

    // Extra fields
    /*s: [[Decl]] extra fields */
    // list<ref_own<Decl> of dclstack
    Decl*	link;
    /*e: [[Decl]] extra fields */
};
/*e: struct [[Decl]] */
/*s: constant [[D]] */
#define	D	((Decl*)nil)
/*e: constant [[D]] */

/*s: struct [[Type]] */
struct	Type
{
    // enum<type_kind>
    char	etype;

    // option<ref_own<Type>, e.g. for '*int' have TIND -link-> TINT
    Type*	link;
    // option<list<ref_own<Type>>, next = Type.down, for TFUNC and TSTRUCT
    Type*	down;

    /*s: [[Type]] qualifier fields */
    // enum<qualifier>
    char	garb;
    /*e: [[Type]] qualifier fields */

    // ----------------------------------------------------------------------
    // Post parsing annotations
    // ----------------------------------------------------------------------
    /*s: [[Type]] code generation fields */
    long	width; // ewidth[Type.etype]
    /*x: [[Type]] code generation fields */
    long	offset;
    /*x: [[Type]] code generation fields */
    schar	shift;
    char	nbits;
    /*e: [[Type]] code generation fields */

    /*s: [[Type]] other fields */
    Sym*	tag;
    /*x: [[Type]] other fields */
    Sym*	sym; // for fields in structures
    /*x: [[Type]] other fields */
    Funct*	funct;
    /*e: [[Type]] other fields */
};
/*e: struct [[Type]] */
/*s: constant [[T]] */
#define	T	((Type*)nil)
/*e: constant [[T]] */
/*s: constant [[NODECL]] */
#define	NODECL	((void(*)(int, Type*, Sym*)) nil)
/*e: constant [[NODECL]] */

/*s: struct [[Init]] */
struct	Init			/* general purpose initialization */
{
    int		code;
    ulong	value;
    char*	s;
};
/*e: struct [[Init]] */


/*s: struct [[Fi]] */
struct Fi
{
    // ref<char> (target = Io.b)
    char*	p;
    // remaining characters in Io.b to read
    int	c;
};
/*e: struct [[Fi]] */
extern struct Fi fi;

/*s: struct [[Io]] */
struct	Io
{
    // option<fdt> (None = -1)
    short	f;
    /*s: [[Io]] buffer fields */
    char	b[BUFSIZ];
    /*x: [[Io]] buffer fields */
    // like Fi, saved pointers in Io.b
    char*	p;
    short	c;
    /*e: [[Io]] buffer fields */
    // Extra
    /*s: [[Io]] extra fields */
    // list<ref_own<Io>> (from = iostack or iofree)
    Io*	link;
    /*e: [[Io]] extra fields */
};
/*e: struct [[Io]] */
/*s: constant [[I]] */
#define	I	((Io*)nil)
/*e: constant [[I]] */

/*s: struct [[Hist]] */
struct	Hist
{
    // option<ref_own<string> (None = nil = a ``pop'')
    char*	name;

    // global line of this Hist
    long	line;
    // 0 for #include, +n for #line, -1 for #pragma lib (ugly)
    long	offset;

    // Extra
    /*s: [[Hist]] extra fields */
    Hist*	link;
    /*e: [[Hist]] extra fields */
};
/*e: struct [[Hist]] */
/*s: constant [[H]] */
#define	H	((Hist*)nil)
/*e: constant [[H]] */
extern Hist*	hist;

/*s: struct [[Term]] */
struct	Term
{
    vlong	mult;
    Node	*node;
};
/*e: struct [[Term]] */

/*s: enum [[os]] */
enum os				/* also in ../{8a,5a,0a}.h */
{
    Plan9	= 1<<0,
    Unix	= 1<<1,
    //Windows	= 1<<2,
};
/*e: enum [[os]] */
/*s: enum [[node_kind]] */
enum Node_kind
{
    OXXX,

    // ----------------------------------------------------------------------
    // Names
    // ----------------------------------------------------------------------
    ONAME, // for uses and declarations

    // ----------------------------------------------------------------------
    // Expressions
    // ----------------------------------------------------------------------
    /*s: expression nodes */
    OCOMMA,
    /*x: expression nodes */
    OCONST,
    /*x: expression nodes */
    OSTRING,
    /*x: expression nodes */
    OLSTRING,
    /*x: expression nodes */
    OADD,
    OSUB,

    OMUL,
    ODIV,
    OMOD,
    /*x: expression nodes */
    OPOS,
    ONEG,
    /*x: expression nodes */
    OAND,
    OOR,
    OXOR,

    OASHL,
    OASHR,
    /*x: expression nodes */
    OANDAND,
    OOROR,
    /*x: expression nodes */
    ONOT,
    OCOM,
    /*x: expression nodes */
    OEQ,
    ONE,

    OLT,
    OGT,
    OLE,
    OGE,
    /*x: expression nodes */
    OAS,
    /*x: expression nodes */
    OASADD,
    OASSUB,

    OASMUL,
    OASMOD,
    OASDIV,

    OASAND,
    OASOR,
    OASXOR,

    OASASHL,
    OASASHR,
    /*x: expression nodes */
    OIND, // for uses (dereference) and also declarations
    OADDR,
    /*x: expression nodes */
    ODOT,
    /*x: expression nodes */
    OFUNC, // used for uses (calls) but also defs (and decls)
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
    OARRAY, // used for uses (designator) and declarations
    /*x: expression nodes */
    OASI, // appears during parsing
    /*x: expression nodes */
    OSTRUCT,
    OUNION,
    /*x: expression nodes */
    OSIGN,
    /*e: expression nodes */

    // ----------------------------------------------------------------------
    // Statements
    // ----------------------------------------------------------------------
    /*s: statement nodes */
    OIF,
    /*x: statement nodes */
    OWHILE,
    ODWHILE,
    OFOR,
    /*x: statement nodes */
    ORETURN,

    OBREAK,
    OCONTINUE,
    /*x: statement nodes */
    OLABEL,
    OGOTO,
    /*x: statement nodes */
    OSWITCH,
    OCASE, // for default too, in which case Node.left is null
    /*x: statement nodes */
    OUSED,
    OSET,
    /*e: statement nodes */

    // ----------------------------------------------------------------------
    // Declarations (parameters, initializers, bit fields)
    // ----------------------------------------------------------------------
    /*s: declaration nodes */
    OINIT,
    /*x: declaration nodes */
    OELEM,  // field designator
    /*x: declaration nodes */
    OPROTO,
    /*x: declaration nodes */
    ODOTDOT,
    /*x: declaration nodes */
    OBIT,
    /*e: declaration nodes */

    // ----------------------------------------------------------------------
    // Misc
    // ----------------------------------------------------------------------
    /*s: misc nodes */
    OLIST, // of stmts/labels/parameters/...  and also for pairs/triples/...
    /*x: misc nodes */
    OINDEX, // x86 only
    OREGPAIR, // x86 only, for 64 bits stuff
    /*e: misc nodes */

    // ----------------------------------------------------------------------
    // Post parsing nodes
    // ----------------------------------------------------------------------
    /*s: after parsing nodes */
    OREGISTER,
    /*x: after parsing nodes */
    OLSHR,
    /*x: after parsing nodes */
    OLMUL,
    OLDIV,
    /*x: after parsing nodes */
    OLMOD,
    /*x: after parsing nodes */
    OASLSHR,
    OASLMUL,
    OASLDIV,
    OASLMOD,
    /*x: after parsing nodes */
    OINDREG,
    /*x: after parsing nodes */
    OHI,
    OHS,
    OLO,
    OLS,
    /*x: after parsing nodes */
    OEXREG,
    /*e: after parsing nodes */

    OEND
};
/*e: enum [[node_kind]] */
/*s: enum [[type_kind]] */
enum Type_kind
{
    TXXX,

    /*s: [[Type_kind]] integer cases */
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
    /*e: [[Type_kind]] integer cases */
    /*s: [[Type_kind]] float cases */
    TFLOAT,
    TDOUBLE,
    /*e: [[Type_kind]] float cases */
    /*s: [[Type_kind]] void case */
    TVOID,
    /*e: [[Type_kind]] void case */
    /*s: [[Type_kind]] composite cases */
    TIND,
    TARRAY,

    TFUNC,

    TSTRUCT,
    TUNION,

    TENUM,
    /*e: [[Type_kind]] composite cases */
    /*s: [[Type_kind]] other cases */
    TDOT, // ... in function types
    /*e: [[Type_kind]] other cases */

    NTYPE,
};
/*e: enum [[type_kind]] */
/*s: enum [[type_kind_bis]] */
enum type_kind_bis {
    // ----------------------------------------------------------------------
    // Type (see separate Type_kind)
    // ----------------------------------------------------------------------

    // ----------------------------------------------------------------------
    // Class storage (temporary, see CAUTO/CEXTERN/... for final storage)
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] storage cases */
    TAUTO	= NTYPE,
    TEXTERN,
    TSTATIC,

    TTYPEDEF, // ugly, not really a storage class
    TREGISTER,
    /*x: [[Type_kind_bis]] storage cases */
    TTYPESTR,
    /*e: [[Type_kind_bis]] storage cases */

    // ----------------------------------------------------------------------
    // Qualifiers (aka garbage) (temporary, see GCONSTNT/GVOLATILE)
    // ----------------------------------------------------------------------
    /*s: [[Type_kind_bis]] qualifier cases */
    TCONSTNT,
    TVOLATILE,
    /*e: [[Type_kind_bis]] qualifier cases */

    // ----------------------------------------------------------------------
    // Signs (temporary, see TUINT/TULONG/... for final types)
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

    /*s: constant [[TRUNE]] */
    /* adapt size of Rune to target system's size */
    TRUNE = sizeof(TRune)==4? TUINT: TUSHORT,
    /*e: constant [[TRUNE]] */
};
/*e: enum [[type_kind_bis]] */
/*s: enum [[align]] */
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
/*e: enum [[align]] */
/*s: enum [[dxxx]] */
enum Namespace
{
    DMARK, // special mark to help separate the different scopes

    DAUTO, // locals/parameters/globals/typedefs/functions identifiers
    DSUE,  // struct/union/enum tags
    DLABEL,// labels, goto
};
/*e: enum [[dxxx]] */
/*s: enum [[storage_class]] */
enum Storage_class
{
    CXXX,

    CAUTO,
    CPARAM,

    CEXTERN,
    CGLOBL,
    CSTATIC,

    /*s: [[Storage_class]] cases */
    CTYPEDEF,
    /*x: [[Storage_class]] cases */
    CLOCAL, // local static
    /*x: [[Storage_class]] cases */
    CEXREG, // extern register, kenccext (used in kernel for mips)
    /*x: [[Storage_class]] cases */
    CTYPESTR,
    /*e: [[Storage_class]] cases */

    NCTYPES,
};
/*e: enum [[storage_class]] */
/*s: enum [[qualifier]] */
enum Qualifier
{
    GXXX		= 0, // None

    GCONSTNT	= 1<<0,
    GVOLATILE	= 1<<1,

    NGTYPES		= 1<<2,
    /*s: [[Qualifier]] other cases */
    GINCOMPLETE	= 1<<2,
    /*e: [[Qualifier]] other cases */

};
/*e: enum [[qualifier]] */
/*s: enum [[bxxx]] */
enum Bxxx
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
    BREGISTER	= 1L<<TREGISTER,
    /*s: [[Bxxx]] cases */
    BTYPESTR	= 1L<<TTYPESTR,
    /*e: [[Bxxx]] cases */

    /*s: [[Bxxx]] constants */
    /* these can be overloaded with complex types */
    BCLASS		= BAUTO|BEXTERN|BSTATIC|BTYPEDEF|BTYPESTR|BREGISTER,
    /*x: [[Bxxx]] constants */
    BGARB		= BCONSTNT|BVOLATILE,
    /*x: [[Bxxx]] constants */
    BINTEGER	= BCHAR|BUCHAR|BSHORT|BUSHORT|BINT|BUINT|BLONG|BULONG|BVLONG|BUVLONG,
    BNUMBER		= BINTEGER | BFLOAT|BDOUBLE,
    /*e: [[Bxxx]] constants */
};
/*e: enum [[bxxx]] */

/*s: struct [[Funct]] */
struct	Funct
{
    Sym*	sym[OEND];
    Sym*	castto[NTYPE];
    Sym*	castfr[NTYPE];
};
/*e: struct [[Funct]] */

/*s: struct [[En]] */
struct En
{
    Type*	tenum;		/* type of entire enum */
    Type*	cenum;		/* type of current enum run */
    /*s: [[En]] value fields */
    vlong	lastenum;	/* value of current enum */
    double	floatenum;	/* value of current enum */ // for floats enums
    /*e: [[En]] value fields */
};
/*e: struct [[En]] */
extern struct En en;

extern	int	autobn;
extern	long	autoffset;
extern	int	blockno;
extern	Decl*	dclstack;
extern	char	debug[256];
extern	Hist*	ehist;
extern	bool	firstbit;
extern	Sym*	firstarg;
extern	Type*	firstargtype;
extern	Decl*	firstdcl;
extern	Sym*	hash[NHASH];
extern	char*	hunk;
extern	char**	include;
extern	Io*	iofree;
extern	Io*	ionext;
extern	Io*	iostack;
extern	long	lastbit;
extern	char	lastclass;
extern	Type*	lastdcltype;
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
extern	Type*	thisfntype;
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
extern	char	typeilp[];
extern	char	typechl[];

// not used on ARM
extern	char	typechlv[];
extern	char	typeil[];

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
long	yylex(void);

Sym*	lookup(void);
Sym*	slookup(char*);

int	filbuf(void);
int	getc(void);
int	getnsc(void);
void	newfile(char*, int);
void	newio(void);
void	pushio(void);
void	unget(int);

// used by dpchk.c
long	getr(void);

//!!!! (hmmm in lex.c, as well as cinit(), compile())
void	main(int, char*[]);


/*
 * mac.c
 */
void	dodefine(char*);
void	domacro(void);
void	linehist(char*, int);
void	macexpand(Sym*, char*);

// for dpchk.c
Sym*	getsym(void);
long	getnsn(void);

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
int	bany(Bits*);
int	bnum(Bits);
Bits	blsh(uint);
int	beq(Bits, Bits);
int	bset(Bits, uint);
//Bits	band(Bits, Bits);
//Bits	bnot(Bits);

/*
 * dpchk.c
 */
void dpcheck(Node*);
void arginit(void);

void pragvararg(void);
void pragpack(void);
void pragfpround(void);
void pragprofile(void);
void pragincomplete(void);

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
vlong	convvtox(vlong, int);
//double	convvtof(vlong);
//vlong		convftov(double);
//double	convftox(double, int);

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
