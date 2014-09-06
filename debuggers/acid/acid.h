/*s: acid/acid.h */
/*s: enum _anon_ (acid/acid.h) */
/* acid.h */
enum
{
    Eof		= -1,
    Strsize		= 4096,
    Hashsize	= 128,
    Maxarg		= 512,
    NFD		= 100,
    Maxproc		= 50,
    Maxval		= 10,
    Mempergc	= 1024*1024,
};
/*e: enum _anon_ (acid/acid.h) */

#pragma varargck type "L"	void

typedef struct Node	Node;
typedef struct StringAcid	String;
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
extern int	gotint;
extern Gc*	gcl;
extern int	stacked;
extern jmp_buf	err;
extern Node*	prnt;
extern List*	tracelist;
extern int	initialising;
extern int	quiet;

extern void	(*expop[])(Node*, Node*);
/*s: macro expr */
#define expr(n, r) do{(r)->comt=0; (*expop[(n)->op])(n, r);}while(0)
/*e: macro expr */
extern int	fmtsize(Value *v) ;

/*s: enum _anon_ (acid/acid.h)2 */
enum
{
    TINT,
    TFLOAT,
    TSTRING,
    TLIST,
    TCODE,
};
/*e: enum _anon_ (acid/acid.h)2 */

/*s: struct Type */
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
/*e: struct Type */

/*s: struct Frtype */
struct Frtype
{
    Lsym*	var;
    Type*	type;
    Frtype*	next;
};
/*e: struct Frtype */

/*s: struct Ptab */
struct Ptab
{
    int	pid;
    int	ctl;
};
/*e: struct Ptab */

extern Ptab	ptab[Maxproc];

/*s: struct Rplace */
struct Rplace
{
    jmp_buf	rlab;
    Node*	stak;
    Node*	val;
    Lsym*	local;
    Lsym**	tail;
};
/*e: struct Rplace */

/*s: struct Gc */
struct Gc
{
    char	gcmark;
    Gc*	gclink;
};
/*e: struct Gc */

/*s: struct Store */
struct Store
{
    char	fmt;
    Type*	comt;
    union {
        vlong	ival;
        double	fval;
        String*	string;
        List*	l;
        Node*	cc;
    };
};
/*e: struct Store */

/*s: struct List */
struct List
{
    Gc;
    List*	next;
    char	type;
    Store;
};
/*e: struct List */

/*s: struct Value */
struct Value
{
    char	set;
    char	type;
    Store;
    Value*	pop;
    Lsym*	scope;
    Rplace*	ret;
};
/*e: struct Value */

/*s: struct Lsym */
struct Lsym
{
    char*	name;
    int	lexval;
    Lsym*	hash;
    Value*	v;
    Type*	lt;
    Node*	proc;
    Frtype*	local;
    void	(*builtin)(Node*, Node*);
};
/*e: struct Lsym */

/*s: struct Node */
struct Node
{
    Gc;
    char	op;
    char	type;
    Node*	left;
    Node*	right;
    Lsym*	sym;
    int	builtin;
    Store;
};
/*e: struct Node */
/*s: constant ZN */
#define ZN	(Node*)0
/*e: constant ZN */

/*s: struct StringAcid */
struct StringAcid
{
    Gc;
    char	*string;
    int	len;
};
/*e: struct StringAcid */

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

/*s: enum _anon_ (acid/acid.h)3 */
enum
{
    ONAME,
    OCONST,
    OMUL,
    ODIV,
    OMOD,
    OADD,
    OSUB,
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
    OCAND,
    OCOR,
    OASGN,
    OINDM,
    OEDEC,
    OEINC,
    OPINC,
    OPDEC,
    ONOT,
    OIF,
    ODO,
    OLIST,
    OCALL,
    OCTRUCT,
    OWHILE,
    OELSE,
    OHEAD,
    OTAIL,
    OAPPEND,
    ORET,
    OINDEX,
    OINDC,
    ODOT,
    OLOCAL,
    OFRAME,
    OCOMPLEX,
    ODELETE,
    OCAST,
    OFMT,
    OEVAL,
    OWHAT,
};
/*e: enum _anon_ (acid/acid.h)3 */
/*e: acid/acid.h */
