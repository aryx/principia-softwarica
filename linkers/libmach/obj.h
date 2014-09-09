/*s: linkers/libmach/obj.h */
/*
 * obj.h -- defs for dealing with object files
 */

typedef enum Kind		/* variable defs and references in obj */
{
    aNone,			/* we don't care about this prog */
    aName,			/* introduces a name */
    aText,			/* starts a function */
    aData,			/* references to a global object */
} Kind;

typedef struct	Prog	Prog;

/*s: struct Prog (linkers/libmach/obj.h) */
struct Prog		/* info from .$O files */
{
    Kind	kind;		/* what kind of symbol */
    char	type;		/* type of the symbol: ie, 'T', 'a', etc. */
    char	sym;		/* index of symbol's name */
    char	*id;		/* name for the symbol, if it introduces one */
    uint	sig;		/* type signature for symbol */
};
/*e: struct Prog (linkers/libmach/obj.h) */

/*s: constant UNKNOWN */
#define UNKNOWN	'?'
/*e: constant UNKNOWN */
void		_offset(int, vlong);
/*e: linkers/libmach/obj.h */
