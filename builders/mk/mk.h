/*s: mk/mk.h */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
#include	<regexp.h>

extern Biobuf bout;

/*s: struct Bufblock */
typedef struct Bufblock
{
    char 		*start;
    char 		*end;
    char 		*current;

    // Extra
    struct Bufblock *next;
} Bufblock;
/*e: struct Bufblock */

/*s: struct Word */
typedef struct Word
{
    char 		*s;

    // Extra
    struct Word 	*next;
} Word;
/*e: struct Word */

// used by main and parse.c
extern Word *target1;


/*s: struct Envy */
typedef struct Envy
{
    char 		*name;

    // list<ref_own<Word>>
    Word 		*values;
} Envy;
/*e: struct Envy */

extern Envy *envy;

/*s: struct Rule */
typedef struct Rule
{
    char 		*target;	/* one target */
    char 		*recipe;	/* do it ! */
    Word 		*tail;		/* constituents of targets */

    int 		rule;		/* rule number */
    short 		line;		/* source line */
    char 		*file;		/* source file */

    // enum<rule_attr>
    short 		attr;		/* attributes */

    Word 		*alltargets;	/* all the targets */
    Reprog		*pat;		/* reg exp goo */
    char		*prog;		/* to use in out of date */

    // Extra
    struct Rule	*chain;		/* hashed per target */
    struct Rule	*next;
} Rule;
/*e: struct Rule */

extern Rule *rules, *metarules, *patrule;

/*s: constant META */
/*	Rule.attr	*/
#define		META		0x0001
/*e: constant META */
/*s: constant UNUSED */
//@Scheck: dead indeed
#define		UNUSED		0x0002
/*e: constant UNUSED */
/*s: constant UPD */
#define		UPD		0x0004
/*e: constant UPD */
/*s: constant QUIET */
#define		QUIET		0x0008
/*e: constant QUIET */
/*s: constant VIR */
#define		VIR		0x0010
/*e: constant VIR */
/*s: constant REGEXP */
#define		REGEXP		0x0020
/*e: constant REGEXP */
/*s: constant NOREC */
#define		NOREC		0x0040
/*e: constant NOREC */
/*s: constant DEL */
#define		DEL		0x0080
/*e: constant DEL */
/*s: constant NOVIRT */
#define		NOVIRT		0x0100
/*e: constant NOVIRT */

/*s: constant NREGEXP */
#define		NREGEXP		10
/*e: constant NREGEXP */

/*s: struct Arc */
typedef struct Arc
{
    //? enum<rule_flag>?
    short		flag;

    // ref<node>, reverse of Node.prereq?
    struct Node	*n;

    Rule		*r;

    char		*stem;
    char		*prog;

    char		*match[NREGEXP];
    
    //Extra
    struct Arc	*next;
} Arc;
/*e: struct Arc */

/*s: constant TOGO */
/* Arc.flag */
#define		TOGO		1
/*e: constant TOGO */

/*s: struct Node */
typedef struct Node
{
    char		*name;
    ulong		time;

    // enum<node_flag>
    ushort		flags;

    // list<ref_own?<Arc>>
    Arc		*prereqs;

    // Extra
    struct Node	*next;		/* list for a rule */
} Node;
/*e: struct Node */

/*s: constant VIRTUAL */
/* Node.flags */
#define		VIRTUAL		0x0001
/*e: constant VIRTUAL */
/*s: constant CYCLE */
#define		CYCLE		0x0002
/*e: constant CYCLE */
/*s: constant READY */
#define		READY		0x0004
/*e: constant READY */
/*s: constant CANPRETEND */
#define		CANPRETEND	0x0008
/*e: constant CANPRETEND */
/*s: constant PRETENDING */
#define		PRETENDING	0x0010
/*e: constant PRETENDING */
/*s: constant NOTMADE */
#define		NOTMADE		0x0020
/*e: constant NOTMADE */
/*s: constant BEINGMADE */
#define		BEINGMADE	0x0040
/*e: constant BEINGMADE */
/*s: constant MADE */
#define		MADE		0x0080
/*e: constant MADE */
/*s: function MADESET */
#define		MADESET(n,m)	n->flags = (n->flags&~(NOTMADE|BEINGMADE|MADE))|(m)
/*e: function MADESET */
/*s: constant PROBABLE */
#define		PROBABLE	0x0100
/*e: constant PROBABLE */
/*s: constant VACUOUS */
#define		VACUOUS		0x0200
/*e: constant VACUOUS */
/*s: constant NORECIPE */
#define		NORECIPE	0x0400
/*e: constant NORECIPE */
/*s: constant DELETE */
#define		DELETE		0x0800
/*e: constant DELETE */
/*s: constant NOMINUSE */
#define		NOMINUSE	0x1000
/*e: constant NOMINUSE */

