/*s: mk/mk.h */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
#include	<regexp.h>

extern Biobuf bout;

typedef struct Symtab Symtab;
typedef struct Word Word;
typedef struct Rule Rule;
typedef struct Node Node;
typedef struct Arc Arc;
typedef struct Envy Envy;
typedef struct Job Job;
typedef struct Bufblock Bufblock;

/*s: struct Bufblock */
struct Bufblock
{
    char 		*start;
    char 		*end;
    char 		*current;

    // Extra
    struct Bufblock *next;
};
/*e: struct Bufblock */

/*s: struct Word */
struct Word
{
    char 		*s;
    struct Word 	*next;
};
/*e: struct Word */

// used by main and parse.c
extern Word *target1;


/*s: struct Envy */
struct Envy
{
    // key
    char 		*name;
    // value, list<ref_own<string>>
    Word 		*values;
};
/*e: struct Envy */

extern Envy *envy;

/*s: struct Rule */
struct Rule
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
    int 		rule;		/* rule number */
    /*x: [[Rule]] other fields */
    Reprog		*pat;		/* reg exp goo */
    /*x: [[Rule]] other fields */
    // list<ref_own?<string>>?
    Word 		*alltargets;	/* all the targets */
    /*x: [[Rule]] other fields */
    char		*prog;		/* to use in out of date */
    /*e: [[Rule]] other fields */

    // Extra
    /*s: [[Rule]] extra fields */
    // list<ref_own<Rule>> (head = rules)
    struct Rule	*next;
    /*x: [[Rule]] extra fields */
    struct Rule	*chain;		/* hashed per target */
    /*e: [[Rule]] extra fields */
};
/*e: struct Rule */

extern Rule *rules, *metarules, *patrule;

/*s: enum rule_attr */
enum rule_attr {
    META   = 0x0001,
    /*s: enum rule_attr cases */
    UPD    = 0x0004,
    VIR    = 0x0010,
    NOREC  = 0x0040,
    DEL    = 0x0080,
    NOVIRT = 0x0100,
    /*x: enum rule_attr cases */
    QUIET  = 0x0008,
    /*x: enum rule_attr cases */
    REGEXP = 0x0020,
    /*e: enum rule_attr cases */
};
/*e: enum rule_attr */

/*s: constant NREGEXP */
#define		NREGEXP		10
/*e: constant NREGEXP */

/*s: struct Arc */
struct Arc
{
    // option<ref<Node>>, the other node in the arc
    struct Node *n;
    // ref<Rule>, contain recipe to gen the target node from the dependent nodes
    Rule *r;

    /*s: [[Arc]] other fields */
    // what will replace the %
    char		*stem;
    /*x: [[Arc]] other fields */
    // bool (TOGO)
    short flag;
    /*x: [[Arc]] other fields */
    char		*match[NREGEXP];
    /*x: [[Arc]] other fields */
    char		*prog;
    /*e: [[Arc]] other fields */
    
    //Extra
    /*s: [[Arc]] extra fields */
    // list<ref_own<arc> (head = Node.prereq)
    struct Arc	*next;
    /*e: [[Arc]] extra fields */
};
/*e: struct Arc */

/*s: constant TOGO */
/* Arc.flag */
#define		TOGO		true
/*e: constant TOGO */

/*s: struct Node */
struct Node
{
    // usually a filename, or target label like 'default'
    char*		name; 
    // last mtime of the file (or zero for non existing files)
    ulong		time;
    // bitset<enum<node_flag>>
    ushort		flags;

    /*s: [[Node]] other fields */
    // list<ref_own<Arc>> (next = Arc.next)
    Arc		*prereqs;
    /*e: [[Node]] other fields */

    // Extra
    /*s: [[Node]] extra fields */
    struct Node	*next;		/* list for a rule */
    /*e: [[Node]] extra fields */
};
/*e: struct Node */

/*s: enum node_flag */
enum node_flag {
    /*s: enum node_flag cases */
    NOTMADE    = 0x0020,
    BEINGMADE  = 0x0040,
    MADE       = 0x0080,
    /*x: enum node_flag cases */
    CYCLE      = 0x0002,
    /*x: enum node_flag cases */
    VACUOUS    = 0x0200,
    /*x: enum node_flag cases */
    PROBABLE   = 0x0100,
    /*x: enum node_flag cases */
    READY      = 0x0004,
    /*x: enum node_flag cases */
    VIRTUAL    = 0x0001,
    NORECIPE   = 0x0400,
    DELETE     = 0x0800,
    /*x: enum node_flag cases */
    NOMINUSE   = 0x1000,
    /*x: enum node_flag cases */
    CANPRETEND = 0x0008,
    PRETENDING = 0x0010,
    /*e: enum node_flag cases */
};
/*e: enum node_flag */
/*s: function MADESET */
#define	MADESET(n,m)	n->flags = (n->flags&~(NOTMADE|BEINGMADE|MADE))|(m)
/*e: function MADESET */

/*s: struct Job */
struct Job
{
    Word		*t;	/* targets */
    Word		*p;	/* prerequistes */

    //list<ref<Node>> (next = Node.next??)
    Node		*n;	/* list of node targets */
    //ref<Rule>
    Rule		*r;	/* master rule for job */

    char		*stem;

    int		nproc;	/* slot number */ // or -1 if unassigned

    /*s: [[Job]] other fields */
    char		**match;
    /*x: [[Job]] other fields */
    Word		*at;	/* all targets */
    /*x: [[Job]] other fields */
    Word		*np;	/* new prerequistes */
    /*e: [[Job]] other fields */

    // Extra
    /*s: [[Job]] extra fields */
    struct Job	*next;
    /*e: [[Job]] extra fields */
};
/*e: struct Job */

extern Job *jobs;

/*s: struct Symtab */
struct Symtab
{
    // the key
    char		*name;
    // enum<sxxx>, the ``namespace''
    short		space;

    // the value (generic)
    union{
        void*	ptr;
        uintptr	value;
    } u;

    // Extra
    /*s: [[Symtab]] extra fields */
    struct Symtab	*next;
    /*e: [[Symtab]] extra fields */
};
/*e: struct Symtab */

/*s: enum sxxx */
enum sxxx {
    S_VAR,	/* variable -> value */ // value is a list of words
    /*s: enum sxxx cases */
    S_MAKEVAR,	/* dumpable mk variable */
    /*x: enum sxxx cases */
    S_INTERNAL,	/* an internal mk variable (e.g., stem, target) */
    /*x: enum sxxx cases */
    S_TARGET,		/* target -> rule */ // actually rules
    /*x: enum sxxx cases */
    S_NODE,		/* target name -> node */
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
    S_AGG,		/* aggregate -> time */
    /*x: enum sxxx cases */
    S_BITCH,	/* bitched about aggregate not there */
    /*x: enum sxxx cases */
    S_OUTOFDATE,	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
    /*x: enum sxxx cases */
    S_EXPORTED,	/* var -> current exported value */
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
extern int runerrs;

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

/*s: enum dxxx */
enum dxxx {
    // for rules
    D_PARSE =		0x01,
    // for node and arcs
    D_GRAPH =		0x02,
    // for jobs
    D_EXEC  =		0x04,
};
/*e: enum dxxx */
/*s: function DEBUG */
#define	DEBUG(x)	(debug&(x))
/*e: function DEBUG */

/*s: function PERCENT */
#define	PERCENT(ch)	(((ch) == '%') || ((ch) == '&'))
/*e: function PERCENT */

#include	"fns.h"
/*e: mk/mk.h */
