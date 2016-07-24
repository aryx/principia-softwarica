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
    // ref_own<string>
    char 		*s;

    struct Word 	*next;
};
/*e: struct Word */

// used by main and parse.c
extern Word *target1;


/*s: struct Envy */
struct Envy
{
    // ref<string>, the key
    char 		*name;

    // list<ref_own<string>>, the value
    Word 		*values;
};
/*e: struct Envy */

extern Envy *envy;

/*s: struct Rule */
struct Rule
{
    char 		*target;	/* one target */
    // option<ref_own<Words>>
    Word 		*tail;		/* constituents of targets */
    // ref_own<string>, never nil, but can be empty string (just '\0')
    char 		*recipe;	/* do it ! */

    /*s: [[Rule]] other fields */
    // bitset<Rule_attr>
    short 		attr;		/* attributes */
    /*x: [[Rule]] other fields */
    // ref<list<ref_own<string>>
    Word 		*alltargets;	/* all the targets */
    /*x: [[Rule]] other fields */
    int 		rule;		/* rule number */
    /*x: [[Rule]] other fields */
    Reprog		*pat;		/* reg exp goo */
    /*x: [[Rule]] other fields */
    char		*prog;		/* to use in out of date */
    /*e: [[Rule]] other fields */
    /*s: [[Rule]] debug fields */
    char* 		file;		/* source file */
    short 		line;		/* source line */
    /*e: [[Rule]] debug fields */

    // Extra
    /*s: [[Rule]] extra fields */
    // list<ref_own<Rule>> (head = rules | metarules)
    struct Rule	*next;
    /*x: [[Rule]] extra fields */
    // list<ref<Rule>> (head = symlook(x, S_TARGET))
    struct Rule	*chain;		/* hashed per target */
    /*e: [[Rule]] extra fields */
};
/*e: struct Rule */

extern Rule *rules, *metarules, *patrule;

/*s: enum Rule_attr */
enum Rule_attr {
    META   = 0x0001,
    /*s: [[Rule_attr]] cases */
    REGEXP = 0x0020,
    /*x: [[Rule_attr]] cases */
    VIR    = 0x0010,
    /*x: [[Rule_attr]] cases */
    DEL    = 0x0080,
    /*x: [[Rule_attr]] cases */
    QUIET  = 0x0008,
    /*x: [[Rule_attr]] cases */
    NOREC  = 0x0040,
    /*x: [[Rule_attr]] cases */
    NOVIRT = 0x0100,
    /*e: [[Rule_attr]] cases */
};
/*e: enum Rule_attr */

/*s: constant NREGEXP */
#define		NREGEXP		10
/*e: constant NREGEXP */

/*s: struct Arc */
struct Arc
{
    // option<ref<Node>>, the other node in the arc (the dependency)
    struct Node *n;
    // ref<Rule>, to gen the target node from the dependent node
    Rule *r;

    /*s: [[Arc]] other fields */
    // ref_own<string>, what will replace the %
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
    // list<ref_own<arc> (head = Node.prereqs)
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
    // ref_own<string>, usually a filename, or a virtual target like 'clean'
    char*		name; 
    // option<Time> (None = 0, for nonexistent files and virtual targets)
    ulong		time; // last mtime of file 
    // bitset<enum<Node_flag>>
    ushort		flags;

    /*s: [[Node]] arcs field */
    // list<ref_own<Arc>> (next = Arc.next)
    Arc		*prereqs;
    /*e: [[Node]] arcs field */

    // Extra
    /*s: [[Node]] extra fields */
    // list<ref<Node>> (head = Job.n)
    struct Node	*next;		/* list for a rule */
    /*e: [[Node]] extra fields */
};
/*e: struct Node */

