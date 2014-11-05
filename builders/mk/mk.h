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
    struct Word 	*next;
} Word;
/*e: struct Word */

// used by main and parse.c
extern Word *target1;


/*s: struct Envy */
typedef struct Envy
{
    char 		*name;

    // list<ref_own<string>>
    Word 		*values;
} Envy;
/*e: struct Envy */

extern Envy *envy;

/*s: struct Rule */
typedef struct Rule
{
    char 		*target;	/* one target */
    //list<ref_own<string>>
    Word 		*tail;		/* constituents of targets */
    char 		*recipe;	/* do it ! */

    char* 		file;		/* source file */
    short 		line;		/* source line */

    // enum<rule_attr>
    short 		attr;		/* attributes */

    /*s: [[Rule]] other fields */
    Reprog		*pat;		/* reg exp goo */
    char		*prog;		/* to use in out of date */
    /*x: [[Rule]] other fields */
    int 		rule;		/* rule number */
    /*x: [[Rule]] other fields */
    // list<ref_own?<string>>?
    Word 		*alltargets;	/* all the targets */
    /*e: [[Rule]] other fields */

    // Extra
    /*s: [[Rule]] extra fields */
    // list<ref_own<Rule>> (head = rules)
    struct Rule	*next;
    /*x: [[Rule]] extra fields */
    struct Rule	*chain;		/* hashed per target */
    /*e: [[Rule]] extra fields */
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
    // ref<Node>, dst node in the arc (src is the Node having the prereqs)
    struct Node *n;
    // ref<Rule>, rule to generate the target node from the dependent node
    Rule *r;
    //? enum<rule_flag>?
    short flag;

    /*s: [[Arc]] other fields */
    // what will replace the %
    char		*stem;
    /*x: [[Arc]] other fields */
    char		*prog;
    /*x: [[Arc]] other fields */
    char		*match[NREGEXP];
    /*e: [[Arc]] other fields */
    
    //Extra
    /*s: [[Arc]] extra fields */
    // list<ref_own<arc> (head = Node.prereq)
    struct Arc	*next;
    /*e: [[Arc]] extra fields */
} Arc;
/*e: struct Arc */

/*s: constant TOGO */
/* Arc.flag */
#define		TOGO		1
/*e: constant TOGO */

/*s: struct Node */
typedef struct Node
{
    // usually a filename, or target like 'default'
    char*		name; 
    // last mtime of the file (or zero for non existing files)
    ulong		time;
    // enum<node_flag>
    ushort		flags;

    /*s: [[Node]] other fields */
    // list<ref_own<Arc>> (next = Arc.next)
    Arc		*prereqs;
    /*e: [[Node]] other fields */

    // Extra
    /*s: [[Node]] extra fields */
    struct Node	*next;		/* list for a rule */
    /*e: [[Node]] extra fields */
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
#define	MADESET(n,m)	n->flags = (n->flags&~(NOTMADE|BEINGMADE|MADE))|(m)
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
    Word		*t;	/* targets */
    Rule		*r;	/* master rule for job */

    // list<ref_?<Node>> (next = Node.next?)
    Node		*n;	/* list of node targets */

    char		*stem;

    Word		*p;	/* prerequistes */
    Word		*np;	/* new prerequistes */
    Word		*at;	/* all targets */

    int		nproc;	/* slot number */

    /*s: [[Job]] other fields */
    char		**match;
    /*e: [[Job]] other fields */

    // Extra
    /*s: [[Job]] extra fields */
    struct Job	*next;
    /*e: [[Job]] extra fields */
} Job;
/*e: struct Job */

extern Job *jobs;

/*s: struct Symtab */
typedef struct Symtab
{
    // the key
    char		*name;
    // enum<sxxx>, the ``namespace''
    short		space;

    // the value (generic part)
    union{
        void*	ptr;
        uintptr	value;
    } u;

    // Extra
    /*s: [[Symtab]] extra fields */
    struct Symtab	*next;
    /*e: [[Symtab]] extra fields */
} Symtab;
/*e: struct Symtab */

/*s: enum sxxx */
enum sxxx {
    S_VAR,	/* variable -> value */ // value is a list of words
    /*s: enum sxxx cases */
    S_MAKEVAR,	/* dumpable mk variable */
    /*x: enum sxxx cases */
    S_INTERNAL,	/* an internal mk variable (e.g., stem, target) */
    /*x: enum sxxx cases */
    S_TARGET,		/* target -> rule */
    /*x: enum sxxx cases */
    S_NODE,		/* target name -> node */
    /*x: enum sxxx cases */
    S_OUTOFDATE,	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
    /*x: enum sxxx cases */
    S_NOEXPORT,	/* var -> noexport */ // set of noexport variables
    /*x: enum sxxx cases */
    S_OVERRIDE,	/* can't override */
    /*x: enum sxxx cases */
    S_WESET,	/* variable; we set in the mkfile */
    /*x: enum sxxx cases */
    S_TIME,		/* file -> time */
    /*x: enum sxxx cases */
    S_BULKED,	/* we have bulked this dir */
    /*x: enum sxxx cases */
    S_EXPORTED,	/* var -> current exported value */
    /*x: enum sxxx cases */
    S_AGG,		/* aggregate -> time */
    /*x: enum sxxx cases */
    S_BITCH,	/* bitched about aggregate not there */
    /*e: enum sxxx cases */
};
/*e: enum sxxx */

extern	int	debug;
extern	bool	nflag, tflag, iflag, kflag, aflag;
extern	int	mkinline;
extern	char	*infile;
extern	int	nreps;
extern	bool explain;
extern	char	*termchars;
extern	char 	*shflags;
//extern	char 	*shell;
//extern	char 	*shellname;
//extern	int	IWS;

/*s: function SYNERR */
#define	SYNERR(l)	(fprint(STDERR, "mk: %s:%d: syntax error; ", infile, ((l)>=0)?(l):mkinline))
/*e: function SYNERR */
/*s: function RERR */
//#define	RERR(r)		(fprint(STDERR, "mk: %s:%d: rule error; ", (r)->file, (r)->line))
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
