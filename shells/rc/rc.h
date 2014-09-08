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
#define	YYMAXDEPTH	500
/*e: constant YYMAXDEPTH */

//#ifndef YYPREFIX
#ifndef PAREN
#include "x.tab.h"
#endif
//#endif

typedef struct tree tree;
typedef struct word word;
typedef struct io io;
typedef union code code;
typedef struct var var;
typedef struct list list;
typedef struct redir redir;
typedef struct thread thread;
typedef struct builtin builtin;

//#ifndef Unix
#pragma incomplete word
#pragma incomplete io
/*s: struct tree */
//#endif

struct tree{
    int	type;
    int	rtype, fd0, fd1;	/* details of REDIR PIPE DUP tokens */
    char	*str;
    int	quoted;
    int	iskw;
    tree	*child[3];
    tree	*next;
};
/*e: struct tree */
tree *newtree(void);
tree *token(char*, int), *klook(char*), *tree1(int, tree*);
tree *tree2(int, tree*, tree*), *tree3(int, tree*, tree*, tree*);
tree *mung1(tree*, tree*), *mung2(tree*, tree*, tree*);
tree *mung3(tree*, tree*, tree*, tree*), *epimung(tree*, tree*);
tree *simplemung(tree*), *heredoc(tree*);
void freetree(tree*);
/*s: global cmdtree */
tree *cmdtree;
/*e: global cmdtree */

/*s: struct code */
/*
 * The first word of any code vector is a reference count.
 * Always create a new reference to a code vector by calling codecopy(.).
 * Always call codefree(.) when deleting a reference.
 */
union code{
    void	(*f)(void);
    int	i;
    char	*s;
};
/*e: struct code */

/*s: global promptstr */
char *promptstr;
/*e: global promptstr */
/*s: global doprompt */
int doprompt;
/*e: global doprompt */

/*s: constant NTOK */
#define	NTOK	8192		/* maximum bytes in a word (token) */
/*e: constant NTOK */

/*s: global tok */
char tok[NTOK + UTFmax];
/*e: global tok */

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
struct var{
    char	*name;		/* ascii name */
    word	*val;		/* value */
    int	changed;
    code	*fn;		/* pointer to function's code vector */
    int	fnchanged;
    int	pc;		/* pc of start of function */
    var	*next;		/* next on hash or local list */
};
/*e: struct var */
var *vlook(char*), *gvlook(char*), *newvar(char*, var*);

/*s: constant NVAR */
#define	NVAR	521
/*e: constant NVAR */

/*s: global gvar */
var *gvar[NVAR];		/* hash for globals */
/*e: global gvar */

#define	new(type)	((type *)emalloc(sizeof(type)))

void *emalloc(long);
void *Malloc(ulong);
void efree(void *);

/*s: struct here */
struct here{
    tree	*tag;
    char	*name;
    struct here *next;
};
/*e: struct here */
/*s: global mypid */
int mypid;
/*e: global mypid */

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

/*s: global argp */
char **argp;
/*e: global argp */
/*s: global args */
char **args;
/*e: global args */
/*s: global nerror */
int nerror;		/* number of errors encountered during compilation */
/*e: global nerror */
/*s: global doprompt (rc/rc.h) */
int doprompt;		/* is it time for a prompt? */
/*e: global doprompt (rc/rc.h) */
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

char *Rcmain, *Fdprefix;
/*s: global ndot */
/*
 * How many dot commands have we executed?
 * Used to ensure that -v flag doesn't print rcmain.
 */
int ndot;
/*e: global ndot */
char *getstatus(void);
/*s: global lastc */
int lastc;
/*e: global lastc */
/*s: global lastword */
int lastword;
/*e: global lastword */
/*e: rc/rc.h */
