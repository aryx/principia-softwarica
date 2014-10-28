/*s: rc/rc.h */
/*
 * Assume plan 9 by default; if Unix is defined, assume unix.
 * Please don't litter the code with ifdefs.  The five below should be enough.
 */

//#ifndef Unix
/* plan 9 */
#include <u.h>
#include <libc.h>

/*s: constant NSIG */
#define	NSIG	32
/*e: constant NSIG */
/*s: constant SIGINT */
#define	SIGINT	2
/*e: constant SIGINT */
/*s: constant SIGQUIT */
#define	SIGQUIT	3
/*e: constant SIGQUIT */

//#define fcntl(fd, op, arg) /* unix compatibility */
//#define F_SETFD  
//#define FD_CLOEXEC 

//#else
//#include "unix.h"
//#endif

#ifndef ERRMAX
/*s: constant ERRMAX */
#define ERRMAX 128
/*e: constant ERRMAX */
#endif

/*s: constant YYMAXDEPTH */
//@Scheck: used in y.tab.c
#define	YYMAXDEPTH	500
/*e: constant YYMAXDEPTH */

//#ifndef YYPREFIX
#ifndef PAREN
//#include "x.tab.h"
//pad: better like that, otherwise get some "redefined FOR macro" error
#endif
//#endif

typedef struct Tree tree;
typedef struct Word word;
typedef struct Io io;
typedef union Code code;
typedef struct Var var;
typedef struct List list;
typedef struct Redir redir;
typedef struct Thread thread;
typedef struct Builtin builtin;

//#ifndef Unix
#pragma incomplete word
#pragma incomplete io
//#endif

/*s: struct tree */
struct Tree {

    // enum<token_kind>
    int	type;

    char *str;

    // array<option<ref_own<Tree>>
    tree	*child[3];

    bool iskw;

    /*s: [[Tree]] other fields */
    //enum<redirection_kind>
    int	rtype;
    int fd0, fd1;	/* details of REDIR PIPE DUP tokens */
    bool	quoted;
    /*e: [[Tree]] other fields */

    // Extra
    /*s: [[Tree]] extra fields */
    tree	*next;
    /*e: [[Tree]] extra fields */
};
/*e: struct tree */

tree *newtree(void);
tree *token(char*, int);
tree *klook(char*);
//@Scheck: useful, for syn.y, and not just for tree.c
tree *tree1(int, tree*);
tree *tree2(int, tree*, tree*);
//@Scheck: useful, for syn.y, and not just for tree.c
tree *tree3(int, tree*, tree*, tree*);
tree *mung1(tree*, tree*);
tree *mung2(tree*, tree*, tree*);
tree *mung3(tree*, tree*, tree*, tree*);
tree *epimung(tree*, tree*);
tree *simplemung(tree*);
tree *heredoc(tree*);


/*s: struct code */
/*
 * The first word of any code vector is a reference count.
 * Always create a new reference to a code vector by calling codecopy(.).
 * Always call codefree(.) when deleting a reference.
 */
union Code {
    void	(*f)(void); // Xxxx() opcode
    int	i;
    char	*s;
};
/*e: struct code */

extern char *promptstr;
extern bool doprompt;

/*s: constant NTOK */
#define	NTOK	8192		/* maximum bytes in a word (token) */
/*e: constant NTOK */

extern char tok[NTOK + UTFmax];

/*s: constant APPEND */
#define	APPEND	1
/*e: constant APPEND */
/*s: constant WRITE */
#define	WRITE	2
/*e: constant WRITE */
/*s: constant READ */
#define	READ	3
/*e: constant READ */
/*s: constant HERE */
#define	HERE	4
/*e: constant HERE */
/*s: constant DUPFD */
#define	DUPFD	5
/*e: constant DUPFD */
/*s: constant CLOSE */
#define	CLOSE	6
/*e: constant CLOSE */
/*s: constant RDWR */
#define RDWR	7
/*e: constant RDWR */

/*s: struct var */
struct Var {
    char	*name;		/* ascii name */
    word	*val;		/* value */

    /*s: [[Var]] other fields */
    bool	changed;
    code	*fn;		/* pointer to function's code vector */
    int	fnchanged;
    int	pc;		/* pc of start of function */
    /*e: [[Var]] other fields */

    // Extra
    /*s: [[Var]] extra fields */
    var	*next;		/* next on hash or local list */
    /*e: [[Var]] extra fields */
};
/*e: struct var */

var *vlook(char*);
var *gvlook(char*);
var *newvar(char*, var*);

/*s: constant NVAR */
#define	NVAR	521
/*e: constant NVAR */

extern var *gvar[NVAR];		/* hash for globals */

#define	new(type)	((type *)emalloc(sizeof(type)))

void *emalloc(long);
void *Malloc(ulong);
void efree(void *);

/*s: struct here */
struct Here {
    tree	*tag;
    char	*name;
    struct Here *next;
};
/*e: struct here */
extern int mypid;

/*s: constant GLOB */
/*
 * Glob character escape in strings:
 *	In a string, GLOB must be followed by *?[ or GLOB.
 *	GLOB* matches any string
 *	GLOB? matches any single character
 *	GLOB[...] matches anything in the brackets
 *	GLOBGLOB matches GLOB
 */
#define	GLOB	'\001'
/*e: constant GLOB */

extern int nerror;		/* number of errors encountered during compilation */
/*s: constant PRD */
/*
 * Which fds are the reading/writing end of a pipe?
 * Unfortunately, this can vary from system to system.
 * 9th edition Unix doesn't care, the following defines
 * work on plan 9.
 */
#define	PRD	0
/*e: constant PRD */
/*s: constant PWR */
#define	PWR	1
/*e: constant PWR */

extern char *Rcmain, *Fdprefix;
extern int ndot;
char *getstatus(void);

extern int lastc;
extern bool lastword;

/*e: rc/rc.h */
