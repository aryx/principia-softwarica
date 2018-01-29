/*s: rc/rc.h */
#include <u.h>
#include <libc.h>

#ifndef ERRMAX
/*s: constant [[ERRMAX]] */
#define ERRMAX 128
/*e: constant [[ERRMAX]] */
#endif

/*s: constant [[YYMAXDEPTH]] */
//@Scheck: used in y.tab.c
#define	YYMAXDEPTH	500
/*e: constant [[YYMAXDEPTH]] */

// forward decls
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
//#pragma incomplete word
//#pragma incomplete io
//#endif

/*s: struct [[Tree]] */
struct Tree {

    // either<enum<Token_kind>, char>
    int	type;
    // string of the token or AST dump of the whole subtree for certain nodes
    char *str;

    // array<option<ref_own<Tree>>
    tree	*child[3];

    /*s: [[Tree]] redirection and pipe specific fields */
    //enum<Redirection_kind>
    int	rtype;

    // For a pipe, fd0 is the left fd of the pipe, and fd1 the right fd.
    // For a redirection, fd0 is what we redirect (stdout for >, stdin for <)
    // and fd1 is what we possibly redirect to (when DUP).
    fdt fd0;
    fdt fd1;	/* details of REDIR PIPE DUP tokens */
    /*e: [[Tree]] redirection and pipe specific fields */
    /*s: [[Tree]] word specific fields */
    bool	quoted;
    /*e: [[Tree]] word specific fields */

    // Extra
    /*s: [[Tree]] extra fields */
    tree	*next;
    /*e: [[Tree]] extra fields */
};
/*e: struct [[Tree]] */

tree *newtree(void);
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

/*s: struct [[Code]] */
/*
 * The first word of any code vector is a reference count.
 * Always create a new reference to a code vector by calling codecopy(.).
 * Always call codefree(.) when deleting a reference.
 */
union Code {
    void	(*f)(void); // Xxxx() bytecode
    int	i;
    char	*s;
};
/*e: struct [[Code]] */

extern char *promptstr;
extern bool doprompt;

/*s: constant [[APPEND]] */
#define	APPEND	1
/*e: constant [[APPEND]] */
/*s: constant [[WRITE]] */
#define	WRITE	2
/*e: constant [[WRITE]] */
/*s: constant [[READ]] */
#define	READ	3
/*e: constant [[READ]] */
/*s: constant [[HERE]] */
#define	HERE	4
/*e: constant [[HERE]] */
/*s: constant [[DUPFD]] */
#define	DUPFD	5
/*e: constant [[DUPFD]] */
/*s: constant [[CLOSE]] */
#define	CLOSE	6
/*e: constant [[CLOSE]] */
/*s: constant [[RDWR]] */
#define RDWR	7
/*e: constant [[RDWR]] */

/*s: struct [[Word]] */
/*
 * word lists are in correct order,
 * i.e. word0->word1->word2->word3->nil
 */
struct Word {
    char *word;

    // Extra
    word *next;
};
/*e: struct [[Word]] */
/*s: struct [[List]] */
struct List {
    // list<ref_own<Word>> (next = Word.next)
    word *words;

    // Extra
    list *next;
};
/*e: struct [[List]] */
word *newword(char *, word *);
word *copywords(word *, word *);

/*s: struct [[Var]] */
struct Var {
    // key
    char	*name;		/* ascii name */
    // value
    word	*val;		/* value */

    /*s: [[Var]] other fields */
    code	*fn;		/* pointer to function's code vector */
    int	pc;		/* pc of start of function */
    bool	fnchanged;
    /*x: [[Var]] other fields */
    bool	changed;
    /*e: [[Var]] other fields */
    // Extra
    /*s: [[Var]] extra fields */
    var	*next;		/* next on hash or local list */
    /*e: [[Var]] extra fields */
};
/*e: struct [[Var]] */

var *vlook(char*);
var *gvlook(char*);
var *newvar(char*, var*);

/*s: constant [[NVAR]] */
#define	NVAR	521
/*e: constant [[NVAR]] */
extern var *gvar[NVAR];		/* hash for globals */

#define	new(type)	((type *)emalloc(sizeof(type)))
void *emalloc(long);
void efree(void *);

/*s: struct [[Here]] */
struct Here {
    tree	*tag;
    char	*name;
    struct Here *next;
};
/*e: struct [[Here]] */
extern int mypid;

/*s: constant [[GLOB]] */
/*
 * Glob character escape in strings:
 *	In a string, GLOB must be followed by *?[ or GLOB.
 *	GLOB* matches any string
 *	GLOB? matches any single character
 *	GLOB[...] matches anything in the brackets
 *	GLOBGLOB matches GLOB
 */
#define	GLOB	'\001'
/*e: constant [[GLOB]] */

extern int nerror;	/* number of errors encountered during compilation */
/*s: constant [[PRD]] */
/*
 * Which fds are the reading/writing end of a pipe?
 * Unfortunately, this can vary from system to system.
 * 9th edition Unix doesn't care, the following defines
 * work on plan 9.
 */
#define	PRD	0
/*e: constant [[PRD]] */
/*s: constant [[PWR]] */
#define	PWR	1
/*e: constant [[PWR]] */

extern char *Rcmain;
extern char *Fdprefix;

extern int ndot;

char *getstatus(void);

tree *token(char*, int);
tree *klook(char*);

extern bool lastword;
/*e: rc/rc.h */
