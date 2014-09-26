/*s: assemblers/aa/aa.h */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

// aa.h  is the generic part, the specific #include XXX/Y.out.h is done
// in Ya/a.h, e.g.:
//#include "386/8.out.h"
//#include "mips/v.out.h"
//#include "arm/5.out.h"
#include <common.out.h>

#pragma	lib	"../aa/aa.a$O"

// was originally in a XXX/Y.out.h (and was always the same in all archi)
// most of the content below was originally copy pasted in 8a/a.h, 5a/a.h, etc

typedef	struct	Sym	Sym;
typedef	struct	Ref	Ref;
typedef struct  Io Io;
typedef	struct	Hist	Hist;

/*s: constant MAXALIGN */
#define MAXALIGN 7
/*e: constant MAXALIGN */
/*s: constant NSYMB */
#define	NSYMB		500
/*e: constant NSYMB */
/*s: constant BUFSIZ */
#define	BUFSIZ		8192
/*e: constant BUFSIZ */
/*s: constant HISTSZ */
#define	HISTSZ		20
/*e: constant HISTSZ */
/*s: constant NINCLUDE */
#define	NINCLUDE	10
/*e: constant NINCLUDE */
/*s: constant NHUNK */
#define	NHUNK		10000
/*e: constant NHUNK */
/*s: constant EOF */
#define	EOF		(-1)
/*e: constant EOF */
/*s: constant IGN */
#define	IGN		(-2)
/*e: constant IGN */
/*s: function GETC */
#define	GETC()		((--fi.c < 0)? filbuf(): *fi.p++ & 0xff)
/*e: function GETC */
/*s: constant NHASH */
#define	NHASH		503
/*e: constant NHASH */
/*s: constant STRINGSZ */
#define	STRINGSZ	200
/*e: constant STRINGSZ */


/*s: struct Sym */
struct	Sym
{
    char	*name;

    // see also itab[i].type and itab[i].value
    ushort	type;
    long	value; // vlong in va!!

    char	sym;

    char*	macro;

    Ref*	ref; // unused for 5a, matters?

    // Extra
    Sym*	link;
};
/*e: struct Sym */
/*s: constant S */
#define	S	((Sym*)0)
/*e: constant S */

/*s: struct Ref */
// only for 8a actually
struct	Ref
{
    int	class;
};
/*e: struct Ref */

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
    Io*	link;
    char	b[BUFSIZ];
    char*	p;
    short	c;
    short	f;
};
/*e: struct Io */
/*s: constant I */
#define	I	((Io*)0)
/*e: constant I */

/*s: struct Htab */
struct Htab
{
    Sym*	sym;
    short	type;
};
/*e: struct Htab */
extern struct Htab h[NSYM];

// Gen, Gen2?

/*s: struct Hist */
struct	Hist
{
    Hist*	link;
    char*	name;
    long	line;
    long	offset;
};
/*e: struct Hist */
/*s: constant H */
#define	H	((Hist*)0)
/*e: constant H */



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
extern	bool	debug[256];
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

// for macbody, was in a.h
int	getc(void);
void	unget(int);
Sym*	slookup(char*);

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

/*s: enum _anon_ (assemblers/aa/aa.h) */
/*
 *	system-dependent stuff from ../cc/compat.c
 */
enum	/* keep in synch with ../cc/cc.h */
{
    Plan9	= 1<<0,
    Unix	= 1<<1,
};
/*e: enum _anon_ (assemblers/aa/aa.h) */
/*e: assemblers/aa/aa.h */
