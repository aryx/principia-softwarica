/*s: acid/acid.h */
/*s: enum [[_anon_ (acid/acid.h)]] */
/* acid.h */
enum
{
    /*s: acid constants */
    Hashsize	= 128,
    /*x: acid constants */
    Maxproc		= 50,
    /*x: acid constants */
    Maxarg		= 512,
    /*x: acid constants */
    NFD		= 100,
    /*x: acid constants */
    Eof		= -1,
    /*x: acid constants */
    Strsize		= 4096,
    /*x: acid constants */
    Maxval		= 10,
    Mempergc	= 1024*1024,
    /*e: acid constants */
};
/*e: enum [[_anon_ (acid/acid.h)]] */

#pragma varargck type "L"	void

typedef struct Node	Node;
typedef struct String	String;
typedef struct Lsym	Lsym;
typedef struct List	List;
typedef struct Store	Store;
typedef struct Gc	Gc;
typedef struct Strc	Strc;
typedef struct Rplace	Rplace;
typedef struct Ptab	Ptab;
typedef struct Value	Value;
typedef struct Type	Type;
typedef struct Frtype	Frtype;

extern int	kernel;
extern int	remote;
extern int	text;
extern int	silent;
extern Fhdr	fhdr;
extern int	line;
extern Biobuf*	bout;
extern Biobuf*	io[32];
extern int	iop;
extern char	symbol[Strsize];
extern int	interactive;
extern int	na;
extern int	wtflag;
extern Map*	cormap;
extern Map*	symmap;
extern Lsym*	hash[Hashsize];
extern long	dogc;
extern Rplace*	ret;
extern char*	aout;
extern bool	gotint;
extern Gc*	gcl;
extern int	stacked;
extern jmp_buf	err;
extern Node*	prnt;
extern List*	tracelist;
extern int	initialising;
extern int	quiet;

extern void	(*expop[])(Node*, Node*);
/*s: macro [[expr]] */
#define expr(n, r) do{(r)->comt=nil; (*expop[(n)->op])(n, r);}while(0)
/*e: macro [[expr]] */
extern int	fmtsize(Value *v) ;

/*s: enum [[_anon_ (acid/acid.h)]]2 */
enum Type_kind
{
    TINT,
    TFLOAT,

    TSTRING,
    TLIST,

    TCODE,
};
/*e: enum [[_anon_ (acid/acid.h)]]2 */

/*s: struct [[Type]] */
struct Type
{
    Type*	next;

    int	offset;
    char	fmt;
    char	depth;

    Lsym*	type;
    Lsym*	tag;
    Lsym*	base;
};
/*e: struct [[Type]] */

/*s: struct [[Frtype]] */
struct Frtype
{
    Lsym*	var;
    Type*	type;
    Frtype*	next;
};
/*e: struct [[Frtype]] */

/*s: struct [[Ptab]] */
struct Ptab
{
    int	pid;
    fdt	ctl;
};
/*e: struct [[Ptab]] */

extern Ptab	ptab[Maxproc];

/*s: struct [[Rplace]] */
struct Rplace
{
    jmp_buf	rlab;
    Node*	stak;
    Node*	val;
    Lsym*	local;
    Lsym**	tail;
};
/*e: struct [[Rplace]] */

/*s: struct [[Gc]] */
struct Gc
{
    char gcmark;
    Gc*	 gclink;
};
/*e: struct [[Gc]] */

/*s: struct [[Store]] */
struct Store
{
    union {
        vlong	ival;
        double	fval;
        String*	string;
        List*	l;

        Node*	cc;
    };

    // enum<Format_kind> 'X', 'D', ...
    char	fmt;
    /*s: [[Store]] other fields */
    Type*	comt;
    /*e: [[Store]] other fields */
};
/*e: struct [[Store]] */

/*s: struct [[List]] */
struct List
{
    /*s: [[List]] first gc field */
    Gc;
    /*e: [[List]] first gc field */

    List*	next;
    // enum<Type_kind> ?
    char	type;

    Store;
};
/*e: struct [[List]] */

/*s: struct [[Value]] */
struct Value
{
    Store;
    // enum<Type_kind>
    char	type;
    /*s: [[Value]] other fields */
    char	set;
    /*x: [[Value]] other fields */
    Value*	pop;
    Lsym*	scope;
    Rplace*	ret;
    /*e: [[Value]] other fields */
};
/*e: struct [[Value]] */

/*s: struct [[Lsym]] */
struct Lsym
{
    // ref_own<string>
    char*	name;
    // enum<Token_kind>
    int	lexval;

    // ref_own<Value>
    Value*	v;

