#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

// aa.h  is the generic part, the specific #include xxx/yyy.out.h is done in a.h
//#include "386/8.out.h"
//#include "mips/v.out.h"
//#include "arm/5.out.h"
#include <common.out.h>

#pragma	lib	"../aa/aa.a$O"

// was originally in a xxx/y.out.h (and was always the same in all archi)

// most of the content below was originally copy pasted in 8a/a.h, 5a/a.h, etc

typedef	struct	Sym	Sym;
typedef	struct	Ref	Ref;
typedef struct  Io Io;
typedef	struct	Hist	Hist;

#define MAXALIGN 7
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


struct	Sym
{
	Sym*	link;

	Ref*	ref; // unused for 5a, matters?

	char*	macro;
	long	value; // vlong in va!!
	ushort	type;
	char	*name;
	char	sym;
};
#define	S	((Sym*)0)

// only for 8a actually
struct	Ref
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

// Gen, Gen2?

struct	Hist
{
	Hist*	link;
	char*	name;
	long	line;
	long	offset;
};
#define	H	((Hist*)0)



// was in a.h
extern	Sym*	hash[NHASH];
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
extern	char*	outfile;
extern	long	pc;
extern	int	peekc;
extern	int	sym;
extern	char	symb[NSYMB];
extern	int	thechar;

// for macbody, was in a.h
extern	char	debug[256];
extern	Hist*	ehist;
extern	int	newflag;
extern	long	thunk;

// for lexbody, was in a.h
int	getnsc(void);
void	gethunk(void);
void	yyerror(char*, ...);
void	linehist(char*, int);
Sym*	lookup(void);
void	syminit(Sym*);
int	filbuf(void);
void	domacro(void);
void	macexpand(Sym*, char*);
void	prfile(long);

// for macbody, was in a.h
int	getc(void);
void	unget(int);
Sym*	slookup(char*);
void	macund(void);
void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);
void*	mysbrk(ulong);

// for macbody, was in lexbody
void pragpack(void);
void pragfpround(void);
void pragprofile(void);
void pragvararg(void);
void* alloc(long n);
void pragincomplete(void);
void* allocn(void *p, long on, long n);
void	pushio(void);
void	newio(void);
void	newfile(char*, int);
void	errorexit(void);

// from lexbody.c
void ieeedtod(Ieee *ieee, double native);

/*
 *	system-dependent stuff from ../cc/compat.c
 */
enum	/* keep in synch with ../cc/cc.h */
{
	Plan9	= 1<<0,
	Unix	= 1<<1,
	Windows	= 1<<2
};