/*s: enum Node_flag */
enum Node_flag {
    /*s: [[Node_flag]] cases */
    CYCLE      = 0x0002,
    /*x: [[Node_flag]] cases */
    VACUOUS    = 0x0200,
    /*x: [[Node_flag]] cases */
    PROBABLE   = 0x0100,
    /*x: [[Node_flag]] cases */
    READY      = 0x0004,
    /*x: [[Node_flag]] cases */
    NOTMADE    = 0x0020,
    BEINGMADE  = 0x0040,
    MADE       = 0x0080,
    /*x: [[Node_flag]] cases */
    VIRTUAL    = 0x0001,
    /*x: [[Node_flag]] cases */
    DELETE     = 0x0800,
    /*x: [[Node_flag]] cases */
    NOMINUSE   = 0x1000,
    /*x: [[Node_flag]] cases */
    NORECIPE   = 0x0400,
    /*x: [[Node_flag]] cases */
    CANPRETEND = 0x0008,
    PRETENDING = 0x0010,
    /*e: [[Node_flag]] cases */
};
/*e: enum Node_flag */
/*s: function MADESET */
#define	MADESET(n,m)	n->flags = (n->flags&~(NOTMADE|BEINGMADE|MADE))|(m)
/*e: function MADESET */

/*s: struct Job */
struct Job
{
    // ref<Rule>
    Rule		*r;	/* master rule for job */

    Word		*t;	/* targets */
    // list<ref<Node>> (next = Node.next)
    Node		*n;	/* list of node targets */

    Word		*p;	/* prerequisites */

    // option<int> (None = -1)
    int		nproc;	/* slot number */

    /*s: [[Job]] other fields */
    char		*stem;
    /*x: [[Job]] other fields */
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
    // the key: (name x space)

    // ref_own<string>
    char		*name;
    // enum<Namespace>, the ``namespace''
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

/*s: enum Sxxx */
enum Sxxx {
    S_VAR,	/* variable -> value */ // value is a list of words
    /*s: [[Sxxx]] cases */
    S_INTERNAL,	/* an internal mk variable (e.g., stem, target) */
    /*x: [[Sxxx]] cases */
    S_TARGET,		/* target -> rules */ 
    /*x: [[Sxxx]] cases */
    S_OVERRIDE,	/* can't override */
    /*x: [[Sxxx]] cases */
    S_TIME,		/* file -> time */
    /*x: [[Sxxx]] cases */
    S_WESET,	/* variable; we set in the mkfile */
    /*x: [[Sxxx]] cases */
    S_OUTOFDATE,	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
    /*x: [[Sxxx]] cases */
    S_NOEXPORT,	/* var -> noexport */ // set of noexport variables
    /*x: [[Sxxx]] cases */
    S_AGG,		/* aggregate -> time */
    /*x: [[Sxxx]] cases */
    S_BITCH,	/* bitched about aggregate not there */
    /*x: [[Sxxx]] cases */
    S_NODE,		/* target name -> node */
    /*x: [[Sxxx]] cases */
    S_BULKED,	/* we have bulked this dir */
    /*e: [[Sxxx]] cases */
};
/*e: enum Sxxx */

extern	int	debug;
extern	bool	nflag, tflag, iflag, kflag, aflag;
extern	int	mkinline;
extern	char	*infile;
extern	int	nreps;
extern	bool explain;
extern	char	*termchars;
extern	char 	*shflags;
extern int runerrs;

/*s: function SYNERR */
#define	SYNERR(l)	(fprint(STDERR, "mk: %s:%d: syntax error; ", \
                            infile, ((l)>=0)? (l) : mkinline))
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

/*s: enum Dxxx */
enum Dxxx {
    // for rules
    D_PARSE =		0x01,
    // for node and arcs
    D_GRAPH =		0x02,
    // for jobs
    D_EXEC  =		0x04,

    // tracing some calls
    D_TRACE  =		0x08,
};
/*e: enum Dxxx */
/*s: function DEBUG */
#define	DEBUG(x)	(debug&(x))
/*e: function DEBUG */

/*s: function PERCENT */
#define	PERCENT(ch)	(((ch) == '%') || ((ch) == '&'))
/*e: function PERCENT */

#include	"fns.h"
/*e: mk/mk.h */
