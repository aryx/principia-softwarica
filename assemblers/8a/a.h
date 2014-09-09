#include <u.h>
#include <libc.h>
#include <bio.h>

#include "386/8.out.h"

typedef	struct	Sym8a	Sym;
typedef	struct	Ref8a	Ref;
typedef	struct	Io	Io;
typedef	struct	Hist	Hist;
typedef	struct	Gen	Gen;
typedef	struct	Gen2 	Gen2;

#define	MAXALIGN	7
#define	FPCHIP		1
#define	NSYMB		500
#define	BUFSIZ		8192
#define	HISTSZ		20
#define	NINCLUDE	10
#define	NHUNK		10000
#define	EOF		(-1)
#define	IGN		(-2)
#define	GETC()		((--fi.c < 0)? filbuf(): *fi.p++ & 0xff)
#define	NHASH		503
#define	STRINGSZ	200
#define	NMACRO		10

struct	Sym8a
{
	Sym*	link;

	Ref*	ref;

	char*	macro;
	long	value;
	ushort	type;
	char	*name;
	char	sym;
};
#define	S	((Sym*)0)

struct	Ref8a
{
	int	class;
};

struct Fi
{
	char*	p;
	int	c;
};
extern struct Fi fi;

struct	Io
{
	Io*	link;
	char	b[BUFSIZ];
	char*	p;
	short	c;
	short	f;
};
#define	I	((Io*)0)


struct Htab
{
	Sym*	sym;
	short	type;
};
extern struct Htab h[NSYM];



struct	Gen
{
	double	dval;
	char	sval[8];
	long	offset;
	long	offset2;
	Sym*	sym;
	short	type;
	short	index;
	short	scale;
};
struct	Gen2
{
	Gen	from;
	Gen	to;
};

struct	Hist
{
	Hist*	link;
	char*	name;
	long	line;
	long	offset;
};
#define	H	((Hist*)0)

enum
{
	CLAST,
	CMACARG,
	CMACRO,
	CPREPROC,
};


extern	char	debug[256];
extern	Sym*	hash[NHASH];
extern	char*	Dlist[30];
extern	int	nDlist;
extern	Hist*	ehist;
extern	int	newflag;
extern	Hist*	hist;
extern	char*	hunk;
extern	char*	include[NINCLUDE];
extern	Io*	iofree;
extern	Io*	ionext;
extern	Io*	iostack;
extern	long	lineno;
extern	int	nerrors;
extern	long	nhunk;
extern	int	ninclude;
extern	Gen	nullgen;
extern	char*	outfile;
extern	int	pass;
extern	char*	pathname;
extern	long	pc;
extern	int	peekc;
extern	int	sym;
extern	char	symb[NSYMB];
extern	int	thechar;
extern	char*	thestring;
extern	long	thunk;
extern	Biobuf	obuf;

long	yylex(void);
int	escchar(int);
void	cinit(void);
void	checkscale(int);
void	cclean(void);
int	isreg(Gen*);
void	outcode(int, Gen2*);
void	outhist(void);
void	zaddr(Gen*, int);
void	zname(char*, int, int);
Sym*	getsym(void);
int	assemble(char*);

// used by lexbody.c
void	gethunk(void);
int	getnsc(void);
void	linehist(char*, int);
Sym*	lookup(void);
void	syminit(Sym*);
int	filbuf(void);
void	domacro(void);
void	macexpand(Sym*, char*);
void	prfile(long);

// for lexbody
void	setinclude(char*);
void*	allocn(void*, long, long);
void	errorexit(void);
Sym*	slookup(char*);
void	pinit(char*);
void	ieeedtod(Ieee*, double);
void	dodefine(char*);
void	yyerror(char*, ...);
int	yyparse(void);

// for macbody
int	getc(void);
void	unget(int);
void	macund(void);
void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);
void	pushio(void);
void	newio(void);
void	newfile(char*, int);

// for macbody, was in lexbody

/*
 *	system-dependent stuff from ../cc/compat.c
 */
enum	/* keep in synch with ../cc/cc.h */
{
	Plan9	= 1<<0,
	Unix	= 1<<1,
	Windows	= 1<<2
};
int	mywait(int*);
int	mycreat(char*, int);
int	systemtype(int);
int	pathchar(void);
char*	mygetwd(char*, int);
int	myexec(char*, char*[]);
int	mydup(int, int);
int	myfork(void);
int	mypipe(int*);
void*	mysbrk(ulong);