    /*s: [[Lsym]] other fields */
    Type*	lt;
    Node*	proc;
    Frtype*	local;
    /*x: [[Lsym]] other fields */
    void	(*builtin)(Node*, Node*);
    /*e: [[Lsym]] other fields */

    // Extra fields
    /*s: [[Lsym]] extra fields */
    // list<ref<Lsym>> (next = Sym.hash) bucket of hashtbl 'hash'
    Lsym*	hash;
    /*e: [[Lsym]] extra fields */

};
/*e: struct [[Lsym]] */

/*s: struct [[Node]] */
struct Node
{
    /*s: [[Node]] first gc field */
    Gc;
    /*e: [[Node]] first gc field */

    // enum<opcode>
    char	op;
    Node*	left;
    Node*	right;

    /*s: [[Node]] other fields */
    // enum<Type_kind> ?
    char	type;

    // option<Lsym>, Some when op = ONAME
    Lsym*	sym;

    // ??
    int	builtin;

    // ??
    Store;
    /*e: [[Node]] other fields */
};
/*e: struct [[Node]] */
/*s: constant [[ZN]] */
#define ZN	(Node*)0
/*e: constant [[ZN]] */

/*s: struct [[String]] */
struct String
{
    /*s: [[String]] first gc field */
    Gc;
    /*e: [[String]] first gc field */
    char	*string;
    int	len;
};
/*e: struct [[String]] */

List*	addlist(List*, List*);
List*	al(int);
Node*	an(int, Node*, Node*);
void	append(Node*, Node*, Node*);
int	fbool(Node*);
void	build(Node*);
void	call(char*, Node*, Node*, Node*, Node*);
void	catcher(void*, char*);
void	checkqid(int, int);
void	cmd(void);
Node*	con(vlong);
List*	construct(Node*);
void	ctrace(int);
void	decl(Node*);
void	defcomplex(Node*, Node*);
void	deinstall(int);
void	delete(List*, int n, Node*);
void	dostop(int);
Lsym*	enter(char*, int);
void	error(char*, ...);
void	execute(Node*);
void	fatal(char*, ...);
void	flatten(Node**, Node*);
void	gc(void);
char*	getstatus(int);
void*	gmalloc(long);
void	indir(Map*, uvlong, char, Node*);
void	installbuiltin(void);
void	kinit(void);
int	Lfmt(Fmt*);
int	listcmp(List*, List*);
int	listlen(List*);
List*	listvar(char*, vlong);
void	loadmodule(char*);
void	loadvars(void);
Lsym*	look(char*);
void	ltag(char*);
void	marklist(List*);
Lsym*	mkvar(char*);
void	msg(int, char*);
void	notes(int);
int	nproc(char**);
void	nthelem(List*, int, Node*);
int	numsym(char);
void	odot(Node*, Node*);
void	pcode(Node*, int);
void	pexpr(Node*);
int	popio(void);
void	pstr(String*);
void	pushfile(char*);
void	pushstr(Node*);
void	readtext(char*);
void	restartio(void);
uvlong	rget(Map*, char*);
String	*runenode(Rune*);
int	scmp(String*, String*);
void	sproc(int);
String*	stradd(String*, String*);
String*	straddrune(String*, Rune);
String*	strnode(char*);
String*	strnodlen(char*, int);
char*	system(void);
void	trlist(Map*, uvlong, uvlong, Symbol*);
void	unwind(void);
void	userinit(void);
void	varreg(void);
void	varsym(void);
Waitmsg*	waitfor(int);
void	whatis(Lsym*);
void	windir(Map*, Node*, Node*, Node*);
void	yyerror(char*, ...);
int	yylex(void);
int	yyparse(void);

/*s: enum [[_anon_ (acid/acid.h)]]3 */
enum Opcode
{
    ONAME,
    OCONST,
    OLIST,

    OADD,
    OSUB,
    OMUL,
    ODIV,
    OMOD,

    ORSH,
    OLSH,

    OLT,
    OGT,
    OLEQ,
    OGEQ,
    OEQ,
    ONEQ,

    OLAND,
    OXOR,
    OLOR,
    ONOT,

    OCAND,
    OCOR,

    OEDEC,
    OEINC,
    OPINC,
    OPDEC,

    OIF,
    OELSE,
    OWHILE,
    ODO,
    ORET,

    OASGN,
    OINDM,
    OCALL,
    OINDEX,
    OINDC,
    ODOT,
    OCAST,

    OHEAD,
    OTAIL,
    OAPPEND,
    ODELETE,

    OLOCAL,
    OFRAME,

    OCTRUCT,
    OCOMPLEX,

    OFMT,
    OEVAL,
    OWHAT,
};
/*e: enum [[_anon_ (acid/acid.h)]]3 */
/*e: acid/acid.h */