/*s: struct Job */
typedef struct Job
{
    Rule		*r;	/* master rule for job */
    Node		*n;	/* list of node targets */
    char		*stem;
    char		**match;

    Word		*p;	/* prerequistes */
    Word		*np;	/* new prerequistes */
    Word		*t;	/* targets */
    Word		*at;	/* all targets */

    int		nproc;	/* slot number */

    // Extra
    struct Job	*next;
} Job;
/*e: struct Job */

extern Job *jobs;

/*s: struct Symtab */
typedef struct Symtab
{
    char		*name;

    short		space;

    // generic part
    union{
        void		*ptr;
        uintptr	value;
    } u;

    // Extra
    struct Symtab	*next;
} Symtab;
/*e: struct Symtab */

/*s: enum _anon_ */
enum {
    S_VAR,		/* variable -> value */
    S_TARGET,	/* target -> rule */
    S_TIME,		/* file -> time */

    S_PID,		/* pid -> products */
    S_NODE,		/* target name -> node */
    S_AGG,		/* aggregate -> time */
    S_BITCH,	/* bitched about aggregate not there */
    S_NOEXPORT,	/* var -> noexport */
    S_OVERRIDE,	/* can't override */
    S_OUTOFDATE,	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
    S_MAKEFILE,	/* target -> node */
    S_MAKEVAR,	/* dumpable mk variable */
    S_EXPORTED,	/* var -> current exported value */
    S_BULKED,	/* we have bulked this dir */
    S_WESET,	/* variable; we set in the mkfile */
    S_INTERNAL,	/* an internal mk variable (e.g., stem, target) */
};
/*e: enum _anon_ */

extern	int	debug;
extern	int	nflag, tflag, iflag, kflag, aflag;
extern	int	mkinline;
extern	char	*infile;
extern	int	nreps;
extern	char	*explain;

extern	char	*termchars;
//extern	char 	*shell;
//extern	char 	*shellname;
extern	char 	*shflags;
//extern	int	IWS;

/*s: function SYNERR */
#define	SYNERR(l)	(fprint(2, "mk: %s:%d: syntax error; ", infile, ((l)>=0)?(l):mkinline))
/*e: function SYNERR */
/*s: function RERR */
//#define	RERR(r)		(fprint(2, "mk: %s:%d: rule error; ", (r)->file, (r)->line))
/*e: function RERR */
/*s: constant NAMEBLOCK */
#define	NAMEBLOCK	1000
/*e: constant NAMEBLOCK */
/*s: constant BIGBLOCK */
#define	BIGBLOCK	20000
/*e: constant BIGBLOCK */

/*s: function SEP */
//#define	SEP(c)		(((c)==' ')||((c)=='\t')||((c)=='\n'))
/*e: function SEP */
/*s: function WORDCHR */
#define WORDCHR(r)	((r) > ' ' && !utfrune("!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", (r)))
/*e: function WORDCHR */

/*s: function DEBUG */
#define	DEBUG(x)	(debug&(x))
/*e: function DEBUG */
/*s: constant D_PARSE */
#define		D_PARSE		0x01
/*e: constant D_PARSE */
/*s: constant D_GRAPH */
#define		D_GRAPH		0x02
/*e: constant D_GRAPH */
/*s: constant D_EXEC */
#define		D_EXEC		0x04
/*e: constant D_EXEC */

/*s: function LSEEK */
#define	LSEEK(f,o,p)	seek(f,o,p)
/*e: function LSEEK */

/*s: function PERCENT */
#define	PERCENT(ch)	(((ch) == '%') || ((ch) == '&'))
/*e: function PERCENT */

#include	"fns.h"
/*e: mk/mk.h */
